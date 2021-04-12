BUILD     := build
SRCDIR    := src
INCDIR    := inc
EXTDIR    := ext
LIBDIR    := $(EXTDIR)/lib
LIBINCDIR := $(EXTDIR)/inc
DEPDIR    := .d

RM      := rm -rf
MKDIR   := mkdir -p
TARGET  := $(BUILD)/main
SRCSALL := $(patsubst ./%, %, $(shell find -path ./$(EXTDIR) -prune -o -path ./_old -prune -o -name "*.cc" -o -name "*.h"))
SRCSCC  := $(filter %.cc, $(SRCSALL))
SRCH    := $(filter %.h, $(SRCSALL))
OBJS    := $(patsubst $(SRCDIR)/%.cc, $(BUILD)/%.o, $(SRCSCC))
DEPS    := $(patsubst $(SRCDIR)/%.cc, $(DEPDIR)/%.d, $(SRCSCC))


CXXFLAGS     := -std=c++2a -Wall -Wextra -Wpedantic -ggdb -fno-inline-small-functions -O0 `sdl2-config --cflags`
CXXFLAGS     += -I$(INCDIR) -I$(LIBINCDIR)

DEPFLAGS     += -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

CXXFLAGSTAGS := -I/home/morion/.vim/tags

SDL_GUI_LIB     := $(LIBDIR)/libSDL_GUI.a
SDL_GUI_INCDIR  := $(LIBINCDIR)/SDL_GUI
SDL_GUI_EXTDIR  := $(EXTDIR)/SDL_GUI
SDL_GUI_HEADERS := $(SDL_GUI_EXTDIR)/inc

SDL2_GFX_LIB    := $(LIBDIR)/libSDL2_gfx.a

LIBRARIES   := $(SDL_GUI_LIB) $(SDL2_GFX_LIB)
LIB_HEADERS := $(SDL_GUI_INCDIR)
DYN_LIBS    := -lSDL2 -lSDL2_ttf -lSDL2_image -lfontconfig

# create directories
$(foreach dirname,$(dir $(OBJS)) $(dir $(DEPS)),$(shell $(MKDIR) $(dirname)))

.PHONY: all
#all: CXXFLAGS += -fsanitize=address
#all: LIBS += -fsanitize=address
all: $(TARGET)

.PHONY: libheaders
libheaders: $(LIB_HEADERS)

.PHONY: run
run: all
	@$(TARGET)

.PHONY: clean
clean:
	$(RM) $(BUILD)
	$(RM) $(DEPDIR)
	$(RM) $(LIBDIR)
	$(RM) $(LIBINCDIR)

.PHONY: sure
sure: clean
	@$(MAKE) --no-print-directory

.PHONY: debug
debug:
	@echo $(OBJS)

tags: $(SRCSCC)
	$(CXX) $(CXXFLAGSTAGS) $(CXXFLAGS) -M $(SRCSCC) | sed -e 's/[\\ ]/\n/g' | \
	sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d' | \
	ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q -o "tags" --language-force=C++

$(OBJS): $(BUILD)/%.o: $(SRCDIR)/%.cc $(LIB_HEADERS) $(DEPDIR)/%.d | $(DEPDIR)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c -o $@ $<

%/:
	$(MKDIR) $@

$(DEPS): $(DEPDIR)/

$(TARGET): $(OBJS) $(LIBRARIES)
	$(CXX) -o $@ $^ $(DYN_LIBS)

$(SDL_GUI_LIB): $(SDL_GUI_EXTDIR)
	$(MAKE) -C $< lib
	ln -fs $(CURDIR)/$</build/libSDL_GUI.a $@

$(SDL2_GFX_LIB): | $(SDL_GUI_LIB)
	ln -fs $(CURDIR)/$(SDL_GUI_EXTDIR)/build/libSDL2_gfx.a $@

$(SDL_GUI_INCDIR): $(SDL_GUI_HEADERS)

$(LIBRARIES): | $(LIBDIR)/

.SECONDEXPANSION:

$(LIB_HEADERS): | $(LIBINCDIR)/ $$(@D)/
	ln -fs $(CURDIR)/$< $@

-include $(wildcard $(DEPS))
