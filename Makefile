BUILD_TYPE ?= debug
BUILD_DIR  := build/$(BUILD_TYPE)

CXX      := g++
MOC      := moc
WARNINGS := -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors
CXXFLAGS := -std=c++23 $(WARNINGS)

# Apply specific flags based on build type
ifeq ($(BUILD_TYPE),release)
    CXXFLAGS += -O3 -DNDEBUG
else
    CXXFLAGS += -U_FORTIFY_SOURCE -Og -g
endif

# Dependencies
QT_MODULES   := Qt6Core Qt6Widgets
QT_MOC_FLAGS := $(shell pkg-config --cflags $(QT_MODULES))
QT_CXXFLAGS  := $(patsubst -I%,-isystem %,$(QT_MOC_FLAGS))
QT_LDFLAGS   := $(shell pkg-config --libs $(QT_MODULES))

BLE_MODULES  := sdbus-c++ bluez
BLE_CXXFLAGS := $(patsubst -I%,-isystem %,$(shell pkg-config --cflags $(BLE_MODULES)))
BLE_LDFLAGS  := $(shell pkg-config --libs $(BLE_MODULES))

CXXFLAGS += $(QT_CXXFLAGS) $(BLE_CXXFLAGS)
LDFLAGS  += $(QT_LDFLAGS) $(BLE_LDFLAGS)

# Shared library
LIB_DIR     := src/lib
LIB_BUILD   := $(BUILD_DIR)/lib
LIB_TARGET  := $(BUILD_DIR)/libcdm.so

LIB_SRC     := $(shell find $(LIB_DIR) -name "*.cpp" -type f)
LIB_OBJ     := $(LIB_SRC:$(LIB_DIR)/%.cpp=$(LIB_BUILD)/%.o)

# Application
APP_DIR     := src/cdm
APP_BUILD   := $(BUILD_DIR)/cdm
APP_TARGET  := cdm

APP_SRC     := $(shell find $(APP_DIR) -name "*.cpp" -type f)
APP_OBJ     := $(APP_SRC:$(APP_DIR)/%.cpp=$(APP_BUILD)/%.o)
APP_MOC_HDR := $(shell find $(APP_DIR) -name "*.hpp" -type f -exec grep -l "Q_OBJECT" {} + 2>/dev/null)
APP_MOC_SRC := $(APP_MOC_HDR:$(APP_DIR)/%.hpp=$(APP_BUILD)/moc_%.cpp)
APP_MOC_OBJ := $(APP_MOC_SRC:.cpp=.o)

# Build Targets
.PHONY: app clean debug release lib

app: $(APP_TARGET)

debug:
	@$(MAKE) BUILD_TYPE=debug app

release:
	@$(MAKE) BUILD_TYPE=release app

lib: $(LIB_TARGET)



$(APP_TARGET): $(APP_OBJ) $(APP_MOC_OBJ) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) $(APP_OBJ) $(APP_MOC_OBJ) -o $@ -L$(BUILD_DIR) -lcdm $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/$(BUILD_DIR)'

$(APP_BUILD)/%.o: $(APP_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_BUILD)/moc_%.o: $(APP_BUILD)/moc_%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_BUILD)/moc_%.cpp: $(APP_DIR)/%.hpp
	@mkdir -p $(dir $@)
	$(MOC) $(QT_MOC_FLAGS) $< -o $@

$(LIB_TARGET): $(LIB_OBJ)
	$(CXX) -shared $^ -o $@ $(LDFLAGS)

$(LIB_BUILD)/%.o: $(LIB_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

clean:
	rm -r build $(APP_TARGET)
