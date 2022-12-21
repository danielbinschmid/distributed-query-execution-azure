
#include <iostream>

#include "config.h"
#include "tools.h"
#include "HashRanging.h"
#include "config.h"
#include "Polling.h"


#include <chrono>
using namespace std::chrono;


milliseconds getTimeSinceStart(milliseconds startTime, std::string message) {
   milliseconds downloadedTime = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
   );
   std::cout << message << (double) (downloadedTime.count() - startTime.count()) / 1000.0 << " seconds" << std::endl;

   return downloadedTime;
}
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
   
   milliseconds coordinatorStartTime = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
   );
   if (config::time_measures_logging) {
      std::cout << "Start coordinator" << std::endl;
   }


   std::vector<std::string> initialPartition;
   initialPartition.reserve(100);
   tools::coordinator::getInitialPartitionsLocalFiles(argv[1], initialPartition);
   std::cout << "Initial partitions metadata downloaded. First file is: "<<initialPartition[0] << std::endl;
   auto listener = tools::coordinator::getListenerSocket(argv[2]);

   if (config::time_measures_logging) {
      getTimeSinceStart(coordinatorStartTime, "Starting counting at ");
   }
   // counting
   CountLoop counting;
   counting.init(listener, initialPartition);
   counting.startTime = coordinatorStartTime;
   counting.countLoop();

   if (config::time_measures_logging) {
      getTimeSinceStart(coordinatorStartTime, "Finished counting at ");
   }

   // merge sort
   MergeSortLoop mergeSortLoop(counting);
   mergeSortLoop.run();
   mergeSortLoop.closePolling();
   if (config::logging) {
      std::cout << counting.result << std::endl;
      std::cout << mergeSortLoop.result << std::endl;
   }

   if (config::time_measures_logging) {
      getTimeSinceStart(coordinatorStartTime, "Finished merge sort from worker side at ");
   }

   SortedOccurencesMap finalTop25;
   tools::coordinator::finalMerge(finalTop25);

   tools::storeTop25ToDisk("result.csv", finalTop25);
   
   if (config::time_measures_logging) {
      getTimeSinceStart(coordinatorStartTime, "Finished whole query at ");
   }
   // final merging

   
   return 0;
}