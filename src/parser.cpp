
#include "nlohmann/json.hpp"
#include "worker.hpp"
#include <thread>

void parse_ans(std::string vk_ans) {
    nlohmann::json j = nlohmann::json::parse(vk_ans);
    j = j["updates"];
    for (auto& el : j) {
        if (el["type"].get<std::string>() == "message_new") {
            nlohmann::json message = el["object"]["message"];
            std::string text = message["text"].get<std::string>();
            long long from_id = message["from_id"].get<long long>();
            std::thread worker(work, text, from_id);
            worker.detach();
        }
    }
}
