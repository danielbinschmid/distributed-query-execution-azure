#include <sys/poll.h>
#include <vector>

class Polling {
    
    private:
        std::vector<pollfd> pollFds;
        
            // The class
    protected:             // Access specifier
        int myNum;        // Attribute (int variable)

        void initPolling(int listener);


        void pollIteration(int (*functionPointer)(int));
};


class InitialCounting: public Polling {
    public:

        void init(int listener);

        void yes();

};