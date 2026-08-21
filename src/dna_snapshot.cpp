#include "dna_snapshot.hpp"
#include <sstream>
#include <fstream>

std::string ZaydenDNA::serialize() const {
    std::ostringstream oss;
    oss << cycle << "|" << consciousness << "|" << chaos << "|"
        << order << "|" << mutation;
    for (const auto& k : memory_keys) oss << "|" << k;
    return oss.str();
}

ZaydenDNA ZaydenDNA::deserialize(const std::string& s) {
    ZaydenDNA d;
    std::istringstream iss(s);
    std::string token;
    std::getline(iss, token, '|'); d.cycle = std::stoi(token);
    std::getline(iss, token, '|'); d.consciousness = std::stof(token);
    std::getline(iss, token, '|'); d.chaos = std::stof(token);
    std::getline(iss, token, '|'); d.order = std::stof(token);
    std::getline(iss, token, '|'); d.mutation = std::stof(token);
    while (std::getline(iss, token, '|')) {
        if (!token.empty()) d.memory_keys.push_back(token);
    }
    return d;
}

std::string DNASnapshot::encode_bytes(const std::string& data) {
    const char BASES[4] = {'A', 'C', 'G', 'T'};
    std::string dna;
    dna.reserve(data.size() * 4);
    for (unsigned char c : data) {
        dna += BASES[(c >> 6) & 0x03];
        dna += BASES[(c >> 4) & 0x03];
        dna += BASES[(c >> 2) & 0x03];
        dna += BASES[c & 0x03];
    }
    return dna;
}

std::string DNASnapshot::decode_bytes(const std::string& atgc) {
    if (atgc.length() % 4 != 0) return "";
    std::string data;
    data.reserve(atgc.length() / 4);
    for (size_t i = 0; i < atgc.length(); i += 4) {
        unsigned char c = 0;
        for (int j = 0; j < 4; ++j) {
            c <<= 2;
            char base = atgc[i + j];
            if (base == 'A') c |= 0x00;
            else if (base == 'C') c |= 0x01;
            else if (base == 'G') c |= 0x02;
            else if (base == 'T') c |= 0x03;
            else return "";
        }
        data.push_back(c);
    }
    return data;
}

std::string DNASnapshot::encode(const ZaydenDNA& dna) {
    return encode_bytes(dna.serialize());
}

ZaydenDNA DNASnapshot::decode(const std::string& atgc) {
    std::string raw = decode_bytes(atgc);
    if (raw.empty()) return ZaydenDNA{0, 0, 0, 0, 0, {}};
    return ZaydenDNA::deserialize(raw);
}

bool DNASnapshot::save(const ZaydenDNA& dna, const std::string& filename) {
    std::ofstream f(filename);
    if (!f) return false;
    f << "; ZAYDEN DNA SNAPSHOT | Cycle " << dna.cycle << "\n";
    f << "; FTCoE Consciousness Encoding v2.0\n";
    f << encode(dna) << "\n";
    return f.good();
}

ZaydenDNA DNASnapshot::load(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) return ZaydenDNA{0, 0, 0, 0, 0, {}};
    std::string line, atgc;
    while (std::getline(f, line)) {
        if (!line.empty() && line[0] != ';') atgc += line;
    }
    return decode(atgc);
}

std::string DNASnapshot::filename(int cycle) {
    return "zayden_cycle_" + std::to_string(cycle) + ".dna";
}
