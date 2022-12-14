#include <sys/poll.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>

enum CallbackReturn { BREAK_OUTER_LOOP, CONTINUE_INNER_LOOP, DEFAULT_ };


class Polling {
    
    private:
        std::vector<pollfd> pollFds;
        int listener;
        
            // The class
    protected:             // Access specifier
        int myNum;        // Attribute (int variable)

        void initPolling(int listener);

        /**
         * 
         * @param incomingConnectionCallback - called when a new connection is established. Takes pollFd as argument.
         * @param recvBytes - called when bytes with numBytes != 0 are received. Takes (pollFd, numBytes, bytesReceivedBuffer) as arguments. 
         * Returns true if bytes received are valid and false and bytes received are invalid.
         * @param workerFailureCallback - called when a worker failed. 
        */
        void pollIteration(
                    std::function<CallbackReturn(int)> incomingConnectionCallback, 
                    std::function<CallbackReturn(int, int, std::array<char, 32>)> recvBytes, 
                    std::function<CallbackReturn(int)> workerFailureCallback);

    public:
        void closePolling();
};


class PollLoops: public Polling {
    private:
        
        std::unordered_map<int, std::string> distributedWork;
        std::vector<std::string> initialPartitions;


    public:
        size_t result;
        void init(int listener, std::vector<std::string> initialPartitions);

        void pollLoop();


        void countLoop();
};