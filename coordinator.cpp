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

   std::vector<std::string> filesTodo;
   filesTodo.reserve(100);
   tools::coordinator::getInitialPartitionsLocalFiles(argv[1], filesTodo);

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


   return 0;
}