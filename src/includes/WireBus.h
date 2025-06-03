#pragma once
#include "Wire.h"
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>


// TODO: Implement this as part of the wire class. Will require a redesign of the Wire class and interpreter
class WireBus {
    std::vector<Wire*> wires;
    std::string name;
    int size;
public:
    static std::unordered_map<std::string, std::vector<Wire*>> wireBusMap;

    WireBus(std::string name, int size, WIRE_STATE initialState = WIRE_STATE::LOGIC_UNDEFINED) : name(name), size(size) {
        for(size_t i = 0; i < size; ++i) {
            std::string wireName = name + "[" + std::to_string(i) + "]";
            wires.push_back(new Wire(wireName, initialState));
        }
        wireBusMap[name] = wires;
        std::cout << "Creating Wire Bus: " << name << " (" << size-1 << " down to " << "0)" << " with initial state: " << static_cast<int>(initialState) << std::endl;
    };

    void addWire(Wire* wire) {
        wires.push_back(wire);
    }

    const std::vector<Wire*>& getWires() const {
        return wires;
    }
};