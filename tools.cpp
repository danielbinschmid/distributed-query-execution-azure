#include "tools.h"

#include <iostream>

#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include "HashRanging.h"
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

#define DELIMITER ';'

void tools::coordinator::serializeCountPartitionTask(CountPartitionTask task, std::string &result) {
    // first 
    result = std::to_string(task.partitionIdx) + DELIMITER + task.url; 
}


void tools::worker::deserializeCountPartitionTask(std::string buffer, CountPartitionTask &task) {
    int i = 0;
    while (buffer[i] != DELIMITER) i++;
    
    task.partitionIdx = std::stoi(buffer.substr(0, i));

    task.url = buffer.substr(i, buffer.size() - i);
}
        


