#include "tools.h"

#include <iostream>

#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include "HashRanging.h"
#include <unordered_map>
#include <algorithm>
#include "config.h"

namespace fs = std::filesystem;

// ------------------------------- COORDINATOR -------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

int tools::coordinator::getListenerSocket(char* port) {
    addrinfo hints = {};
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    // Get us a socket and bind it
    addrinfo* aInfo = nullptr;
    if (auto status = getaddrinfo(nullptr, port, &hints, &aInfo); status != 0) {
        std::cerr << "getaddrinfo() failed: " << gai_strerror(status) << std::endl;
        exit(1);
    }

    int listener;
    addrinfo* iter;
    for (iter = aInfo; iter; iter = iter->ai_next) {
        listener = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
        if (listener < 0)
            continue;

        // Lose the pesky "address already in use" error message
        int optval = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        if (bind(listener, iter->ai_addr, iter->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(aInfo); // All done with this

    // If we got here, it means we didn't get bound
    if (!iter) {
        std::cerr << "could not bind() to: " << port << std::endl;
        exit(1);
    }

    // Listen
    if (listen(listener, 128) == -1) {
        perror("listen() failed");
        exit(1);
    }

    return listener;   
}


void tools::coordinator::getInitialPartitionsAzure(char* pathToCsv,  std::vector<std::string> &todoOutput) {
    pathToCsv[0] = '\0';
    todoOutput.at(0);
    std::cerr << "not implemented" << std::endl;
}

void tools::coordinator::getInitialPartitionsLocalFiles(char* pathToCsv, std::vector<std::string> &todoOutput) {
    std::fstream fs;
    fs.open(std::string(pathToCsv));

    while(!fs.eof()) {
        std::string output;
        std::getline(fs, output);
        if (output.size() == 0) break;
        todoOutput.push_back(std::move(output));
        
    }


    fs.close();
}

void tools::coordinator::getInitialPartitionsHttp(char* pathToCsv, std::vector<std::string> &todoOutput) {
    pathToCsv[0] = '\0';
    todoOutput.at(0);
    std::cerr << "not implemented" << std::endl;
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------


// -----------------------------------  WORKER  ------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

size_t tools::worker::countGoogleRuOccurrences(std::string_view url) {
    using namespace std::literals;
    size_t result = 0;
    // Download the file

    std::string filename = fs::path(std::string(url)).filename();
    std::string filepath = "files/" + filename;

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



void tools::worker::getOccurencesMap(std::string_view url, OccurencesMap &result) {    
    using namespace std::literals;
    std::string filename = fs::path(std::string(url)).filename();
    std::string filepath = "files/" + filename;

    std::fstream content;
    content.open(filepath);

    HashRanging hashranging;


    for (std::string row; std::getline(content, row, '\n');) {
        auto rowStream = std::stringstream(std::move(row));

        // Check the URL in the second column
        unsigned columnIndex = 0;
        for (std::string column; std::getline(rowStream, column, '\t'); ++columnIndex) {
            // column 0 is id, 1 is URL
            if (columnIndex == 1) {
                // parse url to domain
                HashInt hash;
                std::string domain;
                hashranging.hashDomainOfUrl(column, hash, domain);

                auto pos = result.find(hash);
                if (pos == result.end()) {
                    // insert domain
                    DomainAndCount domainCount;
                    domainCount.count = 1;
                    domainCount.domain = domain;
                    result.insert(std::make_pair(hash, domainCount));
                } else {
                    // increment count of domain
                    pos->second.count++;
                    result.insert_or_assign(pos->first, pos->second);
                }
            }
        }
    }
}


void subPartitionToLocalFile(ResultSubPartition partition) {
    std::ofstream outfile;
    outfile.open(partition.filename, std::ofstream::out | std::ofstream::trunc);

    for (auto it = partition.partitionData.begin(); it != partition.partitionData.end(); it++)
        outfile << it->first.hashString << "\t" << std::to_string(it->second.count) << "\t" << it->second.domain << std::endl;

    outfile.close();
}


void tools::worker::storeOccurencesMapToDisk(int partitionIdx, OccurencesMap map) {
    HashRanging hashranging;

    std::vector<ResultSubPartition> partitions;
    hashranging.splitIntoSubPartitions(partitionIdx, map, partitions);

    for (const auto& partition: partitions) {
        subPartitionToLocalFile(partition);
    }

}



// -------------------------------- SERIALIZATION & DESERIALIZATION -----------------------------------

#define DELIMITER ';'
#define DELIMITER_TASKTYPE '|'

void tools::coordinator::serializeCountPartitionTask(CountPartitionTask task, std::string &result) {
    // first 
    result = std::to_string(TaskType::COUNT) + DELIMITER_TASKTYPE + std::to_string(task.partitionIdx) + DELIMITER + task.url; 
}

void tools::coordinator::serializeMergeSortTask(MergeSortTask task, std::string &result) {
    result = std::to_string(TaskType::MERGE_SORT) + DELIMITER_TASKTYPE + std::to_string(task.subPartitionIdx) + DELIMITER;
}

void deserializeCountPartitionTask(std::string buffer, CountPartitionTask &task) {
    int i = 0;
    while (buffer[i] != DELIMITER) i++;
    
    task.partitionIdx = std::stoi(buffer.substr(0, i));


    task.url = buffer.substr(i, buffer.size() - i);
}

void deserializeMergeSortTask(std::string buffer, MergeSortTask &taskResult) {
    int i = 0;
    while (buffer[i] != DELIMITER) i++;
    
    
    taskResult.subPartitionIdx = std::stoi(buffer.substr(0, i));
}

TaskType tools::worker::deserializeTaskBuffer(std::string buffer, CountPartitionTask &countTask, MergeSortTask &mergeSortTask) {
    int i = 0;
    while (buffer[i] != DELIMITER_TASKTYPE) i++;
    
    switch(std::stoi(buffer.substr(0, i))) {
        case TaskType::MERGE_SORT:
            if (LOGGING) {
                std::cout << "Deserialized task: " << buffer.substr(i + 1, buffer.size() - i - 1) << std::endl;
            }
            deserializeMergeSortTask(buffer.substr(i + 1, buffer.size() - i - 1), mergeSortTask);
            return TaskType::MERGE_SORT;
            
        case TaskType::COUNT:
            deserializeCountPartitionTask(buffer.substr(i + 1, buffer.size() - i - 1), countTask);
            return TaskType::COUNT;

            break;
        default:
            std::cerr << "Unknown task type received" << std::endl;
            return TaskType::NONE;
    }


}




CountKey::CountKey(int count, std::string domain) {
    this->domain = domain;
    this->count = count;
}

CountKey::CountKey() {
    this->count = -1;
    this->domain = "";
}

bool CountKey::operator<(const CountKey &other) const {
    return this->count < other.count;
}

bool CountKey::operator==(const CountKey &other) const {
    return this->domain == other.domain;
}


void tools::worker::mergeSort(int subPartitionIdx, SortedOccurencesMap &result) {
    HashRanging hashranging;
    std::vector<std::string> filenames;
    hashranging.getMergeSortTasksFilenames(subPartitionIdx, filenames);
    std::unordered_map<std::string, int> countMap;
    
    for (const auto& filename: filenames) {
        std::fstream content;
        content.open(filename);

        for (std::string row; std::getline(content, row, '\n');) {
            auto rowStream = std::stringstream(std::move(row));
            std::string domain;
            int count;
            // Check the domain-count in the second column. Domain is in the third column
            unsigned columnIndex = 0;
            for (std::string column; std::getline(rowStream, column, '\t'); ++columnIndex) {
                switch(columnIndex) {
                    case 0: // hashID
                        break;
                    case 1: // count
                        count = atoi(column.c_str());
                        break;
                    case 2: // domain
                        domain = column;
                        break;
                }
            }
            // get and update map entry or insert new map entry
            auto entry = countMap.find(domain);
            if (entry == countMap.end()) {
                // entry not existing
                countMap.insert(std::make_pair(domain, count));
            } else {
                // entry exists
                count += entry->second;
                countMap.insert_or_assign(domain, count);
            }
        }
    }
    std::vector<CountKey> vectorized;

    for(auto kv : countMap) {
        CountKey yy;
        yy.count = kv.second;
        yy.domain = kv.first;
        vectorized.push_back(yy);
    } 
    std::sort(vectorized.begin(), vectorized.end());

    // convert map to SortedOccurencesMap
    int topN = 25;
    result.reserve(topN);
    int i = 0;
    auto it = vectorized.end();
    it--;
    while (it != vectorized.begin()) {
        DomainAndCount domainAndCount;
        domainAndCount.count = it->count;
        domainAndCount.domain = it->domain;
        result.push_back(domainAndCount);
        i++;
        if (i == topN) break;
        it--;
    } 
}

void tools::getSubPartitionTop25Filename(int subPartitionIdx, std::string &result) {
    result = "files/top25/top25-" + std::to_string(subPartitionIdx) + ".csv";
}

void domainCountToLocalFile(ResultSubPartition partition) {
    std::ofstream outfile;
    if (LOGGING) {
        std::cout << "saving domain counts into " << partition.filename << std::endl;
    }
    outfile.open(partition.filename, std::ofstream::out | std::ofstream::trunc);

    for (auto it = partition.partitionData.begin(); it != partition.partitionData.end(); it++)
        outfile << it->first.hashString << "\t" << std::to_string(it->second.count) << "\t" << it->second.domain << std::endl;

    outfile.close();
}

void tools::storeTop25ToDisk(std::string filename, SortedOccurencesMap top25) {
    std::ofstream outfile;
    outfile.open(filename, std::ofstream::out | std::ofstream::trunc);

    for (const auto& domain: top25) {
        outfile << domain.domain << "\t" << std::to_string(domain.count) << std::endl;
    }

    outfile.close();
}

void tools::worker::storeTop25ToDisk(int subPartitionIdx, SortedOccurencesMap top25) {
    
    std::string filename;
    tools::getSubPartitionTop25Filename(subPartitionIdx, filename);
    tools::storeTop25ToDisk(filename, top25);
    
}

void insertIntoOrdered(std::vector<DomainAndCount>& results, DomainAndCount result) {
    int index = 0;
    while ((size_t) index < results.size() && results[(size_t) index].count > result.count) {
        index++;
    }
    SortedOccurencesMap::iterator target = results.begin();

  
    results.insert(target + index, result);
    if (results.size() > 25) results.pop_back();
    
    
    
    
}



void tools::coordinator::finalMerge(SortedOccurencesMap &result) {
    result.reserve(26);

    for (int i = 0; i < config::nAggregates; i++)
    {
        std::string filename;
        tools::getSubPartitionTop25Filename(i, filename);
        std::fstream content;
        content.open(filename);

        int rowCount = 0;
        for (std::string row; std::getline(content, row, '\n');) {
            auto rowStream = std::stringstream(std::move(row));
            DomainAndCount domainCount;
            unsigned columnIndex = 0;
            for (std::string column; std::getline(rowStream, column, '\t'); ++columnIndex) {
                switch (columnIndex)
                {
                case 0: // domain
                    domainCount.domain = column;
                    break;
                case 1: // count 
                    domainCount.count = std::atoi(column.c_str());
                    break;
                default:
                    break;
                }
            }
            insertIntoOrdered(result, domainCount);
            rowCount++;
        }

    }
}

