.PHONY: all clean

CC = $(if $(nobear),,bear --append --output compile_commands.json -- )clang
CFLAGS = -fdiagnostics-color=always -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef			 \
		 -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs 		 \
		 -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -pipe -fexceptions -Wcast-qual	 \
		 -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers \
		 -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual 			 \
		 -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel 		 \
		 -Wtype-limits -Wwrite-strings -Werror=vla -Wno-invalid-source-encoding 						 \
		 -Wno-unknown-warning-option

CFLAGS_SANITIZER = -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,$\
				   float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,$\
				   object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,$\
				   undefined,unreachable,vla-bound,vptr

RELEASE_CFLAGS = -O2 -DNDEBUG
DEBUG_CFLAGS   = $(CFLAGS_SANITIZER) -Og -g -D_DEBUG

OPT_FLAGS = $(if $(release),$(RELEASE_CFLAGS),$(DEBUG_CFLAGS))

SRC_DIR = src
BUILD_DIR = build
NON_CODE_DIRS = $(BUILD_DIR) $(DOCS_DIR) .vscode .git
TARGET = main

CD = $(shell pwd)
DOCS_TARGET = $(DOCS_DIR)/docs_generated


NESTED_CODE_DIRS_CD = $(shell find ./$(SRC_DIR) -maxdepth 5 -type d $(NON_CODE_DIRS:%=! -path "*%*"))
NESTED_CODE_DIRS = $(NESTED_CODE_DIRS_CD:.%=%)

FILES_FULL = $(shell find ./$(SRC_DIR) -name "*.cpp")
FILES = $(FILES_FULL:.%=%)

MAKE_DIRS = $(NESTED_CODE_DIRS:%=$(BUILD_DIR)%)
OBJ = $(FILES:%=$(BUILD_DIR)%)
DEPENDS = $(OBJ:%.cpp=%.d)
OBJECTS = $(OBJ:%.cpp=%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(CC) $(OPT_FLAGS) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	@mkdir ./$@

$(MAKE_DIRS): | $(BUILD_DIR)
	@mkdir ./$@

-include $(DEPENDS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR) $(MAKE_DIRS)
	@$(CC) $(OPT_FLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	@rm -rf ./$(BUILD_DIR)/*
	@rm -rf ./$(TARGET)
