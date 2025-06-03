#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
enum class WIRE_STATE {
    LOGIC_LOW,
    LOGIC_HIGH,
    LOGIC_UNDEFINED,
};

class Wire {
    
    private:
        WIRE_STATE state;
        std::string name;
        bool isClock = false;
    public:
        static std::unordered_map<std::string, Wire*> wireMap;
        Wire(std::string name, WIRE_STATE init_state = WIRE_STATE::LOGIC_UNDEFINED) : state(init_state), name(name) {
            std::cout << "Creating Wire: " << name << " with initial state: " << static_cast<int>(state) << std::endl;
            if(name.empty()) {
                std::cout << "Wire name cannot be empty" << std::endl;
                throw std::invalid_argument("Wire name cannot be empty");
            }
            wireMap[name] = this;
        }

        WIRE_STATE getState() const {
            return state;
        }

        std::string getName() const {
            return name;
        }
        void setName(const std::string& wire_name) {
            name = wire_name;
            wireMap[wire_name] = this; // Update the map with the new name
        }

        void setState(WIRE_STATE state) {
            this->state = state;
        }

        bool isLogicHigh() const {
            return state == WIRE_STATE::LOGIC_HIGH;
        }

        bool isLogicLow() const {
            return state == WIRE_STATE::LOGIC_LOW;
        }

        bool isLogicUndefined() const {
            return state == WIRE_STATE::LOGIC_UNDEFINED;
        }

        void setClock(bool clock) {
            isClock = clock;
        }
        bool isClockWire() {
            return isClock;
        }

        // Toggle the state of the wire. Primarily used for toggling the clock wires.
        void toggle() {
            if(state == WIRE_STATE::LOGIC_UNDEFINED)
                // Wire shouldn't be undefined as a clock. Default to low
                state = WIRE_STATE::LOGIC_LOW;
            else if (state == WIRE_STATE::LOGIC_HIGH)
                state = WIRE_STATE::LOGIC_LOW;
            else if (state == WIRE_STATE::LOGIC_LOW)
                state = WIRE_STATE::LOGIC_HIGH;
        }
};