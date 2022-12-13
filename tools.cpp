#include "tools.h"

#include <iostream>

#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>
#include <fstream>
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

}
