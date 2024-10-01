#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "mbr.h"
#include "gpt.h"
#include "globals.h"

#include "fat32.h"

static char* output_filename = "";
static char* esp_foldername = "";
static char* main_foldername = "";

int main(int argc, char *argv[]) {
    int c;
    
    static struct option long_options[] =
        {
        {"output",     required_argument,       0, 'o'},
        {"add-esp",       required_argument,       0, 'e'},
        {"add-main",    required_argument,  0, 'a'},
        {"delete",  required_argument, 0, 'd'},
        {"create",  required_argument, 0, 'c'},
        {"file",    required_argument, 0, 'f'},
        {0, 0, 0, 0}
        };
    
    while(1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "a:bc:d:e:o:",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                    printf ("\n");
                    break;

            case 'o': // --output (sets the output image file name)
                output_filename = optarg;
                break;

            case 'e': // --add-efi (adds a directory to the EFI system partition)
                esp_foldername = optarg;
                break;
            
            case 'a': // --add-main (adds a directory to the main partition)
                main_foldername = optarg;
                break;

            case 'c':
                printf ("option -c with value `%s'\n", optarg);
                break;

            case 'd':
                printf ("option -d with value `%s'\n", optarg);
                break;

            case 'f':
                printf ("option -f with value `%s'\n", optarg);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort ();
        }
    }

    gpt_table_lba_sz = GPT_TABLE_SIZE / lba_size;
    uint64_t padding = (ALIGNMENT * NUM_PARTITIONS + (lba_size * ((gpt_table_lba_sz * 2) + 3))); // Extra padding for MBR, GPD headers and tables + partition alignment
    image_size = esp_size + data_size + padding;
    image_lba_sz = bytes_to_lbas(image_size);
    image_filename = output_filename;
    align_lba = ALIGNMENT / lba_size;

    esp_lba_st = align_lba;
    esp_lba_sz = bytes_to_lbas(esp_size);
    data_lba_st = next_aligned_lba(esp_lba_st + esp_lba_sz);
    data_lba_sz = bytes_to_lbas(data_size);

    srand(time(NULL));

    FILE* image = fopen(output_filename, "wb+");
    if(!image) {
        fprintf(stderr, "Error: could not open file '%s'\n", output_filename);
        exit(EXIT_FAILURE);
    }

    truncate(output_filename, image_size);

    bool mbr_write_success = mbr_write(image);
    bool gpt_write_success = gpt_write(image);

    fclose(image);

    /* Formatting partitions as FAT32 and then adding files */

    system("mkdir loopdir");

    char str[512] = {0};
    sprintf(str, "sudo losetup -fP %s", image_filename);
    system(str);

    memset((void*) str, 0, 512);

    char loop_dev_name[512] = {0};

    sprintf(str, "sudo losetup -j %s", image_filename);

    FILE* pf;
    pf = popen(str,"r"); 

    // Error handling

    // Get the data from the process execution
    fgets(loop_dev_name, 512, pf);
    pclose(pf);

    memset((void*) str, 0, 512);
    
    for(int i = 0; i < 512; i++) {
        if(loop_dev_name[i] == ':') {
            loop_dev_name[i] = 0;
            break;
        }
    }

    printf("Here\n");

    sprintf(str, "sudo mkfs.fat -F32 -v -I '%sp1'", loop_dev_name);
    system(str);
    memset((void*) str, 0, 512);

    sprintf(str, "sudo mkfs.fat -F32 -v -I '%sp2'", loop_dev_name);
    system(str);
    memset((void*) str, 0, 512);

    /* Adding files to ESP */

    sprintf(str, "sudo mount -o loop %sp1 loopdir", loop_dev_name);
    system(str);
    memset((void*) str, 0, 512);

    sprintf(str, "sudo cp -R %s/* loopdir/", esp_foldername);
    system(str);
    memset((void*) str, 0, 512);

    system("sudo umount loopdir");

    /* Adding files to main partition */

    if(main_foldername != "") {
        sprintf(str, "sudo mount -o loop %sp2 loopdir", loop_dev_name);
        system(str);
        memset((void*) str, 0, 512);

        sprintf(str, "sudo cp -R %s/* loopdir/", main_foldername);
        system(str);
        memset((void*) str, 0, 512);

        system("sudo umount loopdir");
    }

    /* Cleaning up */
    
    sprintf(str, "sudo losetup -d %s", loop_dev_name);
    system(str);
    memset((void*) str, 0, 512);

    system("sudo rm -rf loopdir");

    //fat32_t* esp = fat_init(image, esp_lba_st, esp_lba_st + esp_lba_sz);
    
    return 0;
}
