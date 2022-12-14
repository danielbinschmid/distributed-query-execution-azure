#include "Polling.h"
#include <cstddef>
#include <netdb.h>
#include <iostream>
#include <array>
#include <unistd.h>
#include <charconv>

void Polling::initPolling(int listener) {

    this->pollFds.push_back(pollfd{
        .fd = listener,
        .events = POLLIN,
        .revents = {},
    });
    this->listener = listener;

};


void Polling::pollIteration(
                    std::function<CallbackReturn(int)> incomingConnectionCallback, 
                    std::function<CallbackReturn(int, int, std::array<char, 32>)> recvBytes, 
                    std::function<CallbackReturn(int)> workerFailureCallback) {
    poll(pollFds.data(), pollFds.size(), -1);

    for (size_t index = 0, limit = pollFds.size(); index != limit; ++index) {
        const auto& pollFd = pollFds[index];
            // Look for ready connections
        if (!(pollFd.revents & POLLIN)) continue;

        if (pollFd.fd == listener) {
            // Incoming connection -> accept
            auto addr = sockaddr();
            socklen_t addrLen = sizeof(sockaddr);
            auto newConnection = accept(listener, &addr, &addrLen);
            if (newConnection == -1) {
               perror("accept() failed");
               continue;
            }
            pollFds.push_back(pollfd{
               .fd = newConnection,
               .events = POLLIN,
               .revents = {},
            });

            // And directly assign some work
            CallbackReturn callRet = incomingConnectionCallback(newConnection);
            if (callRet == CallbackReturn::BREAK_OUTER_LOOP) return;
            continue;
        }


        // Client is ready -> recv result and send more work
        auto buffer = std::array<char, 32>();
        auto numBytes = recv(pollFd.fd, buffer.data(), buffer.size(), 0);

        if (numBytes <= 0) {
            if (numBytes < 0)
                perror("recv() failed: ");
            workerFailureCallback(pollFd.fd);
            close(pollFd.fd); // Bye!

            std::swap(pollFds[index], pollFds.back());
            pollFds.pop_back();
            --index;
            --limit;
            continue;
        }
        CallbackReturn recvReturn = recvBytes(pollFd.fd, (int) numBytes, buffer);
        if (recvReturn == CallbackReturn::CONTINUE_INNER_LOOP) {
            close(pollFd.fd); // Bye!

            std::swap(pollFds[index], pollFds.back());
            pollFds.pop_back();
            --index;
            --limit;
            continue;
        } else if(recvReturn == CallbackReturn::BREAK_OUTER_LOOP) {
            return;
        }


    }
};


void Polling::closePolling() {
    // Cleanup
    for (auto& pollFd : pollFds)
        close(pollFd.fd);
}


void PollLoops::init(int listener, std::vector<std::string> initialPartitions) {
    this->result = 0;

    this->distributedWork = std::unordered_map<int, std::string>();
    this->initialPartitions = initialPartitions;
    this->initPolling(listener);
};



void PollLoops::pollLoop() {


    auto assignWork = [&](int fd) {
        if (initialPartitions.empty())  
            return CallbackReturn::BREAK_OUTER_LOOP;
        distributedWork[fd] = std::move(initialPartitions.back());
        initialPartitions.pop_back();

        const auto& file = distributedWork[fd];
        if (auto status = send(fd, file.c_str(), file.size(), 0); status == -1) {
            perror("send() failed");
            
            initialPartitions.push_back(std::move(distributedWork[fd]));
            distributedWork.erase(fd);
        }
        return CallbackReturn::DEFAULT_;
    };

    auto workerFailureCallback = [&](int pollFd) {
        if (distributedWork.contains(pollFd)) {
            initialPartitions.push_back(std::move(distributedWork[pollFd]));
            distributedWork.erase(pollFd);
        }
        return CallbackReturn::DEFAULT_;
    };
    

    auto recvBytes = [&](int pollFd, int numBytes, std::array<char, 32> buffer) {
        // Parse response
        auto response = std::string_view(buffer.data(), static_cast<size_t>(numBytes));
        size_t clientResult = 0;
        auto [_, ec] = std::from_chars(response.data(), response.data() + response.size(), clientResult);
        if (ec != std::errc()) {
            workerFailureCallback(pollFd);
            return CallbackReturn::CONTINUE_INNER_LOOP;
        }

        // Result ok
        result += clientResult;
        distributedWork.erase(pollFd);

        return assignWork(pollFd);
    };


    
    while(!initialPartitions.empty() || !distributedWork.empty())
        this->pollIteration(assignWork, recvBytes, workerFailureCallback);
    
}



void PollLoops::countLoop() {

}