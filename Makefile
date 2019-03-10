# Release targets
TARGET_ENGINE_RELEASE ?= bin/mpigrav
TARGET_ENGINE_DEBUG ?= bin/mpigrav-debug
TARGET_VIEWER_RELEASE ?= bin/mpigrav-viewer
TARGET_VIEWER_DEBUG ?= bin/mpigrav-viewer-debug

# Directory controls
OBJ_DIR_BASE ?= build
OBJ_DIR_RELEASE ?= $(OBJ_DIR_BASE)/release
OBJ_DIR_DEBUG ?= $(OBJ_DIR_BASE)/debug
OBJ_DIR_RELEASE_MPI ?= $(OBJ_DIR_BASE)/release-mpi
OBJ_DIR_DEBUG_MPI ?= $(OBJ_DIR_BASE)/debug-mpi
SRC_DIRS ?= src
INC_DIRS ?= include

# Compiler configuration
CXX := g++
MPICXX := mpicxx
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
BASE_FLAGS ?= -MMD -MP -m64 -fopenmp -std=c++11 -Wall
DEBUG_FLAGS ?= $(INC_FLAGS) $(BASE_FLAGS) -g
RELEASE_FLAGS ?= $(INC_FLAGS) $(BASE_FLAGS) -O3

# Sources which define main functions
MAIN_SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name *.cpp)
MAIN_OBJS_RELEASE := $(MAIN_SRCS:%=$(OBJ_DIR_RELEASE)/%.o)
MAIN_OBJS_DEBUG := $(MAIN_SRCS:%=$(OBJ_DIR_DEBUG)/%.o)
MAIN_OBJS_RELEASE_MPI := $(MAIN_SRCS:%=$(OBJ_DIR_RELEASE_MPI)/%.o)
MAIN_OBJS_DEBUG_MPI := $(MAIN_SRCS:%=$(OBJ_DIR_DEBUG_MPI)/%.o)
MAIN_DEPS := $(MAIN_OBJS_DEBUG:.o=.d) $(SUB_OBJS_RELEASE:.o=.d)

# "Subordinate" sources which do not define mains
SUB_SRCS := $(shell find $(SRC_DIRS) -mindepth 2 -name *.cpp)
SUB_OBJS_RELEASE := $(SUB_SRCS:%=$(OBJ_DIR_RELEASE)/%.o)
SUB_OBJS_DEBUG := $(SUB_SRCS:%=$(OBJ_DIR_DEBUG)/%.o)
SUB_OBJS_RELEASE_MPI := $(SUB_SRCS:%=$(OBJ_DIR_RELEASE_MPI)/%.o)
SUB_OBJS_DEBUG_MPI := $(SUB_SRCS:%=$(OBJ_DIR_DEBUG_MPI)/%.o)
SUB_DEPS := $(SUB_OBJS_DEBUG:.o=.d) $(SUB_OBJS_RELEASE:.o=.d)

# Release object compilation g++
$(OBJ_DIR_DEBUG)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(DEBUG_FLAGS) -c $< -o $@

# Release object compilation mpicxx
$(OBJ_DIR_DEBUG_MPI)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(MPICXX) $(DEBUG_FLAGS) -c $< -o $@

# Release object compilation g++
$(OBJ_DIR_RELEASE)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(RELEASE_FLAGS) -c $< -o $@

# Release object compilation mpicxx
$(OBJ_DIR_RELEASE_MPI)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(MPICXX) $(RELEASE_FLAGS) -c $< -o $@

# Engine release build target
ENGINE_RELEASE_OBJS := $(SUB_OBJS_RELEASE_MPI) $(OBJ_DIR_RELEASE_MPI)/src/engine.cpp.o
engine-release: $(ENGINE_RELEASE_OBJS)
	@$(MKDIR_P) $(dir $(TARGET_ENGINE_RELEASE))
	$(MPICXX) $(ENGINE_RELEASE_OBJS) -o $(TARGET_ENGINE_RELEASE) -loptparse

# Engine debug target
ENGINE_DEBUG_OBJS := $(SUB_OBJS_DEBUG_MPI) $(OBJ_DIR_DEBUG_MPI)/src/engine.cpp.o
engine-debug: $(ENGINE_DEBUG_OBJS)
	@$(MKDIR_P) $(dir $(TARGET_ENGINE_DEBUG))
	$(MPICXX) $(ENGINE_DEBUG_OBJS) -o $(TARGET_ENGINE_DEBUG) -loptparse

# Viewer release target
VIEWER_RELEASE_OBJS := $(SUB_OBJS_DEBUG) $(OBJ_DIR_RELEASE)/src/viewer.cpp.o
viewer-release: $(VIEWER_RELEASE_OBJS)
	@$(MKDIR_P) $(dir $(TARGET_VIEWER_RELEASE))
	$(CXX) $(VIEWER_RELEASE_OBJS) -o $(TARGET_VIEWER_RELEASE) -loptparse -lgltools -lGLEW -lglfw -lGL

# Viewer debug target
VIEWER_DEBUG_OBJS := $(SUB_OBJS_DEBUG) $(OBJ_DIR_DEBUG)/src/viewer.cpp.o
viewer-debug: $(VIEWER_DEBUG_OBJS)
	@$(MKDIR_P) $(dir $(TARGET_VIEWER_DEBUG))
	$(CXX) $(VIEWER_DEBUG_OBJS) -o $(TARGET_VIEWER_DEBUG) -loptparse -lgltools -lGLEW -lglfw -lGL

# Make all targets
release: engine-release viewer-release
debug: engine-debug viewer-debug
all: release debug

# Clean, be careful with this
.PHONY: clean
clean:
	@$(RM) -rv $(OBJ_DIR_BASE)

# Include dependencies
-include $(SUB_DEPS) $(MAIN_DEPS)

# Make directory
MKDIR_P ?= mkdir -p
