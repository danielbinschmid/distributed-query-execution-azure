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
#include "credentials.h"


/// Worker process that receives a list of URLs and reports the result
/// Example:
///    ./worker localhost 4242
/// The worker then contacts the leader process on "localhost" port "4242" for work
int main(int argc, char* argv[]) {
   if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
   }

   
   return 0;
}

