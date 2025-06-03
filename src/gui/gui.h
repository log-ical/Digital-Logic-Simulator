#include <unordered_map>
#include <vector>
#include <string>
#include "../includes/Wire.h"
#include "nfd.h"

extern std::unordered_map<std::string, std::vector<WIRE_STATE>> waveform;
static bool isDesignModified;
static bool isTestbenchModified;
static std::string lastSavedDesign;
static std::string lastSavedTestbench;
void init();
void gui_saveFile();
void gui_openFile();
void DrawWaveformVisual(const std::unordered_map<std::string, std::vector<WIRE_STATE>>& waveform, int max_cycles);