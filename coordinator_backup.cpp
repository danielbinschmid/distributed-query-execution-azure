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


// Your settings


// ...

using namespace std::chrono;


// Return a listening socket
int getListenerSocket(char* port) {
   addrinfo hints = {};
   hints.ai_flags = AI_PASSIVE;
   hints.ai_family = AF_INET6;
   hints.ai_socktype = SOCK_STREAM;

   // Get us a socket and bind it
   addrinfo* aInfo = nullptr;
   if (auto status = getaddrinfo(nullptr, port, &hints, &aInfo); status != 0) {
      std::cerr << "getaddrinfo() failed: " << gai_strerror(status) << std::endl;
      exit(1);
   }

   int listener;
   addrinfo* iter;
   for (iter = aInfo; iter; iter = iter->ai_next) {
      listener = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
      if (listener < 0)
         continue;

      // Lose the pesky "address already in use" error message
      int optval = 1;
      setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

      if (bind(listener, iter->ai_addr, iter->ai_addrlen) < 0) {
         close(listener);
         continue;
      }

      break;
   }

   freeaddrinfo(aInfo); // All done with this

   // If we got here, it means we didn't get bound
   if (!iter) {
      std::cerr << "could not bind() to: " << port << std::endl;
      exit(1);
   }

   // Listen
   if (listen(listener, 128) == -1) {
      perror("listen() failed");
      exit(1);
   }

   return listener;
}

milliseconds getTimeSinceStart(milliseconds startTime, std::string message) {
   milliseconds downloadedTime = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
   );
   std::cout << message << (double) (downloadedTime.count() - startTime.count()) / 1000.0 << " seconds" << std::endl;

   return downloadedTime;
}

void getFilesTodoHttp(char* pathToCsv,  std::vector<std::string> todoOutput) {
   auto curlSetup = CurlGlobalSetup();

   auto listUrl = std::string(pathToCsv);

   // Download the file list
   auto curl = CurlEasyPtr::easyInit();
   curl.setUrl(listUrl);
   auto fileList = curl.performToStringStream();

   for (std::string url; std::getline(fileList, url, '\n');)
      todoOutput.push_back(std::move(url));
}

void getFilesTodo(char* pathToCsv, std::vector<std::string> &todoOutput) {
   auto curlSetup = CurlGlobalSetup();
   auto curl = CurlEasyPtr::easyInit();

   std::fstream fs;
   fs.open(std::string(pathToCsv));
   
   while(!fs.eof()) {
      std::string output;
      std::getline(fs, output);
      if (output.size() == 0) break;
      todoOutput.push_back(std::move(output));
      
   }

   
   fs.close();
}


void getFilesTodoAzureBlob() {
   std::string account_name = "tripleheadhydra";
   std::string account_key = "og7nuOD08FqrH9SCwwKkL7c6ApPtFvIMx2rm4V1r4K+ACRAGMvAE";
   bool use_https = true;
   std::string blob_endpoint = "CUSTOMIZED_BLOB_ENDPOINT";
   int connection_count = 2;

   // Setup the client
   auto credential = std::make_shared<azure::storage_lite::shared_key_credential>(account_name, account_key);
   auto storage_account = std::make_shared<azure::storage_lite::storage_account>(account_name, credential, use_https, blob_endpoint);
   azure::storage_lite::blob_client client(storage_account, connection_count);

   // Start using
   auto outcome = client.create_container("cbdp_container").get();
}

/// Leader process that coordinates workers. Workers connect on the specified port
/// and the coordinator distributes the work of the CSV file list.
/// Example:
///    ./coordinator http://example.org/filelist.csv 4242
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <URL to csv list> <listen port>" << std::endl;
      return 1;
   }

   milliseconds coordinatorStartTime = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
   );
   std::cout << "Start coordinator" << std::endl;


   // --------------- AZURE STUFF -----------------

   static const std::string accountName = "";
   static const std::string accountToken = "";
   auto blobClient = AzureBlobClient(accountName, accountToken);

   std::cerr << "Creating Azure blob container" << std::endl;
   blobClient.createContainer("cbdp-assignment4");

   std::vector<std::string> blobs = blobClient.listBlobs();

   

   std::cerr << "Deleting the container" << std::endl;
   blobClient.deleteContainer();


   // ---------------------------------------------




   std::vector<std::string> filesTodo;
   filesTodo.reserve(100);
   getFilesTodo(argv[1], filesTodo); // getFilesTodoHttp(argv[1], filesTodo);
   
   std::cout << filesTodo[0] << std::endl;
   milliseconds downloadedTime = getTimeSinceStart(coordinatorStartTime, "Downloaded after ");

   // Listen
   auto listener = getListenerSocket(argv[2]);

   // Setup polling for new connections and worker responses
   std::vector<pollfd> pollFds;
   pollFds.push_back(pollfd{
      .fd = listener,
      .events = POLLIN,
      .revents = {},
   });

   // Distribute the work
   size_t result = 0;
   auto distributedWork = std::unordered_map<int, std::string>();
   while (!filesTodo.empty() || !distributedWork.empty()) {
      poll(pollFds.data(), pollFds.size(), -1);
      for (size_t index = 0, limit = pollFds.size(); index != limit; ++index) {
         const auto& pollFd = pollFds[index];
         // Look for ready connections
         if (!(pollFd.revents & POLLIN)) continue;

         auto assignWork = [&](int fd) {
            // Assign work (if any)
            if (filesTodo.empty()) return;
            distributedWork[fd] = std::move(filesTodo.back());
            filesTodo.pop_back();

            const auto& file = distributedWork[fd];
            if (auto status = send(fd, file.c_str(), file.size(), 0); status == -1) {
               perror("send() failed");
               

               filesTodo.push_back(std::move(distributedWork[fd]));
               distributedWork.erase(fd);
            }
            milliseconds recvTime = getTimeSinceStart(coordinatorStartTime, "Sending work after ");
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
               filesTodo.push_back(std::move(distributedWork[pollFd.fd]));
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
   milliseconds resTime = getTimeSinceStart(coordinatorStartTime, "Result after ");
   std::cout << result << std::endl;

   // Cleanup
   for (auto& pollFd : pollFds)
      close(pollFd.fd);

   return 0;
}