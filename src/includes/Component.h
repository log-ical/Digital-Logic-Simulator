#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "Wire.h"
#include <iostream>

enum class COMPONENT {
    AND,
    OR,
    NOT,
    XOR,
    NAND,
    NOR,
    XNOR,
};


class Component{
private:
    uint32_t uid;
    static uint32_t next_uid;
protected:
    std::string name;
    const COMPONENT componentType;
    Wire* input_A;
    Wire* input_B;
    Wire* output;
public:
    static std::vector<Component*> components;
    Component(std::string name, COMPONENT component) : name(name), componentType(component), uid(next_uid++) {
        //std::cout << "Creating Component UID: " << uid << ", Type: " << static_cast<int>(componentType) << std::endl;
        components.push_back(this);
    };
    uint32_t getUid() const {
        return uid;
    }
    void setName(std::string name);
    std::string getName() const {
        return name;
    }
    COMPONENT getComponentType() const {
        return componentType;
    }
    void setInput(Wire* input_A, Wire* input_B){
        this->input_A = input_A;
        this->input_B = input_B;
    }
    Wire* getInputA() const {
        return input_A;
    }
    Wire* getInputB() const {
        return input_B;
    }
    Wire* getOutput() const {
        return output;
    }

    // std::vector<Wire*> getInputs() const {
    //     return componentType == COMPONENT::OR : {input_A} ? {input_A, input_B};
    // }

    void setOutput(Wire* output){
        this->output = output;
    }
    static void evaluateSystem();
    virtual void evaluateComponent();

    std::string typeToString() const {
        switch (componentType) {
            case COMPONENT::AND: return "AND";
            case COMPONENT::OR: return "OR";
            case COMPONENT::NOT: return "NOT";
            case COMPONENT::XOR: return "XOR";
            case COMPONENT::NAND: return "NAND";
            case COMPONENT::NOR: return "NOR";
            case COMPONENT::XNOR: return "XNOR";
            default: return "Unknown Component Type";
        }
    }
};

class AND_GATE : public Component {
public:
    AND_GATE(std::string name) : Component(name, COMPONENT::AND) {
        std::cout << "Creating AND Gate: " << name << " UID: " << getUid() << std::endl;
    }

    void evaluateComponent() override {
        bool result = true;
        if (input_A->getState() != WIRE_STATE::LOGIC_HIGH || input_B->getState() != WIRE_STATE::LOGIC_HIGH) {
            result = false;
        }
        output->setState(result ? WIRE_STATE::LOGIC_HIGH : WIRE_STATE::LOGIC_LOW);
#ifdef DEBUG
        std::cout << "AND Gate evaluated: " << (result ? "HIGH" : "LOW") << std::endl;
#endif
    }
};

class OR_GATE : public Component {
public:
    OR_GATE(std::string name) : Component(name, COMPONENT::OR) {
        std::cout << "Creating OR Gate: " << name << " UID: " << getUid() << std::endl;
    }

    void evaluateComponent() override {
        bool result = false;
        if (input_A->getState() == WIRE_STATE::LOGIC_HIGH || input_B->getState() == WIRE_STATE::LOGIC_HIGH) {
            result = true;
        }
        output->setState(result ? WIRE_STATE::LOGIC_HIGH : WIRE_STATE::LOGIC_LOW);
#ifdef DEBUG
        std::cout << "OR Gate evaluated: " << (result ? "HIGH" : "LOW") << std::endl;
#endif
    }
};

class NOT_GATE : public Component {
public:
    NOT_GATE(std::string name) : Component(name, COMPONENT::NOT) {
        std::cout << "Creating NOT Gate: " << name << " UID: " << getUid() << std::endl;
    }

    void evaluateComponent() override {
        if (input_A->getState() == WIRE_STATE::LOGIC_HIGH) {
            output->setState(WIRE_STATE::LOGIC_LOW);
        } else {
            output->setState(WIRE_STATE::LOGIC_HIGH);
        }
#ifdef DEBUG
        std::cout << "NOT Gate evaluated: " << (output->getState() == WIRE_STATE::LOGIC_HIGH ? "HIGH" : "LOW") << std::endl;
#endif
    }
};

class XOR_GATE : public Component {
public:
    XOR_GATE(std::string name) : Component(name, COMPONENT::XOR) {
        std::cout << "Creating XOR Gate: " << name << " UID: " << getUid() << std::endl;
    }

    void evaluateComponent() override {
        bool result = false;
        if (input_A->getState() == WIRE_STATE::LOGIC_HIGH) {
            result = !result;
        }
        if (input_B->getState() == WIRE_STATE::LOGIC_HIGH) {
            result = !result;
        }
        output->setState(result ? WIRE_STATE::LOGIC_HIGH : WIRE_STATE::LOGIC_LOW);
#ifdef DEBUG
        std::cout << "XOR Gate evaluated: " << (result ? "HIGH" : "LOW") << std::endl;
#endif
    }
};

class NAND_GATE : public Component {
public:
    NAND_GATE(std::string name) : Component(name, COMPONENT::NAND) {
        std::cout << "Creating NAND Gate: " << name << " UID: " << getUid() << std::endl;
    }

    void evaluateComponent() override {
        bool result = true;
        if (input_A->getState() != WIRE_STATE::LOGIC_HIGH || input_B->getState() != WIRE_STATE::LOGIC_HIGH) {
            result = false;
        }
        output->setState(result ? WIRE_STATE::LOGIC_LOW : WIRE_STATE::LOGIC_HIGH);
#ifdef DEBUG
        std::cout << "NAND Gate evaluated: " << (result ? "LOW" : "HIGH") << std::endl;
#endif
    }
};

class NOR_GATE : public Component {
public:
    NOR_GATE(std::string name) : Component(name, COMPONENT::NOR) {
        std::cout << "Creating NOR Gate: " << name << " UID: " << getUid() << std::endl;
    }

    void evaluateComponent() override {
        bool result = true;
        if (input_A->getState() == WIRE_STATE::LOGIC_HIGH || input_B->getState() == WIRE_STATE::LOGIC_HIGH) {
            result = false;
        }
        output->setState(result ? WIRE_STATE::LOGIC_HIGH : WIRE_STATE::LOGIC_LOW);
#ifdef DEBUG
        std::cout << "NOR Gate evaluated: " << (result ? "HIGH" : "LOW") << std::endl;
#endif
    }
};

class XNOR_GATE : public Component {
public:
    XNOR_GATE(std::string name) : Component(name, COMPONENT::XNOR) {
        std::cout << "Creating XNOR Gate: " << name << " UID: " << getUid() << std::endl;

    }

    void evaluateComponent() override {
        bool result = true;
        if (input_A->getState() == WIRE_STATE::LOGIC_HIGH) {
            result = !result;
        }
        if (input_B->getState() == WIRE_STATE::LOGIC_HIGH) {
            result = !result;
        }
        output->setState(result ? WIRE_STATE::LOGIC_HIGH : WIRE_STATE::LOGIC_LOW);
#ifdef DEBUG
        std::cout << "XNOR Gate evaluated: " << (result ? "HIGH" : "LOW") << std::endl;
#endif
    }
};