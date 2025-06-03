#pragma once
#include <cstddef>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include "Wire.h"
#include "WireBus.h"

class Multiplexer {
private:
    std::vector<std::vector<Wire*>> inputBuses;
    std::vector<Wire*> select;
    std::vector<Wire*> outBus;
    std::string name;
    size_t size;
public:
    static std::vector<Multiplexer*> multiplexers; 

    Multiplexer(size_t size, std::string name, std::vector<std::vector<Wire*>> inputBuses, std::vector<Wire*> select, std::vector<Wire*> outBus)
        : size(size), name(name), inputBuses(inputBuses), select(select), outBus(outBus) {
        std::cout << "Creating Multiplexer: " << name << " with size: " << size << std::endl;
        if (inputBuses.empty() || select.empty() || outBus.empty()) {
            throw std::invalid_argument("Input buses, select lines, and output bus cannot be empty.");
        }
        if (select.size() > 1 && inputBuses.size() != (1 << select.size())) {
            throw std::invalid_argument("Number of input buses must match 2^number_of_select_lines.");
        }
        if (outBus.size() != inputBuses[0].size()) {
            throw std::invalid_argument("Output bus size must match input bus size.");
        }
        multiplexers.push_back(this);
    }
    
    void tick();
};

class Demultiplexer {
private:
    std::vector<Wire*> input;
    std::vector<Wire*> select;
    std::vector<std::vector<Wire*>> outputBuses;
    std::string name;
    size_t size;
public:
    static std::vector<Demultiplexer*> demultiplexers;

    Demultiplexer(size_t size, std::string name, std::vector<Wire*> input, std::vector<Wire*> select, std::vector<std::vector<Wire*>> outputBuses)
        : size(size), name(name), input(input), select(select), outputBuses(outputBuses) {
        if (select.empty() || outputBuses.empty()) {
            throw std::invalid_argument("Select lines and output buses cannot be empty.");
        }
        if (outputBuses.size() != (1 << select.size())) {
            throw std::invalid_argument("Number of output buses must match 2^number_of_select_lines.");
        }
        if (input.size() != outputBuses[0].size()) {
            throw std::invalid_argument("Input size must match output bus size.");
        }
        demultiplexers.push_back(this);
    }
    
    void tick();

};