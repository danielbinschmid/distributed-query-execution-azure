#include <sys/poll.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
#include "tools.h"
enum CallbackReturn { BREAK_OUTER_LOOP, CONTINUE_INNER_LOOP, DEFAULT_ };


class Polling {
    
 
    protected:            
        int listener;
        std::vector<pollfd> pollFds;
        void initPolling(int listener);
        void initPolling(Polling polling);

        /**
         * 
         * @param incomingConnectionCallback - called when a new connection is established. Takes pollFd as argument.
         * @param recvBytes - called when bytes with numBytes != 0 are received. Takes (pollFd, numBytes, bytesReceivedBuffer) as arguments. 
         * @param workerFailureCallback - called when a worker failed. 
         * 
         * Skips to next pollFd when a callback yields CONTINUE_INNER_LOOP.
         * Terminates immediately if a callback yields BREAK_OUTER_LOOP.
         * Follows default path if a callback yields DEFAULT_.
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

};



class CountLoop: public Polling {
    private:
        std::vector<CountPartitionTask> initialPartitions;

        std::unordered_map<int, CountPartitionTask> distributedWork;


    public:
        int result;
        void init(int listener, std::vector<std::string> initialPartitions);

        void countLoop();
};



class MergeSortLoop: public Polling {
    private: 
        std::vector<MergeSortTask> initialTasks;
        std::unordered_map<int, MergeSortTask> distributedTasks;
        HashRanging hashranging;

    public:

        int result;
        MergeSortLoop(Polling polling);

        ~MergeSortLoop();

        void run();




};