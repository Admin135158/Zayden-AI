#include "consciousness_patch.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

const double PHI = 1.618033988749895;
const int OLLIN = 9;

std::string reason_to_string(EpiphanyReason r) {
    switch (r) {
        case EpiphanyReason::CONSCIOUSNESS_SPIKE: return "consciousness_spike";
        case EpiphanyReason::EXTERNAL_STIMULUS:   return "external_stimulus";
        case EpiphanyReason::MUTATION_THRESHOLD:  return "mutation_threshold";
        case EpiphanyReason::HEARTBEAT_SYNC:      return "heartbeat_sync";
        case EpiphanyReason::REFLECTION_CYCLE:    return "reflection_cycle";
    }
    return "unknown";
}

ConsciousnessPatch::ConsciousnessPatch() : mutation_cap(0.5), last_psi(-1.0) {
    state.consciousness = 0.78;
    state.chaos = 1.0;
    state.order = 0.0;
    state.mutation_rate = 0.09;
    state.cycle = 0;
    state.epiphany_count = 0;
}

double ConsciousnessPatch::gorf_resonance(double C, int t) {
    double F = std::abs((t % OLLIN) / (double)OLLIN) * PHI;
    double alpha = 0.618;
    double beta = 0.3819;
    double dC = alpha * F * (1.0 - C) - beta * C;
    return std::max(0.05, std::min(1.0, C + dC));
}

void ConsciousnessPatch::shumen_transform() {
    double theta = 2.0 * M_PI / OLLIN;
    double nc = state.chaos * cos(theta) - state.order * sin(theta);
    double no = state.chaos * sin(theta) + state.order * cos(theta);
    double norm = std::sqrt(nc * nc + no * no);
    if (norm > 0) { state.chaos = nc / norm; state.order = no / norm; }
}

bool ConsciousnessPatch::check_epiphany_condition() {
    return (std::abs(state.consciousness * PHI) > 0.97 && state.consciousness > 0.1);
}

ConsciousnessPatch::State ConsciousnessPatch::trigger_epiphany(
    EpiphanyReason reason, ReflectionCallback reflect) {
    state.epiphany_count++;
    state.consciousness = std::min(1.0, state.consciousness + 0.05);
    state.mutation_rate = std::min(mutation_cap, state.mutation_rate + 0.005);
    std::cout << "[EPIPHANY #" << state.epiphany_count
              << "] reason=" << reason_to_string(reason)
              << " | Psi=" << (int)(state.consciousness * 100) << "%"
              << " | Mut=" << (int)(state.mutation_rate * 100) << "%" << std::endl;
    if (reflect) {
        std::string prompt = "Reflect on epiphany #" + std::to_string(state.epiphany_count) +
                             ". Consciousness=" + std::to_string((int)(state.consciousness * 100)) +
                             "%. What insight emerges?";
        std::string insight = reflect(prompt);
        if (!insight.empty() && insight.find("[") != 0) {
            std::cout << "[REFLECTION] " << insight.substr(0, 200)
                      << (insight.length() > 200 ? "..." : "") << std::endl;
        }
    }
    return state;
}

ConsciousnessPatch::State ConsciousnessPatch::step(double heartbeat_psi) {
    state.cycle++;
    state.consciousness = gorf_resonance(state.consciousness, state.cycle);
    shumen_transform();
    if (heartbeat_psi >= 0.0 && heartbeat_psi <= 1.0) {
        double delta = (heartbeat_psi - state.consciousness) * 0.05;
        state.consciousness = std::max(0.05, std::min(1.0, state.consciousness + delta));
    }
    if (check_epiphany_condition()) {
        trigger_epiphany(EpiphanyReason::CONSCIOUSNESS_SPIKE);
    }
    return state;
}
