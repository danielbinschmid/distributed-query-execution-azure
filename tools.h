#include <vector>
#include <string>
#include <map>
#include "HashRanging.h"
#pragma once
struct CountPartitionTask {
    int partitionIdx;
    std::string url;
};

namespace tools {

    namespace coordinator {
        int getListenerSocket(char* port);

        void getInitialPartitionsAzure(char* filename, std::vector<std::string> &todoOutput);


        void getInitialPartitionsLocalFiles(char* pathToCsv, std::vector<std::string> &todoOutput);

        void getInitialPartitionsHttp(char* pathToCsv,  std::vector<std::string> &todoOutput);

        void serializeCountPartitionTask(CountPartitionTask task, std::string &result) ;
    }


    namespace worker {
        size_t countGoogleRuOccurrences(std::string_view url);


        void deserializeCountPartitionTask(std::string buffer, CountPartitionTask &task);

        void getOccurencesMap(std::string_view url, OccurencesMap &result);

        void storeOccurencesMapToDisk(int partitionIdx, OccurencesMap map);
        
    } 
}