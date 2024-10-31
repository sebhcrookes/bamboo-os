OVMF = /usr/share/qemu/bios-TianoCoreEFI.bin

TARGET = BOOTX64.EFI
 
include uefi/Makefile

clean:
	rm -f *.o
	rm -f uefi/*.o

	rm -f $(TARGET)