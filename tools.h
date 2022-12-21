#include <vector>
#include <string>
#include <map>
#include "HashRanging.h"
#pragma once
struct CountPartitionTask {
    int partitionIdx;
    std::string url;
};

class CountKey {
    private:

    public:
        int count;
        std::string domain;

        CountKey();

        CountKey(int count, std::string domain);

        bool operator<(const CountKey &other) const;

        bool operator==(const CountKey &other) const;
};


enum TaskType {COUNT, MERGE_SORT, NONE};

namespace tools {

    void getSubPartitionTop25Filename(int subPartitionIdx, std::string &result);

    namespace coordinator {
        int getListenerSocket(char* port);

        void getInitialPartitionsAzure(char* filename, std::vector<std::string> &todoOutput);

        void getInitialPartitionsLocalFiles(char* pathToCsv, std::vector<std::string> &todoOutput);

        void getInitialPartitionsHttp(char* pathToCsv,  std::vector<std::string> &todoOutput);

        std::string getResultFilename(int index);

        void serializeCountPartitionTask(CountPartitionTask task, std::string &result);

        void serializeMergeSortTask(MergeSortTask task, std::string &result);
        
    }


    namespace worker {
        size_t countGoogleRuOccurrences(std::string_view url);

        TaskType deserializeTaskBuffer(std::string buffer, CountPartitionTask &countTask, MergeSortTask &mergeSortTask);


        void getOccurencesMap(std::string_view url, OccurencesMap &result);

        void storeOccurencesMapToDisk(int partitionIdx, OccurencesMap map);

        void storeTop25ToDisk(int subPartitionIndex, SortedOccurencesMap top25);
        
        void mergeSort(int subPartitionIdx, SortedOccurencesMap &result);

    } 
}