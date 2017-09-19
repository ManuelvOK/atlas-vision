BUILD = build

ifneq ($(notdir $(CURDIR)),$(BUILD))

.SUFFIXES:

OBJDIR = $(CURDIR)/$(BUILD)
SRCDIR = $(CURDIR)/src
INCDIR = $(CURDIR)/inc
DEPDIR = $(CURDIR)/.d

.PHONY: $(all)
all: $(OBJDIR) $(DEPDIR)
	+@$(MAKE) --no-print-directory -C $< -f $(CURDIR)/Makefile \
	 SRCDIR=$(SRCDIR) INCDIR=$(INCDIR) DEPDIR=$(DEPDIR) $(MAKECMDGOALS)

.PHONY: $(OBJDIR)
$(OBJDIR):
	+@[ -d $@ ] || mkdir -p $@

.PHONY: $(DEPDIR)
$(DEPDIR):
	+@[ -d $@ ] || mkdir -p $@

Makefile : ;

% :: all ;

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
	rm -rf $(DEPDIR)

else

vpath %.cc $(SRCDIR)
vpath %.d $(DEPDIR)
vpath %.h $(INCDIR)

TARGET = visualisation
LIBS   = -lSDL2

SRC	= $(notdir $(wildcard $(SRCDIR)/*.cc))
OBJ = $(SRC:%.cc=%.o)
DEP = $(SRC:%.cc=$(DEPDIR)/%.d)

CXXFLAGS  = -std=c++17 -Wall -Wextra -Wpedantic -Weffc++ -g `sdl2-config --cflags`
CXXFLAGS += -I$(INCDIR)

.PHONY: all
all: export LANG=en_US.utf8
all: $(TARGET)

.PHONY: sanitized
sanitized: CXXFLAGS += -fsanitize=address
sanitized: LIBS += -fsanitize=address
sanitized: all

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LIBS)

$(DEPDIR)/%.d: %.cc
	$(CXX) $(CXXFLAGS) -MM -o $@ $<

-include $(DEP)

endif
