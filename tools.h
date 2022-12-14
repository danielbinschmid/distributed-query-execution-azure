#include <vector>
#include <string>

namespace tools {
    namespace coordinator {
        int getListenerSocket(char* port);

        void getInitialPartitionsAzure(char* filename, std::vector<std::string> &todoOutput);


        void getInitialPartitionsLocalFiles(char* pathToCsv, std::vector<std::string> &todoOutput);

        void getInitialPartitionsHttp(char* pathToCsv,  std::vector<std::string> &todoOutput);
    } 
}