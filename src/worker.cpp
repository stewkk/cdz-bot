
#include "worker.hpp"
#include <iostream>
#include "VkApi.hpp"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    std::cout << (*((std::string*)userp)) << std::endl;
    return size * nmemb;
}

void work(std::string text, long long from_id) {
    std::string variant_id = get_variant_id(text);
    if (variant_id.size()) {
        MeshApi mesh_api;
        mesh_api.getTest(variant_id);
        mesh_api.getAnswers(variant_id);
        // std::string answers_message = mesh_api.getAnswersMessage();
        mesh_api.sendAnswers(std::to_string(from_id));
    } else {
        VkApi vk_api;
        vk_api.sendHelp(std::to_string(from_id));
    }
}

void MeshApi::sendAnswers(const std::string& from_id) {
    VkApi vk_api;
    for (int i = 0; i < tasks_answers.size(); ++i) {
        tasks_answers[i] = std::to_string(i + 1) + " " + tasks_answers[i];
        vk_api.sendAnswers(tasks_answers[i], from_id);
    }
}

std::string MeshApi::getAnswersMessage() {
    std::string res;
    for (int i = 0; i < tasks_answers.size(); ++i) {
        res += std::to_string(i + 1) + ' ';
        if (tasks_answers[i].size()) {
            res += tasks_answers[i] + '\n';
        }
    }
    return res;
}

std::string get_variant_id(std::string& text) {
    auto pos = text.find("variant/");
    if (pos == std::string::npos) {
        return "";
    } else {
        return text.substr(pos + 8, 8);
    }
}

MeshApi::MeshApi() {
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    struct curl_slist* list = NULL;
    list = curl_slist_append(list, "Cookie: ");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
}

MeshApi::~MeshApi() {
    curl_easy_cleanup(curl);
}

void MeshApi::getTest(const std::string& variant_id) {
    std::string url = "https://uchebnik.mos.ru/exam/rest/secure/testplayer/variant/" + variant_id;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    // std::cout << readBuffer << std::endl;
    nlohmann::json j = nlohmann::json::parse(readBuffer);
    if (j.is_object()) {
        j = j["tasks"];
        int num = 1;
        for (auto& task : j) {
            long long task_id = task["id"].get<long long>();
            tasks_ids.push_back(task_id);
            tasks_bools.push_back(false);
        }
    }
}

void MeshApi::getAnswers(const std::string& variant_id) {
    std::string url = "https://uchebnik.mos.ru/exam/rest/secure/api/answer/variant/" + variant_id;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code == 200) {
        nlohmann::json j = nlohmann::json::parse(readBuffer);
        if (j.is_array()) {
            tasks_answers.resize(tasks_ids.size());
            for (auto& answer : j) {
                if (answer["is_right"].get<bool>()) {
                    long long task_id = answer["task_id"].get<long long>();
                    size_t index = find(tasks_ids.begin(), tasks_ids.end(), task_id) - tasks_ids.begin();
                    if (index < tasks_ids.size() && !tasks_bools[index]) {
                        nlohmann::json task = getTask(variant_id, std::to_string(index + 1));
                        std::string given_answer = "NOT SUPPORTED ANSWER TYPE: " + answer["given_answer"]["@answer_type"].get<std::string>();
                        if (answer["given_answer"]["@answer_type"] == "answer/single") {
                            for (auto& k : task["answer"]["options"]) {
                                if (k["id"] == answer["given_answer"]["id"]) {
                                    given_answer = k["text"];
                                    break;
                                }
                            }
                        } else if (answer["given_answer"]["@answer_type"] == "answer/multiple") {
                            given_answer = "";
                            for (auto& k : task["answer"]["options"]) {
                                std::vector<std::string> ids = answer["given_answer"]["ids"].get<std::vector<std::string>>();
                                if (find(ids.begin(), ids.end(), k["id"].get<std::string>()) != ids.end()) {
                                    given_answer += (k["text"].get<std::string>() + "; ");
                                }
                            }
                        } else if (answer["given_answer"]["@answer_type"] == "answer/number") {
                            given_answer = std::to_string(answer["given_answer"]["number"].get<long long>());
                        } else if (answer["given_answer"]["@answer_type"] == "answer/string") {
                            given_answer = answer["given_answer"]["string"].get<std::string>();
                        } else if (answer["given_answer"]["@answer_type"] == "answer/groups") {
                            given_answer = "";
                            for (auto& ans_group : answer["given_answer"]["groups"]) {
                                for (auto& k : task["answer"]["options"]) {
                                    if (k["id"] == ans_group["group_id"]) {
                                        given_answer += "GROUP \"" + k["text"].get<std::string>() + "\":\n";
                                    }
                                }
                                for (auto& ans_opt : ans_group["options_ids"]) {
                                    for (auto& k : task["answer"]["options"]) {
                                        if (k["id"] == ans_opt) {
                                            given_answer += k["text"].get<std::string>();
                                        }
                                    }
                                    given_answer += '\n';
                                }
                            }
                        } else if (answer["given_answer"]["@answer_type"] == "answer/match") {
                            given_answer = "";
                            for (auto& k : task["answer"]["options"]) {
                                std::vector<std::string> matches = answer["given_answer"]["match"].get<std::vector<std::string>>();
                                if (find(matches.begin(), matches.end(), k["id"].get<std::string>()) != matches.end()) {
                                    given_answer += k["text"].get<std::string>() + " => ";
                                    for (auto& ans_match : task["answer"]["options"]) {
                                        for (auto& m : answer["given_answer"]["match"][k["id"].get<std::string>()]) {
                                            if (ans_match["id"] == m) {
                                                given_answer += ans_match["text"].get<std::string>() + "; ";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        std::string question = "ВОПРОС:\n";
                        for (auto& el : task["question_elements"]) {
                            question += el["text"].get<std::string>() + '\n';
                        }
                        tasks_answers[index] = question + "\nОТВЕТ:\n" + given_answer + "\n\n";
                        tasks_bools[index] = true;
                    }
                }
            }
        }
    }
}

std::string MeshApi::parseAnswer(nlohmann::json& answer) {
    return "";
}

nlohmann::json MeshApi::getTask(const std::string& variant_id, const std::string& task_index) {
    std::string url = "https://uchebnik.mos.ru/exam/rest/secure/api/test_task/" + variant_id + "/" + task_index;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    return nlohmann::json::parse(readBuffer);
}
