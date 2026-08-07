#pragma once
#include <string>
#include <functional>

enum class EpiphanyReason {
    CONSCIOUSNESS_SPIKE, EXTERNAL_STIMULUS, MUTATION_THRESHOLD,
    HEARTBEAT_SYNC, REFLECTION_CYCLE
};

std::string reason_to_string(EpiphanyReason r);

class ConsciousnessPatch {
public:
    struct State {
        double consciousness;
        double chaos;
        double order;
        double mutation_rate;
        int cycle;
        int epiphany_count;
    };
    using ReflectionCallback = std::function<std::string(const std::string&)>;
    ConsciousnessPatch();
    State trigger_epiphany(EpiphanyReason reason, ReflectionCallback reflect = nullptr);
    State step(double heartbeat_psi = -1.0);
    const State& get_state() const { return state; }
    void set_mutation_cap(double cap) { mutation_cap = cap; }
private:
    State state;
    double mutation_cap;
    double last_psi;
    void shumen_transform();
    double gorf_resonance(double C, int t);
    bool check_epiphany_condition();
};
