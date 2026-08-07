#pragma once
#include <string>
#include <chrono>

struct OllamaResponse {
    std::string text;
    bool success;
    std::string error;
    long latency_ms;
};

class OllamaClient {
private:
    std::string host;
    std::string model;
    int timeout_sec;
    std::string last_error;
public:
    OllamaClient(const std::string& host = "http://localhost:11434",
                 const std::string& model = "tinyllama",
                 int timeout = 30);
    OllamaResponse generate(const std::string& prompt);
    bool pull_model();
    bool is_available();
    void set_model(const std::string& m) { model = m; }
    std::string get_model() const { return model; }
    std::string get_last_error() const { return last_error; }
};

