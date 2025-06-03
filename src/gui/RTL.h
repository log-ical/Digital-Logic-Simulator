#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <imgui.h>

struct RTLNode {
    int id;
    std::string name;
    std::string type;
    std::string inputA;
    std::string inputB;
    std::string output;
    ImVec2 position;
};

struct RTLEdge {
    int id;
    int to;
    int from;
};


// Function declarations
void build_RTL();
void draw_RTL();
ImVec2 addImVec2(const ImVec2& a, const ImVec2& b);
