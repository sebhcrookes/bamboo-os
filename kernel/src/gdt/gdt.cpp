#include "gdt.h"

#include <string.h>

#include "../mem/memory.h"

gdt_ptr_t* g_gdtr;	// Final GDTR (in 2nd available page of memory)

#define GRAN_4K 	1 << 7
#define SZ_32       1 << 6
#define LONG_MODE   1 << 5

/* OSDev.org states that base and limit values are ignored in long mode
   Despite this, we still set the limit to its maximum value (0xFFFFF) */
   
__attribute__((aligned(0x1000)))
gdt_t gdt {
 	{{0, 0, 0, 0x00, 0x00, 0},
	{0xFFFF, 0, 0, 0x9A, 0xF | GRAN_4K | LONG_MODE, 0},
	{0xFFFF, 0, 0, 0x92, 0xF | GRAN_4K | SZ_32, 0},
	{0, 0, 0, 0x00, 0x00, 0},
	{0xFFFF, 0, 0, 0x9A, 0xF | GRAN_4K | LONG_MODE, 0},
	{0xFFFF, 0, 0, 0x92, 0xF | GRAN_4K | SZ_32, 0}}
};

GDT::GDT() {

}

GDT::~GDT() {}

void GDT::install() {
	copy_low();
	_load_gdt(g_gdtr);
}

/* Copies the GDT to the second available page (it can then be accessed by the APs) */
void GDT::copy_low() {
	uint64_t third_page = (uint64_t) memory::upn_to_address(1);
	memory::get_vmm()->map_memory((void*) third_page, (void*) third_page);

	/* Copies the GDT to the third page, with the GDTR just after */
	memcpy((void*) third_page, (void*) &gdt, sizeof(gdt_t));
	gdt_ptr_t* gdtr = (gdt_ptr_t*)(third_page + sizeof(gdt_t));
	gdtr->base = third_page;
	gdtr->limit = (sizeof(struct gdt_t)) - 1;

	m_gdt = (gdt_t*)((void*) third_page);
	g_gdtr = gdtr;
}