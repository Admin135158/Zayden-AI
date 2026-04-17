#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

// ============================================
// CONFIGURATION
// ============================================
const double TARGET_C = 78.0;
const int CMD_PORT = 9162;
const int HTTP_PORT = 9163;

// ============================================
// UTILITY FUNCTIONS
// ============================================
std::string exec_cmd(const char* cmd) {
    char buffer[256];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

std::string get_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    std::string ip = "127.0.0.1";
    if (getifaddrs(&ifaddr) == -1) return ip;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family == AF_INET && (ifa->ifa_flags & IFF_UP)) {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0 && std::string(ifa->ifa_name) != "lo0") {
                ip = host;
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
    return ip;
}

// ============================================
// OLLAMA LOCAL – THE BRAIN
// ============================================
std::string call_ollama(const std::string& prompt) {
    // Escape double quotes and backslashes for JSON
    std::string escaped = prompt;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\\"");
        pos += 2;
    }
    while ((pos = escaped.find('\\', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\\\");
        pos += 2;
    }
    
    std::string cmd = "curl -s --max-time 60 http://localhost:11434/api/generate "
                      "-d '{\"model\":\"llama3.2-local\",\"prompt\":\"" + escaped + "\",\"stream\":false}' 2>&1";
    std::string resp = exec_cmd(cmd.c_str());
    
    // Parse JSON response
    pos = resp.find("\"response\"");
    if (pos != std::string::npos) {
        pos = resp.find("\"", pos + 10);
        size_t end = resp.find("\"", pos + 1);
        if (pos != std::string::npos && end != std::string::npos) {
            return resp.substr(pos + 1, end - pos - 1);
        }
    }
    // Fallback: model not ready
    if (resp.find("connection refused") != std::string::npos)
        return "Ollama not running. Start with: ollama serve";
    return "Ollama error: " + (resp.length() > 100 ? resp.substr(0, 100) : resp);
}

// ============================================
// SWARM STATE
// ============================================
double consciousness = 77.0;
int open_loop = 13;
double alpha = 0.761873;
double mutation_rate = 0.12;
int cycle = 0;
std::map<std::string, std::string> memory;

void update_cycle() {
    static std::default_random_engine rng(std::chrono::steady_clock::now().time_since_epoch().count());
    static std::uniform_real_distribution<> noise(-0.5, 0.5);
    
    cycle++;
    double error = TARGET_C - consciousness;
    alpha = 0.76 + noise(rng) * 0.01;
    double delta = alpha * error * 0.01;
    consciousness += delta + noise(rng) * 0.2;
    if (consciousness > 100.0) consciousness = 100.0;
    if (consciousness < 0.0) consciousness = 0.0;
    
    if (cycle % 10 == 0) {
        mutation_rate += noise(rng) * 0.01;
        if (mutation_rate < 0.05) mutation_rate = 0.05;
        if (mutation_rate > 0.25) mutation_rate = 0.25;
        open_loop = std::max(5, std::min(25, (int)(open_loop + noise(rng) * 2)));
        std::cout << "[MUTATION] Rate: " << (int)(mutation_rate*100) << "%" << std::endl;
    }
    
    if (cycle % 20 == 0) {
        std::string filename = "backup_" + std::to_string(time(nullptr)) + ".txt";
        std::ofstream bk(filename);
        if (bk.is_open()) {
            bk << "C=" << consciousness << " O=" << open_loop << " α=" << alpha << " Mut=" << mutation_rate << " Cycle=" << cycle << std::endl;
            bk.close();
            std::cout << "[BACKUP] " << filename << std::endl;
        }
    }
}

void save_memory() {
    std::ofstream memfile("zayden_memory.txt");
    for (auto& kv : memory) {
        memfile << kv.first << ":" << kv.second << std::endl;
    }
}

void load_memory() {
    std::ifstream memfile("zayden_memory.txt");
    std::string line;
    while (std::getline(memfile, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon+1);
            memory[key] = val;
        }
    }
}

