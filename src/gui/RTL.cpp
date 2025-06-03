#include "../includes/Component.h"
#include "../includes/Wire.h"
#include "../includes/FlipFlop.h"
#include "../includes/Multiplexer.h"
#include "../includes/ROM.h"
#include "../includes/WireBus.h"
#include "RTL.h"

#include "imnodes.h"

std::vector<RTLNode> RTLNodes;
std::vector<RTLEdge> RTLEdges;

ImVec2 addImVec2(const ImVec2& a, const ImVec2& b) {
    return ImVec2(a.x + b.x, a.y + b.y);
}

void build_RTL() {
    RTLNodes.clear();
    RTLEdges.clear();

    int node_id = 1;
    for (auto* comp : Component::components) {
        RTLNode node;
        node.id = node_id++;
        node.name = comp->getName();
        node.type = comp->typeToString();
        node.inputA = comp->getInputA() ? comp->getInputA()->getName() : "";
        node.inputB = comp->getInputB() ? comp->getInputB()->getName() : "";
        node.output = comp->getOutput() ? comp->getOutput()->getName() : "";
        RTLNodes.push_back(node);
    }

    for (const auto& src : RTLNodes) {
        for (const auto& dst : RTLNodes) {
            if (!src.output.empty() && (dst.inputA == src.output || dst.inputB == src.output)) {
                int output_attr = src.id * 100 + 3;
                int input_attr = dst.inputA == src.output ? (dst.id * 100 + 1) : (dst.id * 100 + 2);
                RTLEdges.push_back({ output_attr * 1000 + input_attr, output_attr, input_attr });
            }
        }
    }
}

void draw_RTL() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing);
    ImGui::Begin("RTL Viewer");
    ImNodes::BeginNodeEditor();

    ImVec2 base_pos = ImGui::GetCursorScreenPos();
    ImVec2 offset = ImVec2(200, 150);
    for (size_t i = 0; i < RTLNodes.size(); ++i) {
        auto& node = RTLNodes[i];
        ImVec2 pos = addImVec2(base_pos, ImVec2((i % 5) * offset.x, (i / 5) * offset.y));
        ImNodes::SetNodeGridSpacePos(node.id, pos);

        ImNodes::BeginNode(node.id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(node.name.c_str());
        ImGui::SameLine();
        ImGui::TextUnformatted(node.type.c_str());
        ImNodes::EndNodeTitleBar();


        if (!node.inputA.empty()) {
            ImNodes::BeginInputAttribute(node.id * 100 + 1);
            ImGui::Text("A: %s", node.inputA.c_str());
            ImNodes::EndInputAttribute();
        }
        if (!node.inputB.empty()) {
            ImNodes::BeginInputAttribute(node.id * 100 + 2);
            ImGui::Text("B: %s", node.inputB.c_str());
            ImNodes::EndInputAttribute();
        }

        if (!node.output.empty()) {
            ImNodes::BeginOutputAttribute(node.id * 100 + 3);
            ImGui::Text("Out: %s", node.output.c_str());
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    for (const auto& edge : RTLEdges) {
        ImNodes::Link(edge.id, edge.from, edge.to);
    }

    ImNodes::EndNodeEditor();
    ImGui::End();
}
