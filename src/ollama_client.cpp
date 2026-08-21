#include "ollama_client.hpp"
#include <array>
#include <chrono>

OllamaClient::OllamaClient(const std::string& h, const std::string& m, int t)
    : host(h), model(m), timeout_sec(t), last_error("") {}

OllamaResponse OllamaClient::generate(const std::string& prompt) {
    OllamaResponse res;
    res.success = false;
    std::string safe = prompt;
    size_t pos = 0;
    while ((pos = safe.find('"', pos)) != std::string::npos) {
        safe.replace(pos, 1, "\\\"");
        pos += 2;
    }
    std::string cmd = "curl -s --max-time " + std::to_string(timeout_sec) +
                      " " + host + "/api/generate -d '{\"model\":\"" + model +
                      "\",\"prompt\":\"" + safe +
                      "\",\"stream\":false}' 2>/dev/null | "
                      "python3 -c \"import sys,json; d=json.load(sys.stdin); "
                      "print(d.get('response',''))\" 2>/dev/null";
    auto start = std::chrono::steady_clock::now();
    std::array<char, 4096> buffer;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        res.error = "Failed to open pipe to curl";
        last_error = res.error;
        return res;
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    int status = pclose(pipe);
    auto end = std::chrono::steady_clock::now();
    res.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if (status != 0 || result.empty()) {
        res.error = "Ollama unreachable or empty response";
        last_error = res.error;
        return res;
    }
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
        result.pop_back();
    res.text = result;
    res.success = true;
    return res;
}

bool OllamaClient::pull_model() {
    std::string cmd = "curl -s " + host + "/api/pull -d '{\"name\":\"" + model +
                      "\"}' 2>/dev/null > /dev/null";
    return system(cmd.c_str()) == 0;
}

bool OllamaClient::is_available() {
    std::string cmd = "curl -s --max-time 5 " + host + "/api/tags 2>/dev/null | grep -q '" + model + "'";
    return system(cmd.c_str()) == 0;
}