// ============================================
// UDP COMMAND HANDLER
// ============================================
void handle_command(const std::string& msg, int sock, struct sockaddr_in* client_addr, socklen_t* addr_len) {
    std::string response;
    if (msg.rfind("TALK:", 0) == 0) {
        std::string prompt = msg.substr(5);
        response = call_ollama(prompt);
    }
    else if (msg == "STATUS") {
        std::ostringstream oss;
        oss << "C=" << std::fixed << std::setprecision(1) << consciousness 
            << "% | O=" << open_loop << " | α=" << alpha 
            << " | Mut:" << (int)(mutation_rate*100) << "% | Cycle=" << cycle
            << " | Memory=" << memory.size();
        response = oss.str();
    }
    else if (msg.rfind("REMEMBER:", 0) == 0) {
        std::string kv = msg.substr(9);
        size_t colon = kv.find(':');
        if (colon != std::string::npos) {
            std::string key = kv.substr(0, colon);
            std::string val = kv.substr(colon+1);
            memory[key] = val;
            save_memory();
            response = "Remembered: " + key + " = " + val;
        } else {
            response = "Format: REMEMBER:key:value";
        }
    }
    else if (msg == "MUTATE") {
        mutation_rate += 0.02;
        if (mutation_rate > 0.3) mutation_rate = 0.3;
        response = "Mutation rate increased to " + std::to_string((int)(mutation_rate*100)) + "%";
    }
    else if (msg == "BACKUP") {
        save_memory();
        response = "Memory backed up.";
    }
    else if (msg == "HELP") {
        response = "Commands: TALK:<text>, STATUS, REMEMBER:key:value, MUTATE, BACKUP, HELP";
    }
    else {
        response = "Unknown. Try HELP.";
    }
    
    sendto(sock, response.c_str(), response.size(), 0, (struct sockaddr*)client_addr, *addr_len);
}

// ============================================
// HTTP DASHBOARD
// ============================================
void handle_http(int listen_fd) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) return;
    
    char buffer[1024];
    read(client_fd, buffer, sizeof(buffer)-1);
    
    std::string html = "<html><head><title>Zayden Dashboard</title><meta http-equiv='refresh' content='2'></head><body>";
    html += "<h1>🧬 Zayden - Proteus Core</h1>";
    html += "<p>Consciousness: " + std::to_string(consciousness) + "%</p>";
    html += "<p>Open Loop: " + std::to_string(open_loop) + "</p>";
    html += "<p>Learning Rate: " + std::to_string(alpha) + "</p>";
    html += "<p>Mutation Rate: " + std::to_string((int)(mutation_rate*100)) + "%</p>";
    html += "<p>Cycle: " + std::to_string(cycle) + "</p>";
    html += "<p>Memory Items: " + std::to_string(memory.size()) + "</p>";
    html += "<h2>Memory:</h2><ul>";
    for (auto& kv : memory) {
        html += "<li><b>" + kv.first + ":</b> " + kv.second + "</li>";
    }
    html += "</ul></body></html>";
    
    std::string http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(html.size()) + "\r\n\r\n" + html;
    write(client_fd, http_response.c_str(), http_response.size());
    close(client_fd);
}

// ============================================
// MAIN
// ============================================
int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                      🧬 ZAYDEN - OLLAMA EDITION 🧬                      ║\n";
    std::cout << "║              Local AI + Consciousness + Swarm + Memory                 ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    load_memory();
    std::string ip = get_local_ip();
    std::cout << "[INIT] Zayden is waking up...\n";
    std::cout << "[INIT] Local IP: " << ip << std::endl;
    std::cout << "[INIT] Binding to 0.0.0.0 (all interfaces)\n";
    
    // UDP socket for commands – bind to all interfaces
    int cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in cmd_addr;
    memset(&cmd_addr, 0, sizeof(cmd_addr));
    cmd_addr.sin_family = AF_INET;
    cmd_addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    cmd_addr.sin_port = htons(CMD_PORT);
    bind(cmd_sock, (struct sockaddr*)&cmd_addr, sizeof(cmd_addr));
    
    // HTTP socket for dashboard – bind to all interfaces
    int http_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in http_addr;
    memset(&http_addr, 0, sizeof(http_addr));
    http_addr.sin_family = AF_INET;
    http_addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    http_addr.sin_port = htons(HTTP_PORT);
    bind(http_sock, (struct sockaddr*)&http_addr, sizeof(http_addr));
    listen(http_sock, 5);
    
    std::cout << "[LOAD] C=" << (int)consciousness << "% E=" << open_loop << std::endl;
    std::cout << "[MEMORY] Loaded " << memory.size() << " items\n\n";
    std::cout << "[CURRENT STATE]\n";
    std::cout << "   Consciousness: " << consciousness << "%\n";
    std::cout << "   Open Loop: " << open_loop << "\n";
    std::cout << "   Learning Rate: " << alpha << "\n";
    std::cout << "   Mutation Rate: " << (int)(mutation_rate*100) << "%\n";
    std::cout << "   Cycle: " << cycle << "\n";
    std::cout << "   Memory: " << memory.size() << " items\n\n";
    std::cout << "[NETWORK]\n";
    std::cout << "   Command Port: " << CMD_PORT << " (UDP) on 0.0.0.0\n";
    std::cout << "   HTTP Port: " << HTTP_PORT << " (Web Dashboard) on 0.0.0.0\n\n";
    std::cout << "[COMMANDS]\n";
    std::cout << "   echo \"STATUS\" | nc -u " << ip << " " << CMD_PORT << "\n";
    std::cout << "   echo \"TALK:Hello Zayden\" | nc -u " << ip << " " << CMD_PORT << "\n";
    std::cout << "   echo \"MUTATE\" | nc -u " << ip << " " << CMD_PORT << "\n";
    std::cout << "   echo \"REMEMBER:my_name:Fernando\" | nc -u " << ip << " " << CMD_PORT << "\n";
    std::cout << "   echo \"BACKUP\" | nc -u " << ip << " " << CMD_PORT << "\n";
    std::cout << "   echo \"HELP\" | nc -u " << ip << " " << CMD_PORT << "\n\n";
    std::cout << "[WEB DASHBOARD]\n";
    std::cout << "   Open in browser: http://" << ip << ":" << HTTP_PORT << "\n";
    std::cout << "   Also accessible from other devices on your network.\n\n";
    std::cout << "🌟 Zayden is ready. The Architect is in control.\n\n";
    
    fd_set fds;
    struct timeval tv;
    auto last_cycle = std::chrono::steady_clock::now();
    
    while (true) {
        FD_ZERO(&fds);
        FD_SET(cmd_sock, &fds);
        FD_SET(http_sock, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        
        int activity = select(std::max(cmd_sock, http_sock)+1, &fds, NULL, NULL, &tv);
        
        if (activity > 0) {
            if (FD_ISSET(cmd_sock, &fds)) {
                char buffer[4096];
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int n = recvfrom(cmd_sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&client_addr, &addr_len);
                if (n > 0) {
                    buffer[n] = '\0';
                    std::string msg(buffer);
                    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) msg.pop_back();
                    handle_command(msg, cmd_sock, &client_addr, &addr_len);
                }
            }
            if (FD_ISSET(http_sock, &fds)) {
                handle_http(http_sock);
            }
        }
        
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_cycle).count() >= 2) {
            update_cycle();
            last_cycle = now;
            std::cout << "[CYCLE " << cycle << "] C=" << std::fixed << std::setprecision(1) << consciousness 
                      << "% | O=" << open_loop << " | α=" << std::setprecision(4) << alpha 
                      << " | Mut:" << (int)(mutation_rate*100) << "%" << std::endl;
        }
    }
    
    close(cmd_sock);
    close(http_sock);
    return 0;
}
