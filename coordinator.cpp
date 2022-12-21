
#include <iostream>

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

   // counting
   CountLoop counting;
   counting.init(listener, initialPartition);
   counting.countLoop();

   // merge sort
   MergeSortLoop mergeSortLoop(counting);
   mergeSortLoop.run();
   mergeSortLoop.closePolling();
   if (LOGGING) {
      std::cout << counting.result << std::endl;
      std::cout << mergeSortLoop.result << std::endl;
   }

   SortedOccurencesMap finalTop25;
   tools::coordinator::finalMerge(finalTop25);

   tools::storeTop25ToDisk("result.csv", finalTop25);
   
   // final merging

   
   return 0;
}