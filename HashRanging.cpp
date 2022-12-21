#include <boost/functional/hash.hpp>
#include "HashRanging.h"
#include <iostream>

HashInt::HashInt(size_t hash) {
    this->init(hash);
}

HashInt::HashInt() {
}

void HashInt::init(size_t hash) {
    this->hash = hash;
    this->hashString = std::to_string((size_t) hash);
    std::string temp = "0."  + this->hashString;
    this->semanticValue = std::stod(temp);
}

HashInt::~HashInt() {

}

bool HashInt::operator==(const HashInt &other) const {
    return this->hash == other.hash;
}

bool HashInt::operator<(const HashInt &other) const {
    if (this->hash == other.hash) return false;

    int i = 0;
    while(other.hashString[i] == this->hashString[i] && i < (int) other.hashString.size() && i < (int) this->hashString.size()) i++;


    int a = (int) this->hashString[i] + '0';
    int b = (int) other.hashString[i] + '0';

    if (a != b) {
        return a < b;
    } else{
        // last digit is also the same. The longer idstring is bigger
        return this->hashString.size() < other.hashString.size();
    }
}




void HashRanging::hashDomainOfUrl(std::string url, HashInt &result, std::string &domainResult) {
    if (this->hashmethod != HashMethod::BOOST) {
        std::cerr << "Hash method not implemented" << std::endl;
        return;
    } 

    this->urlToDomain(url, domainResult);

    boost::hash<std::string> hasher;
    size_t hash = hasher(domainResult); // boost::hash_value(domain); 

    std::string tmp = std::to_string(hash);
    tmp = tmp.substr(1, tmp.size() - 1);
    hash = (size_t) std::atoll(tmp.c_str());

    result.init(hash);
    
    if (config::logging) {
        if (hash == 0) {
           if (config::logging) {
              std::cout << "Url: " << url << "; was converted to domain: " << domainResult << "; and resulted in hash: " << hash << std::endl;
              std::cout << std::endl;
           }
        }
        
    }
    
}

void HashRanging::urlToDomain(std::string url, std::string &domainResult) {
    bool beginningFound = false;
    domainResult.reserve(url.size());

    int beginningFoundCount = 0;
    std::string beginningString = "://";
    for (char const &c: url) {
        if (beginningFound) {
            if (c == '/') {
                break;
            } else {
                domainResult.push_back(c);        
            }
        } else {

            if (c == beginningString[beginningFoundCount]) {
                beginningFoundCount ++;
            } else {
                beginningFoundCount = 0;
            }
                

            if (beginningFoundCount == 3) {
                beginningFound = true;
            } 
        }
    }

    if (!beginningFound && beginningFoundCount < 3)
        domainResult = url;
    
}

int HashRanging::assignToPartition(HashInt &hash) {
    int partitionIdx = (int) (((hash.semanticValue * 10.0) - 1.0) * ((double)(config::nAggregates ) / 9.0));

    if (partitionIdx < 0) 
        partitionIdx = 0;
    // std::cout << hash.semanticValue << " HUH " << partitionIdx << std::endl;
    return partitionIdx;
}


void HashRanging::getSubPartitionFilenames(int partitionIdx, std::vector<std::string> &result) {
    for (int i = 0; i < config::nAggregates; i++) {
        std::string fname = "files/partitions/partition-" + std::to_string(partitionIdx) + "." + std::to_string(i) + ".csv";
        result.push_back(fname);
    }
}

void HashRanging::getMergeSortTasksFilenames(int subPartitionIdx, std::vector<std::string> &result) {
    for (int i = 0; i < config::nInitialPartitions; i++) {
        std::string fname = "files/partitions/partition-" + std::to_string(i) + "." + std::to_string(subPartitionIdx) + ".csv";
        result.push_back(fname);
    }
}


void HashRanging::splitIntoSubPartitions(int partitionIdx, OccurencesMap map, std::vector<ResultSubPartition> &result) {
    std::vector<std::string> subPartitionFilenames;
    this->getSubPartitionFilenames(partitionIdx, subPartitionFilenames);

    // initialize subpartitions
    for (int i = 0; i < config::nAggregates; i++) {
        ResultSubPartition resPart;
        resPart.filename = subPartitionFilenames[i];
        resPart.partitionData = OccurencesMap();
        result.push_back(resPart);
    }

    // fill subpartitions with values
    int i = 0;
    for(auto it = map.begin(); it != map.end(); it++) {
        HashInt hash = HashInt(it->first.hash);
        int subPartitionIdx = this->assignToPartition(hash);        
        result[subPartitionIdx].partitionData.insert(std::make_pair(it->first, it->second));
        i++;
    }

    
}

HashRanging::HashRanging() {

}
HashRanging::~HashRanging() {

}

void HashRanging::getInitialMergeSortTasks(std::vector<MergeSortTask> &result) {
    result.reserve(config::nAggregates);
    for (int i = 0; i < config::nAggregates; i++) {
        MergeSortTask task;
        task.subPartitionIdx = i;
        result.push_back(task);
    }
}