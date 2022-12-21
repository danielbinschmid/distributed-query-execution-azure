#include "CurlEasyPtr.h"
#include <array>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <netdb.h>
#include <sys/socket.h>
#include <filesystem>
#include <fstream>
#include "AzureBlobClient.h"
#include "config.h"
#include "tools.h"
#define N_BYTES_RECEIVED 1024
#define RECONNECT_PATIENCE 500 // in ms
#define N_CONNECT_TRIES 500
namespace fs = std::filesystem;




/// Worker process that receives a list of URLs and reports the result
/// Example:
///    ./worker localhost 4242
/// The worker then contacts the leader process on "localhost" port "4242" for work
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
   }

   // ------------------------------- CONNECT TO COORDINATOR ---------------------------------
   // Set up the connection
   addrinfo hints = {};
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   addrinfo* coordinatorAddr = nullptr;
   if (auto status = getaddrinfo(argv[1], argv[2], &hints, &coordinatorAddr); status != 0) {
      std::cerr << "getaddrinfo() failed: " << gai_strerror(status) << std::endl;
      return 1;
   }

   // Try to connect to coordinator
   int connection, status;
   for (unsigned i = 0; i < N_CONNECT_TRIES; ++i) {
      for (auto iter = coordinatorAddr; iter; iter = iter->ai_next) {
         connection = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
         if (connection == -1) {
            std::cerr << "socket() failed: " << strerror(connection) << std::endl;
            return 1;
         }
         status = connect(connection, iter->ai_addr, iter->ai_addrlen);
         if (status != -1)
            goto breakConnect;
         close(connection);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_PATIENCE));
   }
   // ------------------------------------------------------------------------------------------


breakConnect:
   freeaddrinfo(coordinatorAddr);
   if (status == -1) {
      perror("connect() failed");
      return 1;
   }

   // Connected
   auto buffer = std::array<char, N_BYTES_RECEIVED>();
   while (true) {
      auto numBytes = recv(connection, buffer.data(), buffer.size(), 0);
      if (numBytes <= 0) {
         // connection closed / error
         break;
      }

      // Get and deserialize incoming task-buffer
      auto taskSerialized = std::string(buffer.data(), static_cast<size_t>(numBytes));
      CountPartitionTask countTask;
      MergeSortTask mergeSortTask;
      TaskType tasktype = tools::worker::deserializeTaskBuffer(taskSerialized, countTask, mergeSortTask);

      // do task
      auto result = 0;
      OccurencesMap occurrencesMap;
      SortedOccurencesMap top25;
      switch(tasktype) {
         case TaskType::COUNT: 
            if (config::logging) std::cout << countTask.url << std::endl;
            tools::worker::getOccurencesMap(countTask.url, occurrencesMap);
            tools::worker::storeOccurencesMapToDisk(countTask.partitionIdx, occurrencesMap);
            result = 1;
            break;
         
         case TaskType::MERGE_SORT:
            if (config::logging) std::cout << "SubPartition: " << mergeSortTask.subPartitionIdx << std::endl;
            tools::worker::mergeSort(mergeSortTask.subPartitionIdx, top25);
            tools::worker::storeTop25ToDisk(mergeSortTask.subPartitionIdx, top25);
            result = 1;
            break;
         default:
            std::cerr << "task type not known to worker" << std::endl; 
      }

      auto response = std::to_string(result);
      if (send(connection, response.c_str(), response.size(), 0) == -1) {
         perror("send() failed");
         break;
      }
   }

   close(connection);
   return 0;
}

