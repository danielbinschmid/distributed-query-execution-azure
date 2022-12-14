#include "Polling.h"
#include <cstddef>

void Polling::initPolling(int listener) {

    this->pollFds.push_back(pollfd{
        .fd = listener,
        .events = POLLIN,
        .revents = {},
    });

};


void Polling::pollIteration(int (*functionPointer)(int)) {
    poll(pollFds.data(), pollFds.size(), -1);

    for (size_t index = 0, limit = pollFds.size(); index != limit; ++index) {
        
    }
}


void InitialCounting::init(int listener) {
    this->init(listener);
};