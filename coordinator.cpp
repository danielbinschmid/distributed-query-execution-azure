#include "CurlEasyPtr.h"
#include <array>
#include <charconv>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>
#include <chrono>
#include <fstream>

#include "storage_credential.h"
#include "storage_account.h"
#include "blob/blob_client.h"

#include <stdlib.h>
#include "AzureBlobClient.h"
#include "config.h"
#include "tools.h"
#include "HashRanging.h"
#include "config.h"

/**
 * Leader process that coordinates workers. 
 * 
 * Workers connect on the specified port
 * and the coordinator distributes the work of the CSV file list
 * Example:
 *    ./coordinator http://example.org/filelist.csv 4242 
*/
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <URL to csv list> <listen port>" << std::endl;
      return 1;
   }  
   // ---------------------- STRATEGY --------------------------
   // First, do the initial work, i.e. computing aggregates. Apply the same elasticity system as in assignment 3. 
   // Compute hash values for domains. Then split in ranges with pre-specified number of ranges.  

   // Second, after the initial work is done, use workers again to merge and sort ranges.

   // After each range is sorted and merged, merge the top25 aggregates in the coordinator
   // ----------------------------------------------------------   

   std::vector<std::string> initialPartition;
   initialPartition.reserve(100);
   tools::coordinator::getInitialPartitionsLocalFiles(argv[1], initialPartition);
   
   auto listener = tools::coordinator::getListenerSocket(argv[2]);

   // Setup polling for new connections and worker responses
   std::vector<pollfd> pollFds;
   pollFds.push_back(pollfd{
      .fd = listener,
      .events = POLLIN,
      .revents = {},
   });


   // 1st: compute aggregates for each partition, resulting in nAggregates * nPartitions files
   // 2nd: Merge and sort nPartitions for every subpartition, resulting in nAggregates top25 files
   // 3rd: Merge nAggregates top25 files in coordinator
   
   // Distribute the work
   size_t result = 0;
   auto distributedWork = std::unordered_map<int, std::string>();
   while (!initialPartition.empty() || !distributedWork.empty()) {
      poll(pollFds.data(), pollFds.size(), -1);

      for (size_t index = 0, limit = pollFds.size(); index != limit; ++index) {
         const auto& pollFd = pollFds[index];
         // Look for ready connections
         if (!(pollFd.revents & POLLIN)) continue;

         auto assignWork = [&](int fd) {
            // Assign work (if any)
            if (initialPartition.empty()) return;
            distributedWork[fd] = std::move(initialPartition.back());
            initialPartition.pop_back();

            const auto& file = distributedWork[fd];
            if (auto status = send(fd, file.c_str(), file.size(), 0); status == -1) {
               perror("send() failed");
               

               initialPartition.push_back(std::move(distributedWork[fd]));
               distributedWork.erase(fd);
            }
         };

         if (pollFd.fd == listener) {
            // Incoming connection -> accept
            auto addr = sockaddr();
            socklen_t addrLen = sizeof(sockaddr);
            auto newConnection = accept(listener, &addr, &addrLen);
            if (newConnection == -1) {
               perror("accept() failed");
               continue;
            }
            pollFds.push_back(pollfd{
               .fd = newConnection,
               .events = POLLIN,
               .revents = {},
            });

            // And directly assign some work
            assignWork(newConnection);
            continue;
         }

         auto handleClientFailure = [&] {
            // Worker failed. Make sure the work gets done by someone else.
            if (distributedWork.contains(pollFd.fd)) {
               initialPartition.push_back(std::move(distributedWork[pollFd.fd]));
               distributedWork.erase(pollFd.fd);
            }
            // Drop the connection
            close(pollFd.fd); // Bye!
            std::swap(pollFds[index], pollFds.back());
            pollFds.pop_back();
            --index;
            --limit;
         };

         // Client is ready -> recv result and send more work
         auto buffer = std::array<char, 32>();
         auto numBytes = recv(pollFd.fd, buffer.data(), buffer.size(), 0);
         
         if (numBytes <= 0) {
            if (numBytes < 0)
               perror("recv() failed: ");
            handleClientFailure();
            continue;
         }

         // Parse response
         auto response = std::string_view(buffer.data(), static_cast<size_t>(numBytes));
         size_t clientResult = 0;
         auto [_, ec] = std::from_chars(response.data(), response.data() + response.size(), clientResult);
         if (ec != std::errc()) {
            handleClientFailure();
            continue;
         }

         // Result ok
         result += clientResult;
         distributedWork.erase(pollFd.fd);

         // Assign more work
         assignWork(pollFd.fd);
      }
   }
   std::cout << result << std::endl;

   // Cleanup
   for (auto& pollFd : pollFds)
      close(pollFd.fd);


   return 0;
}