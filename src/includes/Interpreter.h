#pragma once
#include "Wire.h"
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

struct testbenchInstruction {
    int cycle;
    std::unordered_map<Wire*, WIRE_STATE> assignments;
};

class Interpreter {
public:
    Interpreter(const std::string& filename) : file(filename) {}

    // Reads all lines from the file into a vector
    std::vector<std::string> readAllLines() {
        std::vector<std::string> lines;
        std::string line;
        if (!file.is_open()) return lines;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    // Reads the next line from the file
    bool readNextLine(std::string& line) {
        if (!file.is_open()) return false;
        return static_cast<bool>(std::getline(file, line));
    }

    // I currently use txt files to read the circuit and testbench files. In the future I want to implement JSON, and include that as part of the file format.
    void createCircuitTXT();
    static std::vector<testbenchInstruction> circuitTestbench(const std::string& testbenchFile);
    //void createCircuitJSON();
    //void txtToJSON(const std::string& outputFile);

    // Reset file to beginning
    void reset() {
        if (file.is_open()) {
            file.clear();
            file.seekg(0);
        }
    }

    ~Interpreter() {
        if (file.is_open()) file.close();
    }

    static void runSimulation(std::string designFile, std::string testbenchFile, size_t maxCycles);

private:
    std::ifstream file;
};