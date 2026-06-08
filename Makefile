BUILD_TYPE ?= debug
BUILD_DIR  := build/$(BUILD_TYPE)

CXX      := g++
MOC      := moc
RCC      := rcc
WARNINGS := -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors
CXXFLAGS := -std=c++23 -MMD -MP $(WARNINGS)
PREFIX ?= /usr/local

# Apply specific flags based on build type
ifeq ($(BUILD_TYPE),release)
    CXXFLAGS += -O3 -DNDEBUG
else
    CXXFLAGS += -U_FORTIFY_SOURCE -Og -g
endif

# Dependencies
QT_MODULES   := Qt6Core Qt6Widgets Qt6Svg Qt6SvgWidgets
QT_MOC_FLAGS := $(shell pkg-config --cflags $(QT_MODULES))
QT_CXXFLAGS  := $(patsubst -I%,-isystem %,$(QT_MOC_FLAGS))
QT_LDFLAGS   := $(shell pkg-config --libs $(QT_MODULES))

BLE_MODULES  := sdbus-c++ bluez
BLE_CXXFLAGS := $(patsubst -I%,-isystem %,$(shell pkg-config --cflags $(BLE_MODULES)))
BLE_LDFLAGS  := $(shell pkg-config --libs $(BLE_MODULES))

CXXFLAGS     += $(QT_CXXFLAGS) $(BLE_CXXFLAGS)
LDFLAGS      += $(QT_LDFLAGS) $(BLE_LDFLAGS)

# Shared library
LIB_DIR      := src/lib
LIB_BUILD    := $(BUILD_DIR)/lib
LIB_TARGET   := $(BUILD_DIR)/libcdm.so

LIB_SRC      := $(shell find $(LIB_DIR) -name "*.cpp" -type f)
LIB_OBJ      := $(LIB_SRC:$(LIB_DIR)/%.cpp=$(LIB_BUILD)/%.o)
LIB_DEP      := $(LIB_OBJ:.o=.d)

# Application
APP_DIR      := src/cdm
APP_BUILD    := $(BUILD_DIR)/cdm
APP_TARGET   := cdm

APP_SRC      := $(shell find $(APP_DIR) -name "*.cpp" -type f)
APP_OBJ      := $(APP_SRC:$(APP_DIR)/%.cpp=$(APP_BUILD)/%.o)
APP_DEP      := $(APP_OBJ:.o=.d)
APP_MOC_HDR  := $(shell find $(APP_DIR) -name "*.hpp" -type f -exec grep -l "Q_OBJECT" {} + 2>/dev/null)
APP_MOC_SRC  := $(APP_MOC_HDR:$(APP_DIR)/%.hpp=$(APP_BUILD)/moc_%.cpp)
APP_MOC_OBJ  := $(APP_MOC_SRC:.cpp=.o)

APP_QRC      := $(shell find $(APP_DIR) -name "*.qrc" -type f)
APP_RCC_SRC  := $(APP_QRC:$(APP_DIR)/%.qrc=$(APP_BUILD)/qrc_%.cpp)
APP_RCC_OBJ  := $(APP_RCC_SRC:.cpp=.o)

# Build Targets
.PHONY: app clean debug release lib install

app: $(APP_TARGET)

debug:
	@$(MAKE) BUILD_TYPE=debug app

release:
	@$(MAKE) BUILD_TYPE=release app

lib: $(LIB_TARGET)



#$(APP_TARGET): $(APP_OBJ) $(APP_MOC_OBJ) $(APP_RCC_OBJ) $(LIB_TARGET)
#	$(CXX) $(CXXFLAGS) $(APP_OBJ) $(APP_MOC_OBJ) $(APP_RCC_OBJ) -o $@ -L$(BUILD_DIR) -lcdm $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/$(BUILD_DIR)'
$(APP_TARGET): $(APP_OBJ) $(APP_MOC_OBJ) $(APP_RCC_OBJ) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) $(APP_OBJ) $(APP_MOC_OBJ) $(APP_RCC_OBJ) -o $@ -L$(BUILD_DIR) -lcdm $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/$(BUILD_DIR):$$ORIGIN/../lib'

$(APP_BUILD)/%.o: $(APP_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_BUILD)/moc_%.o: $(APP_BUILD)/moc_%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_BUILD)/moc_%.cpp: $(APP_DIR)/%.hpp
	@mkdir -p $(dir $@)
	$(MOC) $(QT_MOC_FLAGS) $< -o $@

$(APP_BUILD)/qrc_%.o: $(APP_BUILD)/qrc_%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP_BUILD)/qrc_%.cpp: $(APP_DIR)/%.qrc
	@mkdir -p $(dir $@)
	$(RCC) $< -o $@

$(LIB_TARGET): $(LIB_OBJ)
	$(CXX) -shared $^ -o $@ $(LDFLAGS)

$(LIB_BUILD)/%.o: $(LIB_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

install: app lib
	install -d $(PREFIX)/bin $(PREFIX)/lib
	install -m 755 $(APP_TARGET) $(PREFIX)/bin/
	install -m 755 $(LIB_TARGET) $(PREFIX)/lib/

clean:
	rm -r build $(APP_TARGET)

-include $(LIB_DEP) $(APP_DEP)
