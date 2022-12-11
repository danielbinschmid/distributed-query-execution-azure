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
namespace fs = std::filesystem;
/**
 * 
*/
#include <chrono>

using namespace std::chrono;

milliseconds getTimeSinceStart(milliseconds startTime, std::string message) {
   milliseconds downloadedTime = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
   );
   std::cout << message << (double) (downloadedTime.count() - startTime.count()) / 1000.0 << " seconds" << std::endl;

   return downloadedTime;
}

std::stringstream getCsvHTTP(CurlEasyPtr& curl, std::string url) {
   curl.setUrl(url);
   auto csvData = curl.performToStringStream();

   return csvData;
}


size_t processUrl(CurlEasyPtr& curl, std::string_view url) {
   using namespace std::literals;
   size_t result = 0;
   // Download the file

   milliseconds downloadStartTime = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
   );
   std::string filename = fs::path(std::string(url)).filename();
   std::string filepath = "files/" + filename;
   curl.easyInit();
   // auto csvData = getCsvHTTP(curl, std::string(url));
   std::fstream csvData;
   csvData.open(filepath);
   
   getTimeSinceStart(downloadStartTime, "time needed to download csv: ");

   for (std::string row; std::getline(csvData, row, '\n');) {
      auto rowStream = std::stringstream(std::move(row));

      // Check the URL in the second column
      unsigned columnIndex = 0;
      for (std::string column; std::getline(rowStream, column, '\t'); ++columnIndex) {
         // column 0 is id, 1 is URL
         if (columnIndex == 1) {
            // Check if URL is "google.ru"
            auto pos = column.find("://"sv);
            if (pos != std::string::npos) {
               auto afterProtocol = std::string_view(column).substr(pos + 3);
               if (afterProtocol.starts_with("google.ru/"))
                  ++result;
            }
            break;
         }
      }
   }
   return result;
}

/// Worker process that receives a list of URLs and reports the result
/// Example:
///    ./worker localhost 4242
/// The worker then contacts the leader process on "localhost" port "4242" for work
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
   }
   std::cout << "Start worker" << std::endl;
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
   for (unsigned i = 0; i < 10; ++i) {
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
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
   }
breakConnect:
   freeaddrinfo(coordinatorAddr);
   if (status == -1) {
      perror("connect() failed");
      return 1;
   }

   // Connected
   auto curlSetup = CurlGlobalSetup();
   auto curl = CurlEasyPtr::easyInit();
   auto buffer = std::array<char, 1024>();
   while (true) {
      auto numBytes = recv(connection, buffer.data(), buffer.size(), 0);
      if (numBytes <= 0) {
         // connection closed / error
         break;
      }
      auto url = std::string_view(buffer.data(), static_cast<size_t>(numBytes));
      auto result = processUrl(curl, url);

      auto response = std::to_string(result);
      if (send(connection, response.c_str(), response.size(), 0) == -1) {
         perror("send() failed");
         break;
      }
   }

   close(connection);
   return 0;
}

