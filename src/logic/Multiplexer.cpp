#include "../includes/Multiplexer.h"
#include "../includes/Wire.h"
#include <iostream>

std::vector<Multiplexer*> Multiplexer::multiplexers;
std::vector<Demultiplexer*> Demultiplexer::demultiplexers;

void Multiplexer::tick() {
    if (select.empty() || inputBuses.empty()) {
        std::cerr << "Multiplexer called with empty select or input buses." << std::endl;
        return;
    }
    int index = 0;
    for (size_t i = 0; i < select.size(); ++i) {
        if (select[i]->isLogicHigh()) {
            index |= (1 << i);
        }
    }
    if (index < inputBuses.size()) {
        for (size_t i = 0; i < outBus.size(); ++i) {
            outBus[i]->setState(inputBuses[index][i]->getState());
        }
    }
}

void Demultiplexer::tick() {
    if (select.empty() || outputBuses.empty()) {
        std::cerr << "Demultiplexer called with empty select or output buses." << std::endl;
        return;
    }
    int index = 0;
    for (size_t i = 0; i < select.size(); ++i) {
        if (select[i]->isLogicHigh()) {
            index |= (1 << i);
        }
    }
    if (index < outputBuses.size()) {
        for (size_t i = 0; i < outputBuses[index].size(); ++i) {
            outputBuses[index][i]->setState(input[i]->getState());
        }
    }
}