#include "gui.h"
#include "RTL.h"
#include "../includes/Interpreter.h"
#include "../includes/Wire.h"
#include "../includes/Component.h"
#include "../includes/FlipFlop.h"
#include "../includes/Multiplexer.h"
#include "../includes/ROM.h"


#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "nfd.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include "imnodes.h"

#include <iostream>
#include <iomanip>  // for std::setw
#include <algorithm> // for std::sort
#include <vector>
#include <filesystem>
#include <map>
#include <cstdlib>

std::unordered_map<std::string, std::vector<WIRE_STATE>> waveform;
std::string designFilePath, testbenchFilePath;
static char designBuffer[1024 * 16] = "";
static char testbenchBuffer[1024 * 16] = "";
static int cycleCount = 10; 

void open_url(const std::string& url) {
#ifdef _WIN32
    std::string command = "start " + url;
#elif __APPLE__
    std::string command = "open " + url;
#elif __linux__
    std::string command = "xdg-open " + url;
#else
    #error "Unsupported platform"
#endif

    system(command.c_str());
}

void init() {

    // ------------------------ GUI Initialization ------------------------
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Init Failed: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Logic Sim v0.1", 1800, 900, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    std::string font_path = std::filesystem::exists("../third_party/fonts/RobotoMono-Medium.ttf")
        ? "../third_party/fonts/RobotoMono-Medium.ttf"
        : "./third_party/fonts/RobotoMono-Medium.ttf";

    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 25.0f);
    io.FontDefault = io.Fonts->Fonts.back();
    io.Fonts->Build();

    ImGui::StyleColorsDark();

    // Customize ImGui styles
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImVec4 darkColor = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    ImVec4 medColor  = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    ImVec4 lightColor = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]            = darkColor;
    colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBg]            = medColor;
    colors[ImGuiCol_FrameBgHovered]     = lightColor;
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);

    colors[ImGuiCol_TitleBg]            = darkColor;
    colors[ImGuiCol_TitleBgActive]      = darkColor;
    colors[ImGuiCol_MenuBarBg]          = darkColor;

    colors[ImGuiCol_Header]             = medColor;
    colors[ImGuiCol_HeaderHovered]      = lightColor;
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);

    colors[ImGuiCol_Button]             = medColor;
    colors[ImGuiCol_ButtonHovered]      = lightColor;
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);

    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]      = medColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = lightColor;
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    colors[ImGuiCol_CheckMark]          = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    ImVec4 tabColor = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Tab]                = tabColor;
    colors[ImGuiCol_TabHovered]        = tabColor;
    colors[ImGuiCol_TabActive]         = tabColor;
    colors[ImGuiCol_TabUnfocused]      = tabColor;
    colors[ImGuiCol_TabUnfocusedActive] = tabColor;

    // Style tweaks
    style.TabRounding = style.FrameRounding = style.GrabRounding = style.ScrollbarRounding = 6.0f;
    style.TabBorderSize = 2.0f;
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(10, 8);

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");


    // ------------ Main loop -----------------
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport(); 

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) { 
                    std::ofstream file("design.txt");
                    std::ofstream file1("testbench.txt");
                    file.close();
                    file1.close();
                    designFilePath = "design.txt";
                    testbenchFilePath = "testbench.txt";
                    strcpy(designBuffer, "");
                    strcpy(testbenchBuffer, "");
                    isDesignModified = false;
                    isTestbenchModified = false;
                    lastSavedDesign = std::string(designBuffer);
                    lastSavedTestbench = std::string(testbenchBuffer);
                }
                if (ImGui::MenuItem("Open...")) { 
                    gui_openFile();
                }
                if (ImGui::MenuItem("Save")) { 
                    gui_saveFile();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { return; } 
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) { 
                    open_url("https://github.com/log-ical/FPGA-Inspired-Logic-Simulator");
                }
                ImGui::EndMenu();
            } 

            ImGui::EndMainMenuBar();
        }

        ImGuiID dockspace_id = ImGui::GetID("ApplicationDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();


        static bool dock_initialized = false;
        if (!dock_initialized) {
            dock_initialized = true;
            ImGui::DockBuilderRemoveNode(dockspace_id); 
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoTabBar);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID dock_main_id = dockspace_id;
            //ImGuiID dock_design  = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_None, 0.7f, nullptr, &dock_main_id);
            ImGuiID dock_left    = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.35f, nullptr, &dock_main_id);
            ImGuiID dock_right   = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.35f, nullptr, &dock_main_id);
            ImGuiID dock_bottom  = ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.65f, nullptr, &dock_right);

            ImGui::DockBuilderDockWindow("Testbench", dock_left);
            ImGui::DockBuilderDockWindow("Component Tree", dock_right);
            ImGui::DockBuilderDockWindow("Command Toolbar", dock_right);
            ImGui::DockBuilderDockWindow("Design", dock_main_id);
            ImGui::DockBuilderDockWindow("Waveform Viewer", dock_bottom);
            ImGui::DockBuilderDockWindow("RTL Viewer", dock_left);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::Begin("Testbench", nullptr, ImGuiWindowFlags_NoCollapse);
        if (isTestbenchModified) {
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();
            ImVec2 corner = ImVec2(pos.x + size.x, pos.y + size.y);
            ImGui::GetWindowDrawList()->AddRect(pos, corner, IM_COL32(255, 255, 255, 200), 0, 0, 2.0f);
        }
        if(!testbenchFilePath.empty()) {

            if (strcmp(testbenchBuffer, lastSavedTestbench.c_str()) != 0)
                isTestbenchModified = true;
            
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
                std::ofstream out(testbenchFilePath);

                out << testbenchBuffer;
                out.close();
                lastSavedTestbench = std::string(testbenchBuffer);
                isTestbenchModified = false;
            }

            ImGui::InputTextMultiline("##editor", testbenchBuffer, IM_ARRAYSIZE(testbenchBuffer),
                ImGui::GetContentRegionAvail(),
                ImGuiInputTextFlags_AllowTabInput);
            ImGui::End();
        } else {
            ImGui::Text("No testbench file selected.");
            ImGui::End();
        }

        ImGui::Begin("Design", nullptr, ImGuiWindowFlags_NoCollapse);
        if (isDesignModified) {
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();
            ImVec2 corner = ImVec2(pos.x + size.x, pos.y + size.y);
            ImGui::GetWindowDrawList()->AddRect(pos, corner, IM_COL32(255, 255, 255, 200), 0, 0, 2.0f);
        }
        if(!designFilePath.empty()) {
            if (strcmp(designBuffer, lastSavedDesign.c_str()) != 0)
                isDesignModified = true;

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
                std::ofstream out(designFilePath);

                out << designBuffer;
                out.close();
                lastSavedDesign = std::string(designBuffer);
                isDesignModified = false;
            }
            ImGui::InputTextMultiline("##editor", designBuffer, IM_ARRAYSIZE(designBuffer),
                ImGui::GetContentRegionAvail(),
                ImGuiInputTextFlags_AllowTabInput);
            ImGui::End();
        } else {
            ImGui::Text("No design file selected.");
            ImGui::End();
        }
        
        

        ImGui::Begin("Command Toolbar", nullptr, ImGuiWindowFlags_NoCollapse);

        
        
        ImVec2 buttonSizeFile = ImVec2((ImGui::GetContentRegionAvail().x/3)-5, 0);
        ImVec2 buttonSizeOther = ImVec2(ImGui::GetContentRegionAvail().x, 0);

        if(ImGui::Button("New", buttonSizeFile)){
            std::ofstream file("design.txt");
            std::ofstream file1("testbench.txt");
            file.close();
            file1.close();
            designFilePath = "design.txt";
            testbenchFilePath = "testbench.txt";
            strcpy(designBuffer, "");
            strcpy(testbenchBuffer, "");
            isDesignModified = false;
            isTestbenchModified = false;
            lastSavedDesign = std::string(designBuffer);
            lastSavedTestbench = std::string(testbenchBuffer);
        }
        ImGui::SameLine();
        if(ImGui::Button("Open", buttonSizeFile)){
            gui_openFile();
        }
        ImGui::SameLine();
        if(ImGui::Button("Save", buttonSizeFile)){
            gui_saveFile();
        }

        ImGui::Separator();
        
        if (ImGui::Button("Choose Design File", buttonSizeOther)) {
            nfdchar_t *outPath = NULL;
            nfdresult_t result = NFD_OpenDialog(
                NULL,
                NULL,
                &outPath
            );

            if (result == NFD_OKAY) {
                designFilePath = outPath;
                std::ifstream file(outPath); 
                if (!file) {
                    std::cerr << "Failed to open file: " << outPath << std::endl;
                } else {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string fileContent = buffer.str();
                    strcpy(designBuffer, fileContent.c_str());
                    lastSavedDesign = fileContent;
                    file.close();
                }
                free(outPath); 
            }
            else if (result == NFD_CANCEL) {
                std::cout << "User canceled file dialog." << std::endl;
            }
            else {
                std::cerr << "Error: " << NFD_GetError() << std::endl;
            }
        }
        if (ImGui::Button("Choose Testbench File", buttonSizeOther)) {
            nfdchar_t *outPath = NULL;
            nfdresult_t result = NFD_OpenDialog(
                NULL,
                NULL,
                &outPath
            );

            if (result == NFD_OKAY) {
                testbenchFilePath = outPath;
                std::ifstream file(outPath); 
                if (!file) {
                    std::cerr << "Failed to open file: " << outPath << std::endl;
                } else {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string fileContent = buffer.str();
                    strcpy(testbenchBuffer, fileContent.c_str()); 
                    lastSavedTestbench = fileContent;
                    file.close();
                }
                free(outPath); 
            }
            else if (result == NFD_CANCEL) {
                std::cout << "User canceled file dialog." << std::endl;
            }
            else {
                std::cerr << "Error: " << NFD_GetError() << std::endl;
            }
        }
        ImGui::Separator();

        static bool drawRTL = false;
        if (ImGui::Button("RTL View (WIP)", buttonSizeOther)) {
            if (designFilePath.empty() || testbenchFilePath.empty()) {
                ImGui::OpenPopup("Error");
            } else {
                waveform.clear();
                Interpreter::runSimulation(designFilePath, testbenchFilePath, cycleCount);
            }
            drawRTL = true;
            build_RTL();
        }
        if(drawRTL){
            draw_RTL();
        }

        // if (ImGui::Button("Waveform View")) {
        //     if (designFilePath.empty() || testbenchFilePath.empty()) {
        //         ImGui::OpenPopup("Error");
        //     } else {
        //         waveform.clear();
        //         Interpreter::runSimulation(designFilePath, testbenchFilePath, cycleCount);
        //     }
        //     ImGui::OpenPopup("Waveform");
        // }
        ImGui::Text("Clock Cycles:");
        ImGui::SameLine();
        ImGui::PushItemWidth(200.0f);
        ImGui::InputInt("##Cycles", &cycleCount, 1, 5);
        ImGui::PopItemWidth();
        if (cycleCount < 2) {
            cycleCount = 2;
        } else if (cycleCount > 99) {
            cycleCount = 99;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // waveform popup
        
        if (ImGui::BeginPopupModal("Waveform", NULL)) {
            float close_button_height = ImGui::GetFrameHeightWithSpacing() + 10.0f;
            float child_height = ImGui::GetContentRegionAvail().y - close_button_height;

            // Horizontal scrolling region
            ImGui::BeginChild("WaveformRegion", ImVec2(0, child_height), true, ImGuiWindowFlags_HorizontalScrollbar);
                DrawWaveformVisual(waveform, cycleCount);
            ImGui::EndChild();
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
        
            ImGui::EndPopup();
        }

        ImGui::End();



        ImGui::Begin("Component Tree", nullptr, ImGuiWindowFlags_NoCollapse);
        if(designFilePath.empty() || testbenchFilePath.empty()) {
                ImGui::Text("Please select design\nand testbench files first.");
        } else {
            ImGui::Text("Append components to the\ndesign file by clicking '+'");
            if (ImGui::CollapsingHeader("Combinational Logic", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::TreeNode("Logic Gates")) {
                    static const char logicGateNames[7][5] = {"AND", "OR", "NOT", "NAND", "NOR", "XOR", "XNOR"};

                    for(int i = 0; i < 7; ++i) {
                        ImGui::PushID(logicGateNames[i]);
                        ImGui::InputText(("##input" + std::to_string(i)).c_str(), (char*)logicGateNames[i], IM_ARRAYSIZE(logicGateNames[i]));
                        ImGui::SameLine();
                        if (ImGui::SmallButton("+")) {
                            std::string name = logicGateNames[i];
                            if(i == 2){ // Not sure why i'm hardcoding this? I should refactor the component class. 
                                strcat(designBuffer, "NOT <name> <input> <output>\n");
                            } else {
                                strcat(designBuffer, (name + " <name> <inputA> <inputB> <output>\n").c_str());
                            }
                            isTestbenchModified = true;
                        }
                        ImGui::PopID();

                    }
                    ImGui::TreePop();
                }
            
                if (ImGui::TreeNode("Multiplexer")) {
                    static char muxName[4][32] = {"2-to-1 Multiplexer", "4-to-1 Multiplexer", "8-to-1 Multiplexer", "16-to-1 Multiplexer"};
                    for (int i = 0; i < 4; ++i) {
                        ImGui::PushID(muxName[i]);
                        ImGui::InputText(("##input" + std::to_string(i)).c_str(), muxName[i], IM_ARRAYSIZE(muxName[i]));
                        ImGui::SameLine();
                        if (ImGui::SmallButton("+")) {
                            switch(i) {
                                case 0: strcat(designBuffer, "MUX 2x1 <name> <inputBusA> <inputBusB> <selectBus> <outputBus>\n"); break;
                                case 1: strcat(designBuffer, "MUX 4x1 <name> <inputBusA> <inputBusB> <inputBusC> <inputBusD> <selectBus> <outputBus>\n"); break;
                                case 2: strcat(designBuffer, "MUX 8x1 <name> <inputBusA> <inputBusB> ... <inputBusH> <selectBus> <outputBus>\n"); break;
                                case 3: strcat(designBuffer, "MUX 16x1 <name> <inputBusA> <inputBusB> ... <inputBusP> <selectBus> <outputBus>\n"); break;
                            }
                            isTestbenchModified = true;
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            
                if (ImGui::TreeNode("Demultiplexer")) {
                    static char demuxName[4][32] = {"1-to-2 Demultiplexer", "1-to-4 Demultiplexer", "1-to-8 Demultiplexer", "1-to-16 Demultiplexer"};
                    for (int i = 0; i < 4; ++i) {
                        ImGui::PushID(demuxName[i]);
                        ImGui::InputText(("##input" + std::to_string(i)).c_str(), demuxName[i], IM_ARRAYSIZE(demuxName[i]));
                        ImGui::SameLine();
                        if (ImGui::SmallButton("+")) {
                            switch(i) {
                                case 0: strcat(designBuffer, "DEMUX 1x2 <name> <inputBus> <selectBus> <outputBusA> <outputBusB>\n"); break;
                                case 1: strcat(designBuffer, "DEMUX 1x4 <name> <inputBus> <selectBus> <outputBusA> ... <outputBusD>\n"); break;
                                case 2: strcat(designBuffer, "DEMUX 1x8 <name> <inputBus> <selectBus> <outputBusA> ... <outputBusH>\n"); break;
                                case 3: strcat(designBuffer, "DEMUX 1x16 <name> <inputBus> <selectBus> <outputBusA> ... <outputBusP>\n"); break;
                            }
                            isTestbenchModified = true;
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            
                if (ImGui::TreeNode("ROM")) {
                    static char romName[64] = "ROM";
                    ImGui::PushID("ROM");
                    ImGui::InputText("##input", romName, IM_ARRAYSIZE(romName));
                    ImGui::SameLine();
                    if (ImGui::SmallButton("+")) {
                        strcat(designBuffer, "ROM <name> <addressBus> <dataBus> <memoryFilePath>\n");
                        isTestbenchModified = true;
                    }
                    ImGui::PopID();
                    ImGui::TreePop();
                }
            }
        
            if (ImGui::CollapsingHeader("Sequential Logic", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::TreeNode("Flip Flops")) {
                    static const char ffName[4][64] = {"D Flip Flop", "JK Flip Flop", "T Flip Flop", "SR Flip Flop"};
                    for (int i = 0; i < 4; ++i) {
                        ImGui::PushID(ffName[i]);
                        ImGui::InputText(("##input" + std::to_string(i)).c_str(), (char*)ffName[i], IM_ARRAYSIZE(ffName[i]));
                        ImGui::SameLine();
                        if (ImGui::SmallButton("+")) {
                            switch(i) {
                                case 0: strcat(designBuffer, "DFF <name> <clock> <inputD> <outputQ> <default: rising/falling>\n"); break;
                                case 1: strcat(designBuffer, "JKFF <name> <clock> <inputJ> <inputK> <outputQ> <default: rising/falling>\n"); break;
                                case 2: strcat(designBuffer, "TFF <name> <clock> <inputT> <outputQ> <default: rising/falling>\n"); break;
                                case 3: strcat(designBuffer, "SRFF <name> <clock> <inputS> <inputR> <outputQ> <default: rising/falling>\n"); break;
                            }
                            isTestbenchModified = true;
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            }

            if (ImGui::CollapsingHeader("Wires", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushID("Wire");
                char wireName[] = "Wire";
                ImGui::InputText("##input", wireName, IM_ARRAYSIZE(wireName));
                ImGui::SameLine();
                if (ImGui::SmallButton("+")) {
                    strcat(designBuffer, "WIRE <name> <optional: high/low>\n");
                    isTestbenchModified = true;
                }
                ImGui::PopID();

                ImGui::PushID("Wire Bus");
                char wireBusName[] = "Wire Bus";
                ImGui::InputText("##input", wireBusName, IM_ARRAYSIZE(wireBusName));
                ImGui::SameLine();
                if (ImGui::SmallButton("+")) {
                    strcat(designBuffer, "WIRE <name>[<number>:<number>] <optional: high/low>\n");
                    isTestbenchModified = true;
                }
                ImGui::PopID();

                ImGui::PushID("Clock");
                char clockName[] = "Clock";
                ImGui::InputText("##input", clockName, IM_ARRAYSIZE(clockName));
                ImGui::SameLine();
                if (ImGui::SmallButton("+")) {
                    strcat(designBuffer, "WIRE <name> clk\n");
                    isTestbenchModified = true;
                }
                ImGui::PopID();
            }
        }
        ImGui::End();

        // Waveform Panel
        bool showWaveForm = false;
        ImGui::Begin("Waveform Viewer", nullptr, ImGuiWindowFlags_NoCollapse);
            if(ImGui::Button("Run Simulation", ImVec2(ImGui::GetContentRegionAvail().x, 0))){
                waveform.clear();
                Interpreter::runSimulation(designFilePath, testbenchFilePath, cycleCount);
                showWaveForm = true;
            }
            DrawWaveformVisual(waveform, cycleCount);
        ImGui::End();


        // ---- Render ----
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void DrawWaveformVisual(const std::unordered_map<std::string, std::vector<WIRE_STATE>>& waveform, int max_cycles) {
    const float x_scale = 50.0f;  // horizontal spacing per cycle
    const float y_step  = 35.0f;  // vertical space per wire
    const float line_height = 20.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    origin.y += 10.f;

    for (int i = 0; i < max_cycles; ++i) {
        float x = origin.x + 100.0f + i * x_scale;
        draw_list->AddText(ImVec2(x, origin.y - 15), IM_COL32_WHITE, std::to_string(i).c_str());
        draw_list->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + y_step * waveform.size()), IM_COL32(80, 80, 80, 100));
    }

    origin.y += 20.f;

    int row = 0;

    std::map<std::string, std::vector<WIRE_STATE>> sorted_map(waveform.begin(), waveform.end());

    for (const auto& [name, states] : sorted_map) {
        float y = origin.y + row * y_step;
        ImVec2 label_pos = ImVec2(origin.x, y);
        draw_list->AddText(label_pos, IM_COL32_WHITE, name.c_str());

        float x_start = origin.x + 100.0f;

        for (int i = 0; i < std::min(max_cycles - 1, (int)states.size() - 1); ++i) {
            float x0 = x_start + i * x_scale;
            float x1 = x_start + (i + 1) * x_scale;

            WIRE_STATE s0 = states[i];
            WIRE_STATE s1 = states[i + 1];

            ImU32 color = IM_COL32(128, 128, 128, 255); // default undefined
            if (s0 == WIRE_STATE::LOGIC_HIGH)
                color = IM_COL32(0, 255, 0, 255); // green
            else if (s0 == WIRE_STATE::LOGIC_LOW)
                color = IM_COL32(255, 0, 0, 255); // red

            float y0 = (s0 == WIRE_STATE::LOGIC_HIGH) ? y : y + line_height;
            float y1 = (s1 == WIRE_STATE::LOGIC_HIGH) ? y : y + line_height;

            if (s0 == WIRE_STATE::LOGIC_UNDEFINED || s1 == WIRE_STATE::LOGIC_UNDEFINED) {
                // Draw a dashed gray line for undefined
                for (float dash = x0; dash < x1; dash += 4.0f) {
                    draw_list->AddLine(ImVec2(dash, y + line_height / 2), ImVec2(dash + 2.0f, y + line_height / 2), IM_COL32(128, 128, 128, 100), 1.0f);
                }
            } else {
                // Horizontal segment
                draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y0), color, 2.0f);

                // Vertical edge (rising or falling)
                if (s0 != s1)
                    draw_list->AddLine(ImVec2(x1, y0), ImVec2(x1, y1), color, 2.0f);
            }
        }

        row++;
    }
    //ImGui::Text("Simulated %d cycles", max_cycles);
    float content_width = 100.0f + max_cycles * x_scale;
    ImGui::Dummy(ImVec2(content_width, row * y_step + 20));
}

void gui_openFile() {
    // TODO: Expand the functionality of the lsim file format
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog(
        "lsim",  // Filter for .lsim files
        NULL,
        &outPath
    );
    if (result == NFD_OKAY) {
        std::ifstream file(outPath);
        if (!file) {
            std::cerr << "Failed to open file: " << outPath << std::endl;
        } else {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string fileContent = buffer.str();
            
            std::istringstream iss(fileContent);
            std::string line;
            std::string version;

            // This is not good. Hence the TODO above.
            std::getline(iss, version); // [Version]
            std::getline(iss, version); // actual version number
            if (version != "0.1") {
                std::cerr << "Unsupported file version: " << version << std::endl;
                return;
            }

            std::getline(iss, line); // [Design]
            std::getline(iss, designFilePath); // Read design file path
            std::getline(iss, line); // [Testbench]
            std::getline(iss, testbenchFilePath); // Read testbench file path


            std::ifstream file(testbenchFilePath); 
            if (!file) {
                std::cerr << "Failed to open file: " << testbenchFilePath << std::endl;
            } else {
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string fileContent = buffer.str();
                strcpy(testbenchBuffer, fileContent.c_str()); 
                lastSavedTestbench = fileContent;
                file.close();
            }
            file.close();
            std::ifstream designFile(designFilePath);
            if (!designFile) {
                std::cerr << "Failed to open file: " << designFilePath << std::endl;
            } else {
                std::stringstream buffer;
                buffer << designFile.rdbuf();
                std::string fileContent = buffer.str();
                strcpy(designBuffer, fileContent.c_str());
                lastSavedDesign = fileContent;
                designFile.close();
            }
        }
        free(outPath);
    } else if (result == NFD_CANCEL) {
        std::cout << "User canceled open dialog." << std::endl;
    } else {
        std::cerr << "Error: " << NFD_GetError() << std::endl;
    }
}

void gui_saveFile() {
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_SaveDialog(
        "lsim",
        NULL,
        &outPath
    );
    if (result == NFD_OKAY) {
        std::string pathStr = outPath;
        if (pathStr.size() < 5 || pathStr.substr(pathStr.size() - 5) != ".lsim") {
            pathStr += ".lsim";
        }
        std::ofstream out(pathStr);
        if (out.is_open()) {
            out << "[Version]" << std::endl << "0.1" << std::endl;
            out << "[Design]" << std::endl;
            out << designFilePath << std::endl;
            out << "[Testbench]" << std::endl;
            out << testbenchFilePath << std::endl;
            out.close();
        } else {
            std::cerr << "Failed to open file for writing: " << pathStr << std::endl;
        }
        free(outPath);
    } else if (result == NFD_CANCEL) {
        std::cout << "User canceled save dialog." << std::endl;
    } else {
        std::cerr << "Error: " << NFD_GetError() << std::endl;
    }
}