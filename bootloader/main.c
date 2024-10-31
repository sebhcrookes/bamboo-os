/* WARNING: This code is cobbled together and is NOT good at all. I plan on rewriting this soon. */

#include <elf.h>
#include <uefi.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "structs.h"
#include "vmm.h"

#include "efi-helpers.h"

#define KERNEL_START 0xFFFF800000000000

void abort_boot(char* reason) {
	printf("Boot aborted because: %s\n", reason);
	while(true) {}
}

bamboo_mmap_t* get_memory_map() {
	bamboo_mmap_t* mmap = (bamboo_mmap_t*) malloc(sizeof(bamboo_mmap_t));

	/* Getting the memory map must be the last thing we do, as allocating more memory after this would
	change the memory map, therefore we would be giving our kernel the wrong information */

	efi_memory_descriptor_t* memory_map;
	uint64_t memory_map_size;
	uint64_t map_key;
	uint64_t descriptor_size;
	uint32_t descriptor_version;

	// Get the EFI memory map
	memory_map_size  = 0;
	memory_map      = NULL;
	efi_status_t status = BS->GetMemoryMap (
					&memory_map_size,
					memory_map,
					&map_key,
					&descriptor_size,
					&descriptor_version
					);
	memory_map_size += 2 * descriptor_size;
	
	// Use size returned for the AllocatePool
	memory_map = (efi_memory_descriptor_t*) malloc(memory_map_size);
	if(memory_map == NULL) {
		abort_boot("Could not allocate memory for the memory map");
	}
	status = BS->GetMemoryMap (
					&memory_map_size,
					memory_map,
					&map_key,
					&descriptor_size,
					&descriptor_version
					);

	if (EFI_ERROR(status)) {
		free(memory_map);
	}
	
	// Copy EFI memory map info into bamboo memory map
	mmap->map = memory_map;
	mmap->map_size = memory_map_size;
	mmap->key = map_key;
	mmap->desc_size = descriptor_size;
	mmap->desc_ver = descriptor_version;

	return mmap;
}

psf2_font_t* load_psf2_font(char* path) {
	psf2_header_t* header = (psf2_header_t*) malloc(sizeof(psf2_header_t));
	uint8_t* glyphs;

	// Open the PSF2 file
	FILE* font = fopen(path, "r");
	if (font == NULL) {
		printf("[PSF2]: Failed to load font %s\n", path);
		return NULL;
	}

	size_t hdr_size = sizeof(psf2_header_t);

	// Read in the header and make sure the magic value is correct
	fread(header, hdr_size, 1, font);
	if (header->magic != PSF2_FONT_MAGIC) {
		printf("[PSF2]: Magic not PSF2 for font %s\n", path);
		return NULL;
	}

	// Allocate space for and read in the glyphs
	glyphs = (uint8_t*) malloc(header->numglyph * header->bytesperglyph);
	fread(glyphs, header->bytesperglyph * header->numglyph, 1, font);

	// Assemble the final, finished struct
	psf2_font_t* finished = (psf2_font_t*) malloc(sizeof(psf2_font_t));

	finished->psf2_header = header;
	finished->glyphs = glyphs;

	return finished;
}

void* get_rsdp() {
	uint64_t ct_size = ST->NumberOfTableEntries;

	for(uint64_t i = 0; i < ct_size; i++) {
		efi_configuration_table_t current = ST->ConfigurationTable[i];

		if(guid_equal(current.VendorGuid, (efi_guid_t) ACPI_20_TABLE_GUID)) {
			return current.VendorTable;
		}
	}

	/* According to ACPI spec, if we haven't found a V2.0, we need to look for a V1.0 */

	for(uint64_t i = 0; i < ct_size; i++) {
		efi_configuration_table_t current = ST->ConfigurationTable[i];

		if(guid_equal(current.VendorGuid, (efi_guid_t) ACPI_TABLE_GUID)) {
			return current.VendorTable;
		}
	}

	return NULL;
}

