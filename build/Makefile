IMAGENAME=out.img
BINDIR=bin

ESPDIR=esp
MAINFSDIR=main

compile:
	./bamboo-image -o $(BINDIR)/$(IMAGENAME) -e $(ESPDIR) -a $(MAINFSDIR) > /dev/null

clean:
	rm -f bamboo-image
	rm -f $(BINDIR)/$(IMAGENAME)