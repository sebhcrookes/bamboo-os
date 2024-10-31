CPPFLAGS=-g -pthread
LDFLAGS=-g
LDLIBS=

OBJDIR=obj
SRCDIR=src
BINDIR=bin

names += $(patsubst %.cpp,%.o,$(shell find $(SRCDIR) -iname "*.cpp"))

names := $(shell realpath --relative-to src $(names))

names := $(addprefix $(OBJDIR)/, $(names))

$(BINDIR)/bamboo-image: $(names)
	g++ $(CPPFLAGS) -o $(BINDIR)/bamboo-image $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*
	rm -f $(BINDIR)/bamboo-image
