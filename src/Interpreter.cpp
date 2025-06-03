#include "includes/Interpreter.h"
#include "includes/Wire.h"
#include "includes/Component.h"
#include "includes/FlipFlop.h"
#include "includes/WireBus.h"
#include "includes/Multiplexer.h"
#include "includes/ROM.h"
#include "gui/gui.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include <regex>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

// Helper function
inline std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower;
}

// Convention: 
// Where input1, input2, and output are wire names
/*
    wire <wire_name> <state>
    <component_type> <component_name> <input1> <input2> <output>

    Example:
    wire A high
    wire B
    AND AND1 A B C
    NOT NOT1 C D
    OR OR1 A D E
*/
void Interpreter::createCircuitTXT() {
    std::vector<std::string> lines = readAllLines();
    std::cout << lines.size() << " lines read from file." << std::endl << "Creating System Components:" << std::endl << std::endl;
    if (lines.empty()) {
        std::cerr << "No lines to process." << std::endl;
        return;
    }
    for (const auto& line : lines) {

        std::istringstream iss(line);
        std::string command;
        iss >> command;
        command = toLower(command);

        // Should redo how it parses so you can have inline comments. Low priority.
        if (command.empty() || command.substr(0, 2) == "//") {
            continue; // Skip empty lines or comments
        }
        if(command == "assign") {
            std::string variable, value;
            iss >> variable >> value;
            Wire* variableWire = Wire::wireMap[variable];

            // This is combinational logic, so I can't see a case where assigning high low to a wire being useful afterwards?
            if(value == "high" || value == "low") {
                WIRE_STATE state = (value == "high") ? WIRE_STATE::LOGIC_HIGH :
                                  (value == "low") ? WIRE_STATE::LOGIC_LOW :
                                  WIRE_STATE::LOGIC_UNDEFINED;
                if (variableWire) {
                    variableWire->setState(state);
                } else {
                    std::vector<Wire*> variableBus = WireBus::wireBusMap[variable];
                    if(!variableBus.empty()) {
                        for (Wire* wire : variableBus) {
                            wire->setState(state);
                        }
                    } else {
                        std::cerr << "Error: Variable " << variable << " not found." << std::endl;
                    }
                }
                continue;
            }
            Wire* valueWire = Wire::wireMap[value];
            if (variableWire) {
                if (!valueWire) {
                    std::cerr << "Error: Value variable " << value << " not found." << std::endl;
                    continue;
                }
                variableWire->setState(valueWire->getState());
            } else {
                std::vector<Wire*> variableBus = WireBus::wireBusMap[variable];
                if (!variableBus.empty()) {
                    for (Wire* wire : variableBus) {
                        wire->setState(valueWire->getState());
                    }
                } else {
                    std::cerr << "Error: Variable " << variable << " not found." << std::endl;
                }
            }
        } else if (command == "wire") {
            std::string wireName;
            std::string stateStr;
            std::smatch match;
            std::regex bus_regex(R"((\w+)\[(\d+):(\d+)\])");

            iss >> wireName >> stateStr;
            stateStr = toLower(stateStr);
            // If defining a bus 
            if (std::regex_match(wireName, match, bus_regex)) {
                std::string name = match[1];
                int high = std::stoi(match[2]);
                int low  = std::stoi(match[3]);
                int size = std::abs(high - low) + 1;

                if(stateStr == "high") {
                    WireBus* bus = new WireBus(name, size, WIRE_STATE::LOGIC_HIGH);
                } else if(stateStr == "low") {
                    WireBus* bus = new WireBus(name, size, WIRE_STATE::LOGIC_LOW);
                } else {
                    WireBus* bus = new WireBus(name, size, WIRE_STATE::LOGIC_UNDEFINED);
                }
                continue;
            } 

            bool isClock = false;
            WIRE_STATE state = WIRE_STATE::LOGIC_UNDEFINED;
            if(stateStr == "high"){
                state = WIRE_STATE::LOGIC_HIGH;
            } else if(stateStr == "low"){
                state = WIRE_STATE::LOGIC_LOW;
            } else if(stateStr == "clk") {
                isClock = true;
            }
            Wire* wire = new Wire(wireName, state);
            if (isClock) {
                wire->setClock(true);
            }
        } else if (command == "not") {
            std::string name, inputA, output;
            iss >> name >> inputA >> output;
            NOT_GATE* component = new NOT_GATE(name);
            component->setInput(Wire::wireMap[inputA], nullptr);
            component->setOutput(Wire::wireMap[output]);
        } else if (command == "and" || command == "or" || command == "xor" || command == "nand" || command == "nor" || command == "xnor") {
            
            std::string name, inputA, inputB, output;
            iss >> name >> inputA >> inputB >> output;

            // Create the component based on the type
            Component* component = nullptr;
            if (command == "and") component = new AND_GATE(name);
            else if (command == "or") component = new OR_GATE(name);
            else if (command == "xor") component = new XOR_GATE(name);
            else if (command == "nand") component = new NAND_GATE(name);
            else if (command == "nor") component = new NOR_GATE(name);
            else if (command == "xnor") component = new XNOR_GATE(name);

            // Set inputs and outputs using the wires created earlier
            // TODO: I should probably put the inputs into the constructor of the component
            component->setInput(Wire::wireMap[inputA], Wire::wireMap[inputB]);
            component->setOutput(Wire::wireMap[output]);
        } else if (command == "dff" || command == "srff" || command == "jkff") {
            std::string name, clk, inputA, inputB, output, edgeType;
            iss >> name >> clk;
            FlipFlop* flipFlop = nullptr;
            if (command == "dff") {
                iss >> inputA >> output >> edgeType;
                if(edgeType == "rising") {
                    flipFlop = new DFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[output], EDGE_TYPE::RISING_EDGE);
                } else if(edgeType == "falling") {
                    flipFlop = new DFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[output], EDGE_TYPE::FALLING_EDGE);
                } else
                    flipFlop = new DFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[output]);
            } else if (command == "srff") {
                iss >> inputA >> inputB >> output >> edgeType;
                if(edgeType == "falling") {
                    flipFlop = new SRFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[inputB], Wire::wireMap[output], EDGE_TYPE::FALLING_EDGE);
                } else if(edgeType == "rising") {
                    flipFlop = new SRFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[inputB], Wire::wireMap[output], EDGE_TYPE::RISING_EDGE);
                } else
                    flipFlop = new SRFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[inputB], Wire::wireMap[output]);
            } else if (command == "jkff") {
                iss >> inputA >> inputB >> output >> edgeType;
                if(edgeType == "rising") {
                    flipFlop = new JKFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[inputB], Wire::wireMap[output], EDGE_TYPE::RISING_EDGE);
                } else if(edgeType == "falling") {
                    flipFlop = new JKFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[inputB], Wire::wireMap[output], EDGE_TYPE::FALLING_EDGE); 
                } else 
                    flipFlop = new JKFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[inputB], Wire::wireMap[output]);
            } else if (command == "tff") {
                iss >> inputA >> output >> edgeType;
                if(edgeType == "falling") {
                    flipFlop = new TFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[output], EDGE_TYPE::FALLING_EDGE);
                } else if(edgeType == "rising") {
                    flipFlop = new TFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[output], EDGE_TYPE::RISING_EDGE);
                } else
                flipFlop = new TFlipFlop(name, Wire::wireMap[clk], Wire::wireMap[inputA], Wire::wireMap[output]);
            }
        } else if (command == "mux" || command == "demux") {
            std::string name, input, output, select, dimensions;
            iss >> dimensions >> name;
            // Both command "mux" and "demux" do the same thing here. They are providing the dimensions anyways 
            // TODO Honestly I shouldn't be using else ifs, should have dynamic sizing, this needs to be refactored
            // This is bad design. 

            // <dimensions> <name> <inputs...> <output> <select>
            // wire A[3:0] 
            // wire B[3:0]
            // wire C[3:0]
            // wire D[3:0]
            // wire out[3:0]
            // wire select[1:0]
            // mux 4x1 A B C D out select
 
            if(dimensions == "1x2") {
                std::string output2;
                iss >> input >> select >> output >> output2;
                Demultiplexer* demux = new Demultiplexer(2, name, WireBus::wireBusMap[input], WireBus::wireBusMap[select], {WireBus::wireBusMap[output], WireBus::wireBusMap[output2]});
            } else if(dimensions == "2x1") {
                std::string input2;
                iss >> input >> input2 >> select >> output;
                Multiplexer* mux = new Multiplexer(2, name, {WireBus::wireBusMap[input], WireBus::wireBusMap[input2]}, WireBus::wireBusMap[select], WireBus::wireBusMap[output]);
            } else if(dimensions == "1x4") {
                std::string output2, output3, output4;
                iss >> input >> select >> output >> output2 >> output3 >> output4;
                Demultiplexer* demux = new Demultiplexer(4, name, WireBus::wireBusMap[input], WireBus::wireBusMap[select], {WireBus::wireBusMap[output], WireBus::wireBusMap[output2], WireBus::wireBusMap[output3], WireBus::wireBusMap[output4]});
            } else if(dimensions == "4x1") {
                std::string input2, input3, input4;
                iss >> input >> input2 >> input3 >> input4 >> select >> output;
                Multiplexer* mux = new Multiplexer(4, name, {WireBus::wireBusMap[input], WireBus::wireBusMap[input2], WireBus::wireBusMap[input3], WireBus::wireBusMap[input4]}, WireBus::wireBusMap[select], WireBus::wireBusMap[output]);
            } else if(dimensions == "1x8") {
                std::string output2, output3, output4, output5, output6, output7;
                iss >> input >> select >> output >> output2 >> output3 >> output4 >> output5 >> output6 >> output7;
                Demultiplexer* demux = new Demultiplexer(8, name, WireBus::wireBusMap[input], WireBus::wireBusMap[select], {WireBus::wireBusMap[output], WireBus::wireBusMap[output2], WireBus::wireBusMap[output3], WireBus::wireBusMap[output4], WireBus::wireBusMap[output5], WireBus::wireBusMap[output6], WireBus::wireBusMap[output7]});
            } else if(dimensions == "8x1") {
                std::string input2, input3, input4, input5, input6, input7;
                iss >> input >> input2 >> input3 >> input4 >> input5 >> input6 >> input7 >> select >> output;
                Multiplexer* mux = new Multiplexer(8, name, {WireBus::wireBusMap[input], WireBus::wireBusMap[input2], WireBus::wireBusMap[input3], WireBus::wireBusMap[input4], WireBus::wireBusMap[input5], WireBus::wireBusMap[input6], WireBus::wireBusMap[input7]}, WireBus::wireBusMap[select], WireBus::wireBusMap[output]);
            } else if(dimensions == "1x16") {
                std::string output2, output3, output4, output5, output6, output7, output8;
                iss >> input >> select >> output >> output2 >> output3 >> output4 >> output5 >> output6 >> output7 >> output8;
                Demultiplexer* demux = new Demultiplexer(16, name, WireBus::wireBusMap[input], WireBus::wireBusMap[select], {WireBus::wireBusMap[output], WireBus::wireBusMap[output2], WireBus::wireBusMap[output3], WireBus::wireBusMap[output4], WireBus::wireBusMap[output5], WireBus::wireBusMap[output6], WireBus::wireBusMap[output7], WireBus::wireBusMap[output8]});
            } else if(dimensions == "16x1") {
                std::string input2, input3, input4, input5, input6, input7, input8;
                iss >> input >> input2 >> input3 >> input4 >> input5 >> input6 >> input7 >> input8 >> select >> output;
                Multiplexer* mux = new Multiplexer(16, name, {WireBus::wireBusMap[input], WireBus::wireBusMap[input2], WireBus::wireBusMap[input3], WireBus::wireBusMap[input4], WireBus::wireBusMap[input5], WireBus::wireBusMap[input6], WireBus::wireBusMap[input7], WireBus::wireBusMap[input8]}, WireBus::wireBusMap[select], WireBus::wireBusMap[output]);
            // Too large. Will get dynamic sizing to work when I refactor.
            // } else if(dimensions == "1x32") {
                
            // } else if(dimensions == "32x1") {

            } else {
                std::cerr << "Unknown dimensions for MUX/DEMUX: " << dimensions << std::endl;
                continue;
            }
        } else if (command == "rom"){
            std::string name, addr, data, memoryFile;
            iss >> name >> addr >> data >> memoryFile;
            std::vector<Wire*> addressBus = WireBus::wireBusMap[addr];
            std::vector<Wire*> outputBus = WireBus::wireBusMap[data];
            ROM* rom = new ROM(name, addressBus, outputBus, memoryFile);
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }
}

