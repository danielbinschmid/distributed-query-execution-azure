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
#include <stdlib.h>
#include "storage_credential.h"
#include "storage_account.h"
#include "blob/blob_client.h"


#include "AzureBlobClient.h"
#include "config.h"
#include "tools.h"
#include "HashRanging.h"
#include "config.h"
#include "Polling.h"

/**
 * Leader process that coordinates workers. 
 * 
 * Workers connect on the specified port
 * and the coordinator distributes the work of the CSV file list
 * Example:
 *    ./coordinator http://example.org/filelist.csv 4242 
*/

struct ResultPerFile
{
   std::string domain;
   int count;
};

void insertDomainCountInRightPlace(std::vector<ResultPerFile>& results, ResultPerFile result) {
   int index = 0;
   while ((size_t) index < results.size() && results[(size_t) index++].count > result.count);
   results.insert(results.begin() + index - 1, result);
   results.pop_back();
}

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

   PollLoops polling;
   polling.init(listener, initialPartition);
   polling.pollLoop();

   std::cout << polling.result << std::endl;
   polling.closePolling();

   const std::string accountName = config::credentials::accountName;
   const std::string accountToken = config::credentials::accountToken;

   AzureBlobClient client = AzureBlobClient(accountName, accountToken);
   
   std::vector<ResultPerFile> orderedResult;
   orderedResult.reserve(25);

   std::string domain;
   int count;

   for (int i = 1; i <= 10; i++)
   {
      const std::string filename = tools::coordinator::getResultFilename(i);
      std::stringstream result = client.downloadStringStream(filename);

      while (result >> domain) {
         result >> count;
         ResultPerFile res;
         res.domain = domain;
         res.count = count;
         insertDomainCountInRightPlace(orderedResult, res);
      }
   }
   
   return 0;
}