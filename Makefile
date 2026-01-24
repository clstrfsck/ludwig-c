########################################################################
#                                                                      #
#            L      U   U   DDDD   W      W  IIIII   GGGG              #
#            L      U   U   D   D   W    W     I    G                  #
#            L      U   U   D   D   W ww W     I    G   GG             #
#            L      U   U   D   D    W  W      I    G    G             #
#            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              #
#                                                                      #
########################################################################
#                                                                      #
#   Makefile Copyright (C) 2018                                        #
#   Martin Sandiford, Adelaide, Australia                              #
#   All rights reserved.                                               #
#                                                                      #
########################################################################

# This Makefile pretty much requires GNU make now.


#
# Configurable variables
#

# Generate dependency files
GENDEPS := -MMD -MP

ifdef NDEBUG
# These are release flags.
# Works clang++ doesn't understand -Wno-maybe-uninitialized
CXXFLAGS:= -O3 -Wall -std=c++17 -fdiagnostics-color=never
LDFLAGS := -lncurses -flto
CFLAGS  := -O3 -Wall -DMKSTEMP
else
# These are debug flags. Works for both g++ and clang++.
DEFS    := -DDEBUG -D_GLIBCXX_DEBUG
CXXFLAGS:= -g -Wall -Wextra -std=c++17 $(DEFS) -fdiagnostics-color=never
LDFLAGS := -lncurses -flto
CFLAGS  := -g -Wall -DMKSTEMP
endif


#
# Not sure there is much worth configuring below this line.
#

# Directory for build artifacts
BUILD_DIR := build
# Directory for source files
SRC_DIR := src
# Target executable name
LUDWIG := ludwig
HLPBLD := ludwighlpbld

HLPBLD_SRCS := $(SRC_DIR)/ludwighlpbld.cpp
HLPBLD_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(HLPBLD_SRCS))
HLPBLD_DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.d,$(HLPBLD_SRCS))

LUDWIG_SRCS := $(filter-out $(HLPBLD_SRCS), $(wildcard $(SRC_DIR)/*.cpp))
LUDWIG_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(LUDWIG_SRCS))
LUDWIG_DEPS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.d,$(LUDWIG_SRCS))


#
# Targets
#

.PHONY: all clean

all: $(BUILD_DIR) $(LUDWIG) $(HLPBLD)

$(HLPBLD): $(HLPBLD_OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(LUDWIG): $(LUDWIG_OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(GENDEPS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR) $(LUDWIG) $(HLPBLD)

-include $(LUDWIG_DEPS) $(HLPBLD_DEPS)

# EOF