std::vector<testbenchInstruction> Interpreter::circuitTestbench(const std::string& testbenchFile) {
    Interpreter interpreter(testbenchFile);
    std::vector<std::string> lines = interpreter.readAllLines();

    std::vector<testbenchInstruction> testbench;
   
    if (lines.empty()) {
        std::cerr << "No lines to process." << std::endl;
        return {};
    }

    for (const auto& line : lines) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        command = toLower(command);
        if (command.empty() || command.substr(0, 2) == "//") {
            continue;
        }
        if (command[0] == '@'){
            int targetedCycle = command[1] - '0';
            iss >> command; // Reassign command to the testbench command
            if(command == "set"){
                std::string wireName, stateStr;
                iss >> wireName >> stateStr;
                stateStr = toLower(stateStr);
                WIRE_STATE state = WIRE_STATE::LOGIC_UNDEFINED;
                if(stateStr == "high"){
                    state = WIRE_STATE::LOGIC_HIGH;
                } else if(stateStr == "low"){
                    state = WIRE_STATE::LOGIC_LOW;
                } else {
                    std::cerr << "Invalid state for wire: " << wireName << std::endl;
                    continue;
                }
                testbench.push_back(testbenchInstruction{targetedCycle, {{Wire::wireMap[wireName], state}}});
            } else {
                std::cerr << "Unknown testbench command: " << command << std::endl;
            }
        }
    }
    return testbench;
}

