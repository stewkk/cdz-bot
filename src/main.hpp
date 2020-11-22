
#include <curl/curl.h>
#include <thread>
#include "nlohmann/json.hpp"
#include "parser.hpp"
#include "config.hpp"

class Server {
private:
    CURL* curl;
    std::string key;
    std::string server;
    std::string ts;
public:
    Server();
    ~Server();
    std::string getUpdates();
    void getLongPollServer();
    void parseLongPollServer(const std::string& server_answer);
    void update_mesh_session();
};
