#pragma once

#include <stdint.h>

#include "mem/efi_memory.h"

typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} __attribute__((packed)) psf2_header_t;

typedef struct {
	psf2_header_t* psf2_header;
	uint8_t* glyphs;
} __attribute__((packed)) psf2_font_t;

typedef struct {
    efi_memory_descriptor_t* map;
    uint64_t map_size;
    uint64_t key;
    uint64_t desc_size;
    uint32_t desc_ver;
} __attribute__((packed)) bamboo_mmap_t;

typedef struct {
    void* base_address;
    uint64_t buffer_size;
    uint32_t width;     // Horizontal resolution
    uint32_t height;    // Vertical resolution
    uint32_t pixels_per_scanline;
} __attribute__((packed)) bamboo_gop_t;

typedef struct {
    bamboo_gop_t* framebuffer;
    psf2_font_t* font;
    void* rsdp;
    bamboo_mmap_t* mmap;
    uint64_t kernel_phys_end;
} __attribute__((packed)) bamboo_boot_info_t;