void Interpreter::runSimulation(std::string designFile, std::string testbenchFile, size_t maxCycles) {
    Wire::wireMap.clear();
    WireBus::wireBusMap.clear();
    Component::components.clear();
    FlipFlop::flipFlops.clear();
    Multiplexer::multiplexers.clear();
    Demultiplexer::demultiplexers.clear();
    ROM::roms.clear();
    
    Interpreter interpreter(designFile);
    interpreter.createCircuitTXT();
    std::vector<testbenchInstruction> testbench = Interpreter::circuitTestbench(testbenchFile);
    std::ostringstream out;
    std::cout << std::endl << "System created: " << Wire::wireMap.size() << " wires, " << Component::components.size() << " components, " << FlipFlop::flipFlops.size() << " flip-flops." << std::endl;
    Wire::wireMap.erase("");

    for (size_t cycle = 0; cycle < maxCycles; ++cycle) {
#ifdef DEBUG
        std::cout << std::endl << "Cycle: " << cycle << std::endl;
#endif

        // Run testbench instruction for current cycle
        for(const testbenchInstruction& instruction : testbench) {
            if (instruction.cycle == cycle) {
                for (const auto& assignment : instruction.assignments) {
                    Wire* wire = assignment.first;
                    WIRE_STATE state = assignment.second;
                    wire->setState(state);
                }
            }
        }

        // Clock
        for(auto& wirePair : Wire::wireMap) {
            Wire* wire = wirePair.second;
            if (wire->isClockWire()) {
                wire->toggle();
            }
        }

        for(auto& mux : Multiplexer::multiplexers) {
            mux->tick();
        }
        for(auto& demux : Demultiplexer::demultiplexers) {
            demux->tick();
        }
        for(auto& rom : ROM::roms) {
            rom->tick();
        }
        Component::evaluateSystem();
        for(auto& flipFlop : FlipFlop::flipFlops) {
            flipFlop->tick();
        }

        // Collect waveform data
        for (const auto& wire : Wire::wireMap) {
            waveform[wire.first].push_back(wire.second->getState());
        }
    }
}