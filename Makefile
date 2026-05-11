CXX := g++
CXXFLAGS := -g -std=c++23 -Wall -Wextra
MOC := moc

QT_MODULES := Qt6Core Qt6Widgets Qt6Bluetooth
QT_CXXFLAGS := $(shell pkg-config --cflags $(QT_MODULES))
QT_LDFLAGS  := $(shell pkg-config --libs $(QT_MODULES))

BLE_MODULES := sdbus-c++ bluez
BLE_CXXFLAGS := $(shell pkg-config --cflags $(BLE_MODULES))
BLE_LDFLAGS  := $(shell pkg-config --libs $(BLE_MODULES))

CXXFLAGS += $(QT_CXXFLAGS) $(BLE_CXXFLAGS)
LDFLAGS += $(QT_LDFLAGS) $(BLE_LDFLAGS)

LIB_SRC := $(shell find src/lib -name "*.cpp" -type f)
LIB_OBJ := $(LIB_SRC:src/lib/%.cpp=build/lib/%.o)
LIB_MOC_HDR := $(shell find src/lib -name "*.hpp" -type f -exec grep -l "Q_OBJECT" {} + 2>/dev/null)
LIB_MOC_SRC := $(LIB_MOC_HDR:src/lib/%.hpp=build/lib/moc_%.cpp)
LIB_MOC_OBJ := $(LIB_MOC_SRC:.cpp=.o)
LIB_TARGET := build/libcdm.so

APP_SRC := $(shell find src/cdm -name "*.cpp" -type f)
APP_OBJ := $(APP_SRC:src/cdm/%.cpp=build/cdm/%.o)
APP_MOC_HDR := $(shell find src/cdm -name "*.hpp" -type f -exec grep -l "Q_OBJECT" {} + 2>/dev/null)
APP_MOC_SRC := $(APP_MOC_HDR:src/cdm/%.hpp=build/cdm/moc_%.cpp)
APP_MOC_OBJ := $(APP_MOC_SRC:.cpp=.o)
APP_TARGET := cdm

.PHONY: app lib clean

app $(APP_TARGET): $(APP_OBJ) $(APP_MOC_OBJ) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) $(APP_OBJ) $(APP_MOC_OBJ) -o $(APP_TARGET) -Lbuild -lcdm $(LDFLAGS) -Wl,-rpath,'$$ORIGIN/build'

lib $(LIB_TARGET): $(LIB_OBJ) $(LIB_MOC_OBJ)
	$(CXX) -shared $^ -o $(LIB_TARGET) $(LDFLAGS)

build/lib/%.o: src/lib/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

build/cdm/%.o: src/cdm/%.cpp $(APP_MOC_SRC)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/cdm/moc_%.o: build/cdm/moc_%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/cdm/moc_%.cpp: src/cdm/%.hpp
	mkdir -p $(dir $@)
	$(MOC) $(QT_CXXFLAGS) $< -o $@

clean:
	rm -r build $(APP_TARGET)
