BUILD := ./build
BUILD_CODING := ./build/coding
BUILD_IO := ./build/io
BUILD_TABLE := ./build/table
BUILD_UTIL := ./build/util
SRC_FOLDER := ./src
SRC := $(wildcard $(SRC_FOLDER)/*.cpp)
# OBJ := $(SRC:$(SRC_FOLDER)/%.cpp=$(BUILD)/%.o)
OBJ := $(SRC:$(SRC_FOLDER)/%.cpp=$(BUILD)/%.o)

SRC_CODING := $(wildcard $(SRC_FOLDER)/coding/*.cpp)
OBJ_CODING := $(SRC_CODING:$(SRC_FOLDER)/coding/%.cpp=$(BUILD)/coding/%.o)

SRC_IO := $(wildcard $(SRC_FOLDER)/io/*.cpp)
OBJ_IO := $(SRC_IO:$(SRC_FOLDER)/io/%.cpp=$(BUILD)/io/%.o)

SRC_TABLE := $(wildcard $(SRC_FOLDER)/table/*.cpp)
OBJ_TABLE := $(SRC_TABLE:$(SRC_FOLDER)/table/%.cpp=$(BUILD)/table/%.o)

SRC_UTIL := $(wildcard $(SRC_FOLDER)/util/*.cpp)
OBJ_UTIL := $(SRC_UTIL:$(SRC_FOLDER)/util/%.cpp=$(BUILD)/util/%.o)

OBJ += $(OBJ_CODING)
OBJ += $(OBJ_IO)
OBJ += $(OBJ_TABLE)
OBJ += $(OBJ_UTIL)

# SRC := $(wildcard src/*.cpp)
# OBJ := $(SRC:%.cpp=%.o)
GCC = g++
# LIB = -L./lib
INCLUDES = -I.\
					-I./include\
					-I./include/coding\
					-I./include/io\
					-I./include/table\
					-I./include/util

FLAGS = -g -pthread

OUTPUT = main.out
MAIN = main.cpp

TEST_OUTPUT = test/test.out
TEST_MAIN = test/test.cpp

all : $(BUILD) CLEAN_INTERMEDIATE_FILES
$(BUILD):
	@echo "<Executing> Building directories ..."
	@mkdir $(BUILD)
	@mkdir $(BUILD_CODING)
	@mkdir $(BUILD_IO)
	@mkdir $(BUILD_TABLE)
	@mkdir $(BUILD_UTIL)
	
$(OUTPUT) : $(OBJ)
	@echo "<Executing> Generating main.out and linking .o files ..."
	@$(GCC) $(MAIN) $^ -o $@ $(INCLUDES) $(FLAGS)
$(word 1,$(dir $(OBJ_CODING)))%.o : $(word 1,$(dir $(SRC_CODING)))%.cpp
	@echo "<Executing> Compiling $< targets ..."
	@$(GCC) -c $< -o $@ $(INCLUDES) $(FLAGS)
$(word 1,$(dir $(OBJ_IO)))%.o : $(word 1,$(dir $(SRC_IO)))%.cpp
	@echo "<Executing> Compiling $< targets ..."
	@$(GCC) -c $< -o $@ $(INCLUDES) $(FLAGS)
$(word 1,$(dir $(OBJ_TABLE)))%.o : $(word 1,$(dir $(SRC_TABLE)))%.cpp
	@echo "<Executing> Compiling $< targets ..."
	@$(GCC) -c $< -o $@ $(INCLUDES) $(FLAGS)
$(word 1,$(dir $(OBJ_UTIL)))%.o : $(word 1,$(dir $(SRC_UTIL)))%.cpp
	@echo "<Executing> Compiling $< targets ..."
	@$(GCC) -c $< -o $@ $(INCLUDES) $(FLAGS)

CLEAN_INTERMEDIATE_FILES : $(OUTPUT)
	@echo "<Executing> Cleaning intermediate files ..."
	@rm -r $(BUILD)
	-@rm -r ./db 2>/dev/null

.PHONY : clean
clean :
	@echo "<Executing> Cleaning target ..."
	-@rm -r $(BUILD) $(OUTPUT) $(TEST_OUTPUT) 2>/dev/null
	-@rm -r ./db 2>/dev/null