
#include "VkApi.hpp"
#include <cstdlib>
#include "config.hpp"
#include <iostream>

VkApi::VkApi() {
    curl = curl_easy_init();
    if (!curl) {
        exit(1);
    }
}

VkApi::~VkApi() {
    curl_easy_cleanup(curl);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void encodeStringToURL(CURL* curl_handle, std::string& str) {
    char* output = curl_easy_escape(curl_handle, str.c_str(), str.size());
    str = output;
    curl_free(output);
}

void VkApi::messagesSend(std::string& message, const std::string& peer_id) {
    encodeStringToURL(curl, message);
    std::string url = VK_DOMAIN + "messages.send?v=" + V + "&access_token=" + ACCESS_TOKEN + "&random_id=0&peer_id=" +
            peer_id + "&message=" + message;
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
}

void VkApi::sendAnswers(std::string& answers, const std::string& from_id) {
    messagesSend(answers, from_id);
}

void VkApi::sendHelp(const std::string& from_id) {
    std::string help_message;
    help_message = "Чтобы получить ответы, отправьте боту ссылку на тест\nпример: https://uchebnik.mos.ru/exam/test/test_by_binding/9178117/homework/117921370/variant/19426663/num/1";
    messagesSend(help_message, from_id);
}
