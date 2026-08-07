#pragma once
#include <string>
#include <vector>

struct ZaydenDNA {
    int cycle;
    float consciousness;
    float chaos;
    float order;
    float mutation;
    std::vector<std::string> memory_keys;
    std::string serialize() const;
    static ZaydenDNA deserialize(const std::string& dna_str);
};

class DNASnapshot {
public:
    static std::string encode(const ZaydenDNA& dna);
    static ZaydenDNA decode(const std::string& atgc);
    static bool save(const ZaydenDNA& dna, const std::string& filename);
    static ZaydenDNA load(const std::string& filename);
    static std::string filename(int cycle);
private:
    static std::string encode_bytes(const std::string& data);
    static std::string decode_bytes(const std::string& atgc);
};