int main(int argc, char** argv) {

	// Disabling the watchdog
	BS->SetWatchdogTimer(0, 0, 0, NULL);

	efi_status_t status;

	/* ========= Menu code ========= */
	efi_input_key_t key;
	bool selected = false;
	uint8_t option = 0;

	FILE* boot_options = fopen("options.boot", "r");

	if(boot_options == NULL) {

		while(!selected) {		
			ST->ConOut->ClearScreen(ST->ConOut);

			printf("========= BambooOS - First use boot options =========\n");
			printf("(these can be changed later in the operating system)\n\n");

			printf("Use up and down arrows to select, and right to proceed\n\n");

			switch(key.ScanCode) {
				case SCAN_UP: {
					if(option > 0) option--;
				} break;
				case SCAN_DOWN: {
					if(option < 1) option++;
				} break;
				case SCAN_RIGHT: {
					selected = true;
				} break;
			}

			for(int i = 0; i < 2; i++) {
				if(option == i) {
					printf("> ");
				} else {
					printf("  ");
				}

				if(i == 0) {
					printf("Boot without GUI (terminal only)\n");
				} else if (i == 1) {
					printf("Boot with GUI\n");
				}
			}
			while(ST->ConIn->ReadKeyStroke(ST->ConIn, &key) != EFI_SUCCESS && !selected) {}
		}

		ST->ConOut->ClearScreen(ST->ConOut);

		/* Write the options file */
		boot_options = fopen("options.boot", "w+");
		fclose(boot_options);
	}

	/* ========= GOP ========= */

	/* Detecting GOP */

	efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  	efi_gop_t* gop;

	status = BS->LocateProtocol(&gopGuid, NULL, (void**) &gop);
	if(EFI_ERROR(status))
		abort_boot("Unable to locate GOP");

	/* Getting GOP mode */

	efi_gop_mode_info_t* info;
	uint64_t SizeOfInfo, numModes, nativeMode;
	
	status = gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &SizeOfInfo, &info);
	// this is needed to get the current video mode
	if (status == EFI_NOT_STARTED)
		status = gop->SetMode(gop, 0);
	if(EFI_ERROR(status)) {
		abort_boot("Unable to get native GOP mode");
	} else {
		nativeMode = gop->Mode->Mode;
		numModes = gop->Mode->MaxMode;
	}

	/* Setting mode + getting framebuffer */

	status = gop->SetMode(gop, nativeMode);
	if(EFI_ERROR(status)) {
		abort_boot("Unable to set GOP mode");
	}

	bamboo_gop_t final_gop = {
		.base_address = (void*) gop->Mode->FrameBufferBase,
		.buffer_size = gop->Mode->FrameBufferSize,
		.width = gop->Mode->Information->HorizontalResolution,
		.height = gop->Mode->Information->VerticalResolution,
		.pixels_per_scanline = gop->Mode->Information->PixelsPerScanLine,
	};

	/* ========= Bootloader logo ========= */

	printf(" ____                        _                        ____     _____ \n");
	printf("|  _ \\                      | |                      / __ \\   / ____|\n");
	printf("| |_) |   __ _   _ __ ___   | |__     ___     ___   | |  | | | (___  \n");
	printf("|  _ <   / _` | | '_ ` _ \\  | '_ \\   / _ \\   / _ \\  | |  | |  \\___ \\ \n");
	printf("| |_) | | (_| | | | | | | | | |_) | | (_) | | (_) | | |__| |  ____) |\n");
	printf("|____/   \\__,_| |_| |_| |_| |_.__/   \\___/   \\___/   \\____/  |_____/");

	printf("  Bootloader\n\n");
	
	/* ========= Calculating Memory Size ========= */

	bamboo_mmap_t* temp = get_memory_map();

	uint64_t memory_size_pages = 0;
    efi_memory_descriptor_t* ent = temp->map;
	
	do {
		memory_size_pages += ent->NumberOfPages;
		ent = (efi_memory_descriptor_t*)(((uint8_t*) ent) + temp->desc_size);
	} while((uint8_t*) ent < (uint8_t*) temp->map + temp->map_size);
	
	/* ========= Identity Mapping ========= */

	page_table_t* pml4;
	BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (efi_physical_address_t*) &pml4);
    memset(pml4, 0, 0x1000);

	ent = temp->map;
        
	do {
		for(int i = 0; i < ent->NumberOfPages * 0x1000; i += 0x1000) {
			map_memory(pml4, (void*)(ent->PhysicalStart + i), (void*)(ent->PhysicalStart + i));
		}
		ent = (efi_memory_descriptor_t*)(((uint8_t*) ent) + temp->desc_size);
	} while((uint8_t*) ent < (uint8_t*) temp->map + temp->map_size);

	uint64_t framebuffer_base = (uint64_t) final_gop.base_address;
	uint64_t framebuffer_size = (uint64_t) final_gop.buffer_size + 0x1000;
	for(uint64_t i = framebuffer_base; i < framebuffer_base + framebuffer_size; i += 0x1000) {
		map_memory(pml4, (void*) i, (void*) i);
	}

	// Remove temporary memory map (it will have changed when we get to actually calling the kernel)
	free(temp->map);
	free(temp);

	/* ========= Loading Kernel ========= */

	char* buff;
	FILE* kernel_file = fopen("kernel.elf", "r");
	size_t size;

	Elf64_Ehdr* elf;
    Elf64_Phdr* phdr;
    void* kernel_entry;

	if(kernel_file) {
		fseek(kernel_file, 0, SEEK_END);
        size = ftell(kernel_file);
        fseek(kernel_file, 0, SEEK_SET);

        buff = malloc(size + 1);
		memset((void*) buff, size + 5, 0);
        fread(buff, size, 1, kernel_file);

        fclose(kernel_file);
	}

	uint64_t kernel_phys_end = 0;

	/* is it a valid ELF executable for this architecture? */
    elf = (Elf64_Ehdr*)buff;
    if(!memcmp(elf->e_ident, ELFMAG, SELFMAG) &&    /* magic match? */
        elf->e_ident[EI_CLASS] == ELFCLASS64 &&     /* 64 bit? */
        elf->e_ident[EI_DATA] == ELFDATA2LSB &&     /* LSB? */
        elf->e_type == ET_EXEC &&                   /* executable object? */
        elf->e_machine == EM_MACH &&                /* architecture match? */
        elf->e_phnum > 0) {                         /* has program headers? */
            /* Load segments */
			int i = 0;
            for(phdr = (Elf64_Phdr*)(buff + elf->e_phoff);
                i < elf->e_phnum;
                i++, phdr = (Elf64_Phdr*)((uint8_t*)phdr + elf->e_phentsize)) {
                    if(phdr->p_type == PT_LOAD) {
                        memcpy((void*)phdr->p_vaddr, buff + phdr->p_offset, phdr->p_filesz);

						/* Map the segment in the higher half */

						for(int i = 0; i < phdr->p_filesz + 0x1000; i += 0x1000) {
							map_memory(pml4, (void*)(phdr->p_vaddr + i), (void*)(phdr->p_vaddr + i + KERNEL_START));
							if(phdr->p_vaddr + i > kernel_phys_end) {
								kernel_phys_end = phdr->p_vaddr + i;
							}
						}

                        memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
                    }
                }
            kernel_entry = elf->e_entry;
			printf("Loaded kernel into memory, with entry address 0x%X\n", kernel_entry);
    } else {
		abort_boot("Kernel not a valid ELF executable for this architecture");
    }

   free(buff);

	/* ========= PSF2 FONT ========= */

	psf2_font_t* font = load_psf2_font("font.psfu");

	/* ========= RSDP / ACPI ========= */
	
	void* rsdp = get_rsdp();

	if(rsdp != 0) {
		printf("Found RSDP at address 0x%X\n", rsdp);
	}

	/* ========= Memory Map ========= */

	bamboo_mmap_t* mmap = get_memory_map();

	/* ========= Preparing boot info ========= */

	bamboo_boot_info_t boot_info = {
		.framebuffer = &final_gop,
		.font = font,
		.rsdp = rsdp,
		.mmap = mmap,
		.kernel_phys_end = kernel_phys_end,
	};

	exit_bs(mmap->key);

	// Load the PML4
	asm volatile ("mov %0, %%cr3" : : "r" (((void*) pml4)));

	// Call the kernel entry function
    int r = (*((int(* __attribute__((sysv_abi)))(bamboo_boot_info_t*))(kernel_entry + KERNEL_START)))(&boot_info);

	return EFI_SUCCESS; // Exit the UEFI application
}
