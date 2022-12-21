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
#include <unistd.h>
#include "AzureBlobClient.h"
#include "config.h"

#define TRUE 1
#define FALSE 0
#define DEBUG TRUE

namespace fs = std::filesystem;

void check_error(int code, const std::string& message) {
   if (code < 0) {
      fprintf(stderr, "[ERROR] (worker): %s\n", message.c_str());
      exit(1);
   }
}

void log(int code, const std::string& message) {
   if (code < 0) {
      printf("[INFO] (worker): %s\n", message.c_str());
   } else {
      printf("[WARN] (worker): %s\n", message.c_str());
   }
}

size_t processUrl(CurlEasyPtr& curl, std::string_view url) {
   using namespace std::literals;
   size_t result = 0;
   // Download the file

   std::string filename = fs::path(std::string(url)).filename();
   std::string filepath = "files/" + filename;
   curl.easyInit();
   // auto csvData = getCsvHTTP(curl, std::string(url));
   std::fstream content;
   content.open(filepath);

   
   for (std::string row; std::getline(content, row, '\n');) {
      auto rowStream = std::stringstream(std::move(row));

      // Check the URL in the second column
      unsigned columnIndex = 0;
      for (std::string column; std::getline(rowStream, column, '\t'); ++columnIndex) {
         // column 0 is id, 1 is URL
         if (columnIndex == 1) {
            // Check if URL is "google.ru"

            // TODO: replace with counting the URLs

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

   auto status = getaddrinfo(argv[1], argv[2], &hints, &coordinatorAddr);
   check_error((int) status, "getaddrinfo failed");

   // Try to connect to coordinator
   int connection;
   for (unsigned i = 0; i < 10; ++i) {
      for (auto iter = coordinatorAddr; iter; iter = iter->ai_next) {
         connection = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
         check_error(connection, "socket() failed");;

         status = connect(connection, iter->ai_addr, iter->ai_addrlen);
         if (status != -1)
            goto breakConnect;
         close(connection);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
   }

breakConnect:
   freeaddrinfo(coordinatorAddr);
   check_error(status, "connection() failed");

   // Connected
   auto curlSetup = CurlGlobalSetup();
   auto curl = CurlEasyPtr::easyInit();
   auto buffer = std::array<char, 1024>();
   
   auto numBytes = recv(connection, buffer.data(), buffer.size(), 0);
   check_error((int) numBytes, "recv() failed");

   while (numBytes > 0) {
      auto url = std::string_view(buffer.data(), static_cast<size_t>(numBytes));
      auto result = processUrl(curl, url);

      auto response = std::to_string(result);
      auto bytesSent = send(connection, response.c_str(), response.size(), 0);
      check_error((int) bytesSent, "send() failed");

      numBytes = recv(connection, buffer.data(), buffer.size(), 0);
      check_error((int) numBytes, "recv() failed");
   }

   close(connection);
   return 0;
}

