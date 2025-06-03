#include "WireBus.h"
#include "Wire.h"
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <bitset>
#include <iomanip>

class ROM {
    std::vector<Wire*> addressBus;
    std::vector<Wire*> outputBus;
    std::unordered_map<int, std::vector<WIRE_STATE>> memory;
    std::string name;
    std::string filename;
public:
    static std::vector<ROM*> roms;
    ROM(std::string name,
        const std::vector<Wire*>& addressBus,
        const std::vector<Wire*>& outputBus,
        const std::string filename)
        : name(name), addressBus(addressBus), outputBus(outputBus), filename(filename) {

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening memory file: " << filename << std::endl;
            return;
        }
        std::string line;
        int address = 0;
        while (std::getline(file, line)) {
            std::stringstream iss(line);
            std::vector<WIRE_STATE> binaryStates;
            unsigned int hexValue;
            iss >> std::hex >> hexValue;

            std::cout << "Hex value: " << hexValue << " at address: " << address << std::endl;
            
            std::bitset<8> bits(hexValue);
            for (int i = 0; i < 8; ++i) {
                binaryStates.push_back(bits[i] ? WIRE_STATE::LOGIC_HIGH : WIRE_STATE::LOGIC_LOW);
            }
            memory[address++] = binaryStates;
        }
        file.close();
        roms.push_back(this);
        std::cout << "ROM " << name << " loaded with " << memory.size() << " entries from " << filename << std::endl;
    }

    void tick() {
        int address = 0;
        for (size_t i = 0; i < addressBus.size(); ++i)
            if (addressBus[i]->isLogicHigh())
                address |= (1 << i);

        auto it = memory.find(address);
        if (it != memory.end()) {
            const auto& data = it->second;
            for (size_t i = 0; i < outputBus.size(); ++i) {
                outputBus[i]->setState(data[i]);
            }
        }
    }
};
