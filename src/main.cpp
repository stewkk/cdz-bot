
#include "main.hpp"
#include <iostream>

Server::Server() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        exit(1);
    }
    getLongPollServer();
}

Server::~Server() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Server::getUpdates() {
    std::string url = server + "?act=a_check&key=" + key + "&ts=" + ts + "&wait=25";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    if (curl) {
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }
    nlohmann::json j = nlohmann::json::parse(readBuffer);
    j = j;
    if (!j["ts"].is_null()) {
        ts = j["ts"].get<std::string>();
    } else {
        getLongPollServer();
    }
    if (j["failed"].is_null()) {
        return readBuffer;
    }
    return "";
}

void Server::getLongPollServer() {
    std::string url = VK_DOMAIN + "groups.getLongPollServer?group_id=" + GROUP_ID + "&v=" + V
            + "&access_token=" + ACCESS_TOKEN;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    if (curl) {
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }
    parseLongPollServer(readBuffer);
}

void Server::parseLongPollServer(const std::string& server_answer) {
    nlohmann::json j = nlohmann::json::parse(server_answer);
    if (!j.is_object()) {
        exit(1);
    }
    j = j["response"];
    if (!j.is_null()) {
        key = j["key"].get<std::string>();
        server = j["server"].get<std::string>();
        ts = j["ts"].get<std::string>();
    } else {
        exit(1);
    }
}

void Server::update_mesh_session() {
    std::string url = "https://uchebnik.mos.ru/catalogue";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    struct curl_slist* list = NULL;
    list = curl_slist_append(list, "Cookie: ");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_perform(curl);
}

int main() {
    Server vk_api;
    while (true) {
        vk_api.update_mesh_session();
        std::string vk_ans = vk_api.getUpdates();
        if (vk_ans.size()) {
            std::thread manager(parse_ans, vk_ans);
            manager.detach();
        }
    }
    return 0;
}
