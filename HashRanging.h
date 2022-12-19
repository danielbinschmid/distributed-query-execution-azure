#include "config.h"
#pragma once

class HashInt {
    private:
        

    public:
        HashInt();
        HashInt(size_t hash);
        ~HashInt();

        void init(size_t hash);
        size_t hash;
        std::string hashString;
        double semanticValue;


        bool operator<(const HashInt & other) const;
        bool operator=(const HashInt & other) const;
};

struct DomainAndCount {
    size_t count;
    std::string domain;
};

typedef std::map<HashInt, DomainAndCount> OccurencesMap;


struct ResultSubPartition {
    OccurencesMap partitionData;
    std::string filename;    
};


class HashRanging {
    private:
        static const HashMethod hashmethod = config::hashmethod; 

        void urlToDomain(std::string url, std::string &result);

        int assignToPartition(HashInt &hash);
    public:
        HashRanging();
        ~HashRanging();


        void hashDomainOfUrl(std::string url, HashInt &result, std::string &domainResult);

        void getSubPartitionFilenames(int partitionIdx, std::vector<std::string> &result);

        void splitIntoSubPartitions(int partitionIdx, OccurencesMap map, std::vector<ResultSubPartition> &result);

};