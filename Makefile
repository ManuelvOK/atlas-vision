BUILD  := build
SRCDIR := src
INCDIR := inc
LIBDIR := lib
DEPDIR := .d

RM     := rm -rf
MKDIR  := mkdir -p
TARGET := $(BUILD)/main
SRCSALL      := $(patsubst ./%, %, $(shell find -path ./$(LIBDIR) -prune -o -path ./_old -prune -o -name "*.cc" -o -name "*.h"))
SRCSCC       := $(filter %.cc, $(SRCSALL))
SRCH         := $(filter %.h, $(SRCSALL))
OBJS         := $(patsubst $(SRCDIR)/%.cc, $(BUILD)/%.o, $(SRCSCC))
DEPS         := $(patsubst $(SRCDIR)/%.cc, $(DEPDIR)/%.d, $(SRCSCC))


CXXFLAGS     := -std=c++2a -Wall -Wextra -Wpedantic -ggdb -O0 `sdl2-config --cflags`
CXXFLAGS     += -I$(INCDIR) -I$(LIBDIR)

CXXFLAGSTAGS := -I/home/morion/.vim/tags

LIBS         := -lSDL2 -lSDL2_gfx -lSDL2_ttf -lSDL2_image
LIBRARIES    := $(LIBDIR)/SDL_GUI.a

# create directories
$(foreach dirname,$(dir $(OBJS)) $(dir $(DEPS)),$(shell $(MKDIR) $(dirname)))

.PHONY: all
#all: CXXFLAGS += -fsanitize=address
#all: LIBS += -fsanitize=address
all: $(TARGET)

.PHONY: run
run: all
	@$(TARGET)

.PHONY: clean
clean:
	$(RM) $(BUILD)
	$(RM) $(DEPDIR)

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

$(OBJS): $(BUILD)/%.o: $(DEPDIR)/%.d
$(OBJS): $(BUILD)/%.o: $(SRCDIR)/%.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(DEPS): $(DEPDIR)/%.d: $(SRCDIR)/%.cc
	$(CXX) $(CXXFLAGS) -MM -MT $(BUILD)/$*.o -o $@ $<

$(TARGET): $(OBJS) $(LIBRARIES)
	$(CXX) -o $@ $^ $(LIBS)

$(LIBRARIES): $(LIBDIR)/%.a: $(LIBDIR)/%/
	$(MAKE) -C $(LIBDIR)/$* lib
	ln -fs $(CURDIR)/$(LIBDIR)/$*/build/$*.a $(LIBDIR)/$*.a

-include $(DEPS)




#BUILD = build
#ifneq ($(notdir $(CURDIR)),$(BUILD))
#
#.SUFFIXES:
#OBJDIR := $(CURDIR)/build
#DEPDIR := $(CURDIR)/.d
#INCDIR := $(CURDIR)/inc
#SRCDIR := $(CURDIR)/src
#LIBDIR := $(CURDIR)/lib
#
#RM     := rm -rf
#MKDIR  := mkdir -p
#
#SRCSALL      := $(shell find $(CURDIR) -name "*.cc" -o -name "*.h")
#SRCSCCABS    := $(filter %.cc, $(SRCSALL))
#SRCSCC       := $(patsubst $(SRCDIR)/%,%,$(SRCSCCABS))
#
## create directories
#$(foreach dirname,$(dir $(SRCSCC)),$(shell $(MKDIR) $(DEPDIR)/$(dirname)))
#$(foreach dirname,$(dir $(SRCSCC)),$(shell $(MKDIR) $(OBJDIR)/$(dirname)))
#
#.PHONY: $(all)
#all: lib
#	+@$(MAKE) --no-print-directory -C $(OBJDIR) -f $(CURDIR)/Makefile \
#	 SRCDIR=$(SRCDIR) INCDIR=$(INCDIR) DEPDIR=$(DEPDIR) ROOTDIR=$(CURDIR) \
#	 LIBDIR=$(LIBDIR) \
#	 $(MAKECMDGOALS)
#
#.PHONY: lib
#lib:
#	$(MAKE) -C $(LIBDIR)/SDL_GUI lib
#
#Makefile : ;
#
#% :: all ;
#
#.PHONY: clean
#clean:
#	$(RM) $(OBJDIR)
#	$(RM) $(DEPDIR)
#
#.PHONY: superclean
#superclean: clean
#	$(MAKE) -C $(LIBDIR)/SDL_GUI clean
#
#.PHONY: sure
#sure: clean
#	@$(MAKE) --no-print-directory
#
#.PHONY: supersure
#supersure: superclean
#	@$(MAKE) --no-print-directory
#else
#
#TARGET       := visualisation
#
#SRCSALL      := $(shell find $(ROOTDIR) -path $(LIBDIR) -prune -o -path $(INCDIR)/_old -prune -o -path $(SRCDIR)/_old -prune -o -name "*.cc" -o -name "*.h")
#SRCSCCABS    := $(filter %.cc, $(SRCSALL))
#SRCSCC       := $(patsubst $(SRCDIR)/%,%,$(SRCSCCABS))
#SRCHABS      := $(filter %.h, $(SRCSALL))
#SRCSH        := $(patsubst $(INCDIR)/%,%,$(SRCSHABS))
#OBJS         := $(SRCSCC:.cc=.o)
#DEPS         := $(addprefix $(patsubst $(ROOTDIR)/%,%,$(DEPDIR))/,$(SRCSCC:.cc=.d))
#
#CXXFLAGS     := -std=c++2a -Wall -Wextra -Wpedantic -ggdb -O0 `sdl2-config --cflags`
#CXXFLAGS     += -I$(INCDIR) -I$(LIBDIR)
#
#CXXFLAGSTAGS := -I/home/morion/.vim/tags
#
#LIBS         := -lSDL2 -lSDL2_gfx -lSDL2_ttf -lSDL2_image
#
#vpath %.h $(dir $(SRCSHABS))
#vpath %.cc $(dir $(SRCSCCABS))
#vpath %.d $(dir $(addprefix $(DEPDIR)/, $(DEPS)))
#
#.PHONY: all
#all: LIBS += -fsanitize=address
#all: $(TARGET)
#
#.PHONY: tags
#tags: $(ROOTDIR)/tags
#
#.PHONY: sanitized
#sanitized: CXXFLAGS += -fsanitize=address
#sanitized: LIBS += -fsanitize=address
#sanitized: all
#
#.PHONY: effective
#effective: CXXFLAGS += -Weffc++
#effective: all
#
#.PHONY: makefile-debug
#makefile-debug:
#	@echo $(SRCSALL)
#
#$(TARGET): $(OBJS)
#	$(CXX) -o $@ $^ $(LIBDIR)/SDL_GUI/build/SDL_GUI.a $(LIBS)
#
#$(DEPDIR)/%.d: %.cc
#	$(CXX) $(CXXFLAGS) -MM -o $@ $<
#
#$(ROOTDIR)/tags: $(SRCSCC)
#	$(CXX) $(CXXFLAGSTAGS) $(CXXFLAGS) -M $(SRCSCCABS) | sed -e 's/[\\ ]/\n/g' | \
#	sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d' | \
#	ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q -o "$(ROOTDIR)/tags" --language-force=C++
#
#-include $(DEPS)
#
#endif
