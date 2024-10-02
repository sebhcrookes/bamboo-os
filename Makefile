IMAGINGDIR=imaging
BUILDDIR=build
BOOTLOADERDIR=bootloader
KERNELDIR=kernel

VM_MEMORY=4G
VM_CORES=8

.SILENT: compile
compile: 
	echo - Compiling BambooOS imaging software
	make -s -C $(IMAGINGDIR)/
	cp $(IMAGINGDIR)/bin/bamboo-image $(BUILDDIR)/bamboo-image
	chmod +x $(BUILDDIR)/bamboo-image

	echo - Compiling bootloader
	make -s -C $(BOOTLOADERDIR)/
	cp $(BOOTLOADERDIR)/BOOTX64.EFI $(BUILDDIR)/esp/EFI/BOOT/BOOTX64.EFI

	echo - Compiling kernel
	make -s -C $(KERNELDIR)/
	cp $(KERNELDIR)/bin/kernel.elf $(BUILDDIR)/esp/kernel.elf

	echo - Building image
	make -s -C $(BUILDDIR)/

	echo - Successfully compiled the project [✓]

run:
	qemu-system-x86_64 -bios $(BUILDDIR)/bin/bios64.bin -machine q35 -net none -drive file=$(BUILDDIR)/bin/out.img,format=raw -m $(VM_MEMORY) -smp $(VM_CORES) -monitor stdio
.SILENT: clean
clean:
	echo - BambooOS imaging software...
	make -s -C $(IMAGINGDIR)/ clean
	echo - Bootloader...
	make -s -C $(BOOTLOADERDIR)/ clean
	echo - Kernel...
	make -s -C $(KERNELDIR)/ clean
	echo - Build system...
	make -s -C $(BUILDDIR)/ clean

	echo Successfully cleaned the project! [✓]