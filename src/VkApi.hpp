
#include <curl/curl.h>
#include <string>

class VkApi {
private:
    CURL* curl;
    void messagesSend(std::string& message, const std::string& peer_id);
public:
    VkApi();
    ~VkApi();
    void sendAnswers(std::string& answers, const std::string& from_id);
    void sendHelp(const std::string& from_id);
};
