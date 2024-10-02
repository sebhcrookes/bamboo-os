CPPFLAGS=-g -pthread
LDFLAGS=-g
LDLIBS=

OBJDIR=obj
SRCDIR=src
BINDIR=bin

names += $(patsubst %.c,%.o,$(shell find $(SRCDIR) -iname "*.c"))

names := $(shell realpath --relative-to src $(names))

names := $(addprefix $(OBJDIR)/, $(names))

$(BINDIR)/bamboo-image: $(names)
	gcc $(CPPFLAGS) -o $(BINDIR)/bamboo-image $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*
	rm -f $(BINDIR)/bamboo-image
