# tool macros
GLSLCC := glslc
CC := g++
CCFLAGS := -std=c++17 -O2 -Wall
LDFLAGS := -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
DBGFLAGS := -g -DDEBUG
CCOBJFLAGS := $(CCFLAGS) -c

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug
SPV_PATH := $(BIN_PATH)/shaders
DSPV_PATH := $(DBG_PATH)/shaders

# compile macros
TARGET_NAME := main
ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# shader files
SPV_SRC := $(foreach x, $(SRC_PATH)/shaders, $(wildcard $(addprefix $(x)/*,.glsl)))
SPV := $(addprefix $(SPV_PATH)/, $(addsuffix .spv, $(notdir $(basename $(SPV_SRC)))))
DSPV := $(addprefix $(DSPV_PATH)/, $(addsuffix .spv, $(notdir $(basename $(SPV_SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG) \
									$(SPV) \
									$(DSPV)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: makedir all

# non-phony targets
$(TARGET): $(OBJ) $(SPV)
	$(CC) $(CCFLAGS) $(LDFLAGS) -o $@ $(OBJ)

$(SPV_PATH)/%.spv: $(SRC_PATH)/shaders/%.glsl
	$(GLSLCC) -o $@ $<

$(DSPV_PATH)/%.spv: $(SRC_PATH)/shaders/%.glsl
	$(GLSLCC) -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) $(DBGFLAGS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG) $(DSPV)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(DBGFLAGS) $(OBJ_DEBUG) -o $@

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH) $(SPV_PATH) $(DSPV_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)
