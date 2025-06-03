# Compiler and flags
CXX = g++
CXXFLAGS = -g -O0 -std=c++17 \
  -Ithird_party/SDL3/include \
  -Ithird_party/imgui \
  -Ithird_party/imgui/backends \
  -Ithird_party/NFD/include \
  -Ithird_party/ImNodes

# Executable name
TARGET = build/logic_sim.exe

# Source and object files
SRCS = src/main.cpp src/logic/Component.cpp src/logic/Wire.cpp src/Interpreter.cpp src/logic/FlipFlop.cpp src/logic/WireBus.cpp src/logic/Multiplexer.cpp src/logic/ROM.cpp \
       third_party/imgui/imgui.cpp \
       third_party/imgui/imgui_draw.cpp \
       third_party/imgui/imgui_tables.cpp \
       third_party/imgui/imgui_widgets.cpp \
       third_party/imgui/imgui_demo.cpp \
       third_party/imgui/backends/imgui_impl_sdl3.cpp \
       third_party/imgui/backends/imgui_impl_opengl3.cpp \
       third_party/NFD/nfd_win.cpp \
       third_party/NFD/nfd_common.c \
       third_party/ImNodes/imnodes.cpp \
       src/gui/gui.cpp \
       src/gui/RTL.cpp

OBJS = $(SRCS:.cpp=.o)

# Resource file
RES = res/resource.res

# NFD libs
LIBS = -lole32 -luuid -lcomdlg32

# SDL3 and OpenGL libs
LDFLAGS = -Lthird_party/SDL3/lib/x64 -lSDL3 -lopengl32

# Default target
all: $(TARGET)

# Link with resource
$(TARGET): $(OBJS) $(RES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS) -mwindows

# Build .res from .rc
$(RES): res/resource.rc res/icon.ico
	windres res/resource.rc -O coff -o $(RES)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(RES) $(TARGET)
