
#include <string>
#include "config.hpp"
#include <curl/curl.h>
#include <vector>
#include "nlohmann/json.hpp"

void work(std::string text, long long from_id);

std::string get_variant_id(std::string& text);

class MeshApi {
private:
    CURL* curl;
    std::vector<long long> tasks_ids;
    std::vector<bool> tasks_bools;
    std::vector<std::string> tasks_answers;
public:
    MeshApi();
    ~MeshApi();
    void getTest(const std::string& variant_id);
    void getAnswers(const std::string& variant_id);
    std::string getAnswersMessage();
    std::string parseAnswer(nlohmann::json& answer);
    nlohmann::json getTask(const std::string& variant_id, const std::string& task_index);
    void sendAnswers(const std::string& from_id);
};
