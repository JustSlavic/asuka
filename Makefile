PROJECT=Asuka
CXX=g++
CXX_STANDARD=c++17

DEFINITIONS = \
	UI_EDITOR_ENABLED \
	ASUKA_OS_LINUX \

INC_DIR =  \
	common \
	src    \

LIB_DIR=/usr/lib/

# Local static libraries: I DO NOT HAVE ANY LOCAL LIBRARIES
LOCAL_LIBS=

LIBS =     \
	X11    \
	asound

CXXFLAGS =  \
	-Wall   \
	-Werror \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	-std=$(CXX_STANDARD) \

CXXFLAGS += $(addprefix -I, $(INC_DIR))

LDFLAGS = \
	$(addprefix -l, $(LIBS))    \
	$(addprefix -L, $(LIB_DIR)) \

# Settings for debug/release build configurations
ifndef MAKECMDGOALS
	# If nothing is set => no debug info is included in binary, also no flags are set
	SUB_DIR  := plain
else ifeq ($(MAKECMDGOALS),debug)
	# In debug build put debug info into binary, set DEBUG definition
	SUB_DIR  := debug
	CXXFLAGS += -ggdb3
	DEFINITIONS += ASUKA_DEBUG
else ifeq ($(MAKECMDGOALS),release)
	# In release build set optimisation level O2, set RELEASE definition
	SUB_DIR  := release
	CXXFLAGS += -O2
else ifeq ($(MAKECMDGOALS),test)
	# Runs tests
else
	SUB_DIR  := plain
endif

CXXFLAGS += $(addprefix -D, $(DEFINITIONS))

SOURCES = \
	src/linux_main \
	src/asuka      \
	src/sim_region \
	src/world      \
	src/ui/ui         \
	src/ui/ui_editor  \
	src/ui/ui_element \
	common/bmp     \
	common/crc     \
	common/png     \
	common/wav     \
	common/array   \
	common/code_location \
	common/math/float   \
	common/math/vector2 \
	common/math/vector3 \
	common/math/vector4 \
	common/math/color   \
	common/math/matrix4 \
	common/os/file      \
	common/os/time      \
	common/os/memory    \
	common/os/linux/file   \
	common/os/linux/time   \
	common/os/linux/memory \

OBJECTS      := $(addprefix build/$(SUB_DIR)/, $(addsuffix .o,   $(SOURCES)))
SOURCES      := $(addprefix src/,              $(addsuffix .cpp, $(SOURCES)))

STATIC_LIBS  := $(foreach lib, $(LOCAL_LIBS), $(addprefix external/$(lib)/bin/$(SUB_DIR)/lib, $(addsuffix .a, $(lib))))

# CURRENT_DIR  := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# PROJECT_LIB := build/$(SUB_DIR)/lib$(PROJECT).a
PROJECT_EXE := bin/$(SUB_DIR)/$(PROJECT)


# ================= RULES ================= #


# Unconditional rules
.PHONY: clean run

# Silent rules
# .SILENT:

all debug release: $(PROJECT_EXE)

run:
	(cd data ; ../$(PROJECT_EXE))

# ================= UTILITY ================= #

clean:
	@find build -type f -name '*.o' -delete
	@find build -type f -name '*.d' -delete
	@rm -fv build/*/lib$(PROJECT).a
	@rm -fv bin/*/$(PROJECT)
#	@rm -fv generated/*
	@rm -fv run
	@rm -fv test
	@rm -fv perf.data
	@rm -fv perf.data.old
	$(MAKE) -C tests clean
	$(MAKE) -C config_generator clean

# Cleaning local lib's builds
# .PHONY: $(LOCAL_LIBS)
# clean: $(LOCAL_LIBS)
# $(LOCAL_LIBS):
#   $(MAKE) -C $(addprefix libs/, $@) clean


-include $(OBJECTS:.o=.d)


# ================= CONFIG ================== #


# config: generated/config.hpp generated/config.cpp generated/config.scheme.son

# generated/config.hpp generated/config.cpp generated/config.scheme.son: config.son
# 	$(MAKE) -C config_generator
# 	@mkdir -p generated
# 	./config_generator/generator config.son


# ================= PROJECT ================= #


$(PROJECT_EXE): $(OBJECTS)
	@mkdir -p $(dir $@)
	g++ $(OBJECTS) -o $(PROJECT_EXE) $(CXXFLAGS) $(LDFLAGS)

# $(PROJECT_LIB): $(OBJECTS)
# 	ar rcvs $(PROJECT_LIB) $(OBJECTS)

build/$(SUB_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@g++ -MM -MT "$@" $(CXXFLAGS) $< > build/$(SUB_DIR)/$*.d
	g++ $< -c -o $@ $(CXXFLAGS)

# build/$(SUB_DIR)/common/%.o: common/%.cpp
# 	@mkdir -p $(dir $@)
# 	@g++ -MM -MT "$@" $(CXXFLAGS) $< > build/$(SUB_DIR)/$*.d
# 	g++ $< -c -o $@ $(CXXFLAGS)

${STATIC_LIBS}:
	$(MAKE) -C $(shell dirname $(shell dirname $(dir $@))) $(MAKECMDGOALS)
