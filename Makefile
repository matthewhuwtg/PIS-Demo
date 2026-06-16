# Makefile for Linux-development — PIS Demo System
# Targets: all, run, test, debug, clean, check-deps

CXX       := g++
CXXFLAGS  := -std=c++23 -Wall -Wextra -O2 -D_GNU_SOURCE
LDFLAGS   :=
INCLUDES  := -I. -Iinclude
SRCDIR    := src
BUILDDIR  := build
TARGET    := bin/main

# ---- Library detection via pkg-config ----
DBUS_CFLAGS   := $(shell pkg-config --cflags dbus-1 2>/dev/null || echo "")
DBUS_LIBS     := $(shell pkg-config --libs dbus-1 2>/dev/null || echo "")
GST_CFLAGS    := $(shell pkg-config --cflags gstreamer-1.0 2>/dev/null || echo "")
GST_LIBS      := $(shell pkg-config --libs gstreamer-1.0 2>/dev/null || echo "")
QT5_CFLAGS    := $(shell pkg-config --cflags Qt5Widgets 2>/dev/null || echo "")
QT5_LIBS      := $(shell pkg-config --libs Qt5Widgets 2>/dev/null || echo "")

ifneq ($(DBUS_LIBS),)
    CXXFLAGS += -DHAS_DBUS $(DBUS_CFLAGS)
    LDFLAGS  += $(DBUS_LIBS)
endif
ifneq ($(GST_LIBS),)
    CXXFLAGS += -DHAS_GSTREAMER $(GST_CFLAGS)
    LDFLAGS  += $(GST_LIBS)
endif

# ---- Test target ----
TEST_TARGET := bin/test_schedule
TEST_SRCS   := tests/test_schedule.cpp src/pis_schedule.cpp src/pis_config.cpp
TEST_OBJS   := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(TEST_SRCS))

# ---- Source files ----
PIS_SRCS := main.cpp \
            $(SRCDIR)/utils.cpp \
            $(SRCDIR)/pis_config.cpp \
            $(SRCDIR)/pis_schedule.cpp \
            $(SRCDIR)/pis_display.cpp \
            $(SRCDIR)/pis_ipc.cpp \
            $(SRCDIR)/pis_dbus.cpp \
            $(SRCDIR)/pis_core.cpp

PIS_OBJS := $(patsubst %.cpp, $(BUILDDIR)/%.o, $(PIS_SRCS))

# ---- Default target ----
all: check-deps $(TARGET)

# ---- Link PIS executable ----
$(TARGET): $(PIS_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo ">>> Build complete: $@"

# ---- Compile ----
$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ---- Run ----
run: $(TARGET)
	@echo ">>> Starting PIS Demo..."
	./$(TARGET)

# ---- Test ----
test: $(TEST_TARGET)
	@echo ">>> Running PIS unit tests..."
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo ">>> Test build complete: $@"

# ---- Check dependencies ----
check-deps:
	@echo ">>> Checking build dependencies..."
	@command -v $(CXX) >/dev/null 2>&1 || { echo "ERROR: $(CXX) not found"; exit 1; }
	@echo "  [OK] $(CXX)"
ifneq ($(DBUS_LIBS),)
	@echo "  [OK] D-Bus: $(DBUS_LIBS)"
else
	@echo "  [--] D-Bus: not found (using mock)"
endif
ifneq ($(GST_LIBS),)
	@echo "  [OK] GStreamer: $(GST_LIBS)"
else
	@echo "  [--] GStreamer: not found (using mock)"
endif
ifneq ($(QT5_LIBS),)
	@echo "  [OK] Qt5Widgets: $(QT5_LIBS)"
else
	@echo "  [--] Qt5Widgets: not found (QT GUI disabled)"
endif
	@echo ">>> All checks passed."

# ---- Clean ----
clean:
	rm -rf $(BUILDDIR) bin/main bin/test_schedule
	@echo ">>> Clean complete"

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

.PHONY: all run test clean debug check-deps
