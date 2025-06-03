// Not ideal to have its own class for flip-flops
// TODO - refactor to be part of the Component class

#pragma once
#include <string>
#include <vector>
#include "Wire.h"
#include <iostream>


// Unused for now.
// enum class FLIP_FLOP_TYPE {
//     D_FLIP_FLOP,
//     SR_FLIP_FLOP,
//     JK_FLIP_FLOP,
//     T_FLIP_FLOP
// };

enum class EDGE_TYPE {
    RISING_EDGE,
    FALLING_EDGE
};


class FlipFlop {

public:
    static std::vector<FlipFlop*> flipFlops;
    FlipFlop(std::string name, std::vector<Wire*> inputs, Wire* output, Wire* clock, EDGE_TYPE edgeType = EDGE_TYPE::RISING_EDGE)
        : inputs(inputs), output(output), clock(clock), edgeType(edgeType) {
            this->name = name;
            flipFlops.push_back(this);
        }

    virtual void tick();
    void process();

    std::vector<Wire*> getInputs() const {
        return inputs;
    }

    Wire* getOutput() const {
        return output;
    }

    std::string getName() const {
        return name;
    }

    bool onEdge() const {
        WIRE_STATE currentClock = clock->getState();
        if (edgeType == EDGE_TYPE::RISING_EDGE)
            return (previousClock == WIRE_STATE::LOGIC_LOW && currentClock == WIRE_STATE::LOGIC_HIGH);
        else // falling edge
            return (previousClock == WIRE_STATE::LOGIC_HIGH && currentClock == WIRE_STATE::LOGIC_LOW);
    }

protected:
    void updatePreviousClock() {
        previousClock = clock->getState();
    }
    
    //const FLIP_FLOP_TYPE type;
    std::vector<Wire*> inputs;
    Wire* output;
    Wire* clock;
    std::string name;
    WIRE_STATE previousClock = WIRE_STATE::LOGIC_LOW;
    EDGE_TYPE edgeType;
};

class DFlipFlop : public FlipFlop {
public:
    DFlipFlop(std::string name, Wire* clk, Wire* d, Wire* q, EDGE_TYPE edgeType = EDGE_TYPE::RISING_EDGE) : FlipFlop(name, {d}, q, clk, edgeType) {
        std::cout << "Creating D Flip-Flop: " << name << " D: " << d->getName() << " Q: " << q->getName() << " Clock: " << clk->getName() << std::endl;
    }

    void tick() override {
        if (onEdge()) {
            output->setState(inputs[0]->getState());
#ifdef DEBUG
            std::cout << "D Flip-Flop rising edge: Output set to " << (output->getState() == WIRE_STATE::LOGIC_HIGH ? "HIGH" : "LOW") << std::endl;
#endif
        }
        updatePreviousClock();
    }
};

class SRFlipFlop : public FlipFlop {
public:
    SRFlipFlop(std::string name, Wire* clk, Wire* s, Wire* r, Wire* q, EDGE_TYPE edgeType = EDGE_TYPE::RISING_EDGE) : FlipFlop(name, {s, r}, q, clk, edgeType) {
        std::cout << "Creating SR Flip-Flop: " << name << " S: " << s->getName() << " R: " << r->getName() << " Q: " << q->getName() << " Clock: " << clk->getName() << std::endl;
    }

    void tick() override {
        if (onEdge()) {
            if (inputs[0]->isLogicHigh() && inputs[1]->isLogicLow()) {
                output->setState(WIRE_STATE::LOGIC_HIGH);
            } else if (inputs[0]->isLogicLow() && inputs[1]->isLogicHigh()) {
                output->setState(WIRE_STATE::LOGIC_LOW);
            }
#ifdef DEBUG
            std::cout << "SR Flip-Flop rising edge: Output set to " << (output->getState() == WIRE_STATE::LOGIC_HIGH ? "HIGH" : "LOW") << std::endl;
#endif
        }
        updatePreviousClock();
    }
};

class JKFlipFlop : public FlipFlop {
    
public:
    JKFlipFlop(std::string name, Wire* clk, Wire* j, Wire* k, Wire* q, EDGE_TYPE edgeType = EDGE_TYPE::RISING_EDGE) : FlipFlop(name, {j, k}, q, clk, edgeType) {
        std::cout << "Creating JK Flip-Flop: " << name << " J: " << j->getName() << " K: " << k->getName() << " Q: " << q->getName() << " Clock: " << clk->getName() << std::endl;
    }

    void tick() override {
        if (onEdge()) {
            if(inputs[0]->isLogicLow() && inputs[1]->isLogicHigh()){
                output->setState(WIRE_STATE::LOGIC_LOW);
            }
            else if (inputs[0]->isLogicHigh() && inputs[1]->isLogicLow()) {
                output->setState(WIRE_STATE::LOGIC_HIGH);
            } else if (inputs[0]->isLogicHigh() && inputs[1]->isLogicHigh()) {
                output->setState(output->getState() == WIRE_STATE::LOGIC_HIGH ? WIRE_STATE::LOGIC_LOW : WIRE_STATE::LOGIC_HIGH);
            }
#ifdef DEBUG
            std::cout << "JK Flip-Flop rising edge: Output set to " << (output->getState() == WIRE_STATE::LOGIC_HIGH ? "HIGH" : "LOW") << std::endl;
#endif
        } 
        updatePreviousClock();
    }
};

class TFlipFlop : public FlipFlop {
public:
    TFlipFlop(std::string name, Wire* clk, Wire* t, Wire* q, EDGE_TYPE edgeType = EDGE_TYPE::RISING_EDGE) : FlipFlop(name, {t}, q, clk, edgeType) {
        std::cout << "Creating T Flip-Flop: " << name << " T: " << t->getName() << " Q: " << q->getName() << " Clock: " << clk->getName() << std::endl;
    }

    void tick() override {
        if (onEdge()) {
            if (inputs[0]->isLogicHigh()) {
                output->setState(output->getState() == WIRE_STATE::LOGIC_HIGH ? WIRE_STATE::LOGIC_LOW : WIRE_STATE::LOGIC_HIGH);
            }
#ifdef DEBUG
            std::cout << "T Flip-Flop rising edge: Output set to " << (output->getState() == WIRE_STATE::LOGIC_HIGH ? "HIGH" : "LOW") << std::endl;
#endif
        }
        updatePreviousClock();
    }
};