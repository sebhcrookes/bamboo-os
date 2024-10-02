#include "globals.h"

char* image_filename = "";
uint64_t lba_size = 512;
uint64_t esp_size = MEGABYTE * 33;
uint64_t data_size = MEGABYTE * 33;
uint64_t image_size = 0;
uint64_t esp_lba_sz, data_lba_sz, image_lba_sz;
uint64_t gpt_table_lba_sz;
uint64_t esp_lba_st, data_lba_st;
uint64_t align_lba = 0;