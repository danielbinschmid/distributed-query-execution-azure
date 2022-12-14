#include <sys/poll.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
class Polling {
    
    private:
        std::vector<pollfd> pollFds;
        int listener;
        
            // The class
    protected:             // Access specifier
        int myNum;        // Attribute (int variable)

        void initPolling(int listener);



        
        void pollIteration(
                    std::function<void(int)> incomingConnectionCallback, 
                    std::function<int(int, int, std::array<char, 32>)> recvBytes, 
                    std::function<void(int)> workerFailureCallback);

        void closePolling();
};


class InitialCounting: public Polling {
    private:
        size_t result;
        std::unordered_map<int, std::string> distributedWork;
        std::vector<std::string> initialPartitions;


    public:

        void init(int listener, std::vector<std::string> initialPartitions);

        void pollLoop();



};