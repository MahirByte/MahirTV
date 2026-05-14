CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra \
            $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image glew) \
            -DASSETS_PATH=\"$(CURDIR)/assets/\"
LDFLAGS  := $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image glew) \
            -lGL

TARGET   := MahirTV
SRCDIR   := src
SRCS     := $(SRCDIR)/main.cpp \
            $(SRCDIR)/App.cpp \
            $(SRCDIR)/Shader.cpp \
            $(SRCDIR)/TextRenderer.cpp \
            $(SRCDIR)/screens/LanguageScreen.cpp \
            $(SRCDIR)/screens/AccountScreen.cpp \
            $(SRCDIR)/screens/SaveDataScreen.cpp \
            $(SRCDIR)/screens/LoadingScreen.cpp \
            $(SRCDIR)/screens/HomeScreen.cpp \
            $(SRCDIR)/screens/SettingsScreen.cpp \
            $(SRCDIR)/screens/StoreScreen.cpp

OBJDIR   := build/obj
OBJS     := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

.PHONY: all clean run install

all: $(TARGET)
	@echo ""
	@echo "  Done! Run with: make run"

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

install: all
	install -Dm755 $(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)
	install -dm755 $(DESTDIR)/usr/local/share/mahirtv
	cp -r assets $(DESTDIR)/usr/local/share/mahirtv/

clean:
	rm -rf build/ $(TARGET)
	@echo "Cleaned."
