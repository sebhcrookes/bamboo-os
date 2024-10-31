#include <iostream>

#include <string>

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "mbr.h"
#include "gpt.h"
#include "layout.h"

int main(int argc, char *argv[]) {

    std::string output_filename = "";
    std::string esp_foldername = "";
    std::string main_foldername = "";

    std::string mainfs_name = "BambooOS";
    
    // Loop through all the options - getopt returns -1 when no more are present
    int option;
    while((option = getopt(argc, argv, "a:e:o:")) != -1) {
        switch (option) {
            case 'o': // --output (sets the output image file name)
                output_filename = optarg;
                break;

            case 'e': // --add-efi (adds a directory to the EFI system partition)
                esp_foldername = optarg;
                break;
            
            case 'a': // --add-main (adds a directory to the main partition)
                main_foldername = optarg;
                break;

            default:
                abort();
        }
    }

    // Checking that all arguments have been passed
    if(output_filename == "" || esp_foldername == "" || main_foldername == "") {
        fprintf(stderr, "Error: incorrect arguments passed - output filename, ESP directory path and main FS directory path are all required\n");
        exit(EXIT_FAILURE);
    }

    std::string image_filename = output_filename;

    srand(time(NULL));

    /* === Generating blank .img file with MBR and GPT === */

    // The layout class stores and calculates the disk layout - where partitions are, padding etc.
    Layout layout(512, 50 * MEGABYTE, 100 * MEGABYTE);

    // Using C file I/O operations as it is easier for precise control with binary files
    FILE* image = fopen(output_filename.c_str(), "wb+");
    if(!image) {
        fprintf(stderr, "Error: could not open file '%s'\n", output_filename);
        exit(EXIT_FAILURE);
    }

    // Set the file size
    truncate(output_filename.c_str(), layout.get_image_size() * layout.get_lba_size());

    // Writing the MBR
    MBR mbr = MBR(layout);
    mbr.write(image);

    // Writing the GPT headers + partition tables
    GPT gpt = GPT(layout);
    gpt.write(image);

    fclose(image);

    /* ===== Formatting partitions as FAT32 and then adding files ===== */

    std::string command;
    system("mkdir loopdir");

    /* === Create loop device associated with the image === */
    system(("sudo losetup -fP " + image_filename).c_str());

    /* === Get loop device's name by opening a pipe === */
    char losetup_output[512] = {0};
    std::string loop_device;

    /* === Use popen to get result of "sudo losetup -j %.img" === */
    FILE* pf = popen(("sudo losetup -j " + image_filename).c_str(), "r");
    fgets(losetup_output, 512, pf);
    pclose(pf);
    
    // Extracting the path - the loop device path ends at the ':' character
    for(int i = 0; i < 512; i++) {
        if(losetup_output[i] == ':') {
            losetup_output[i] = 0;
            break;
        }
        loop_device += losetup_output[i];
    }

    /* === Creating two FAT32 partitions - the ESP partition and the main partition === */
    // Here, instead of writing a whole FAT32 driver, we just use mkfs.fat and loopback devices

    system(("sudo mkfs.fat -F32 -v -I '" + loop_device + "p1'").c_str());
    system(("sudo mkfs.fat -F32 -v -I '" + loop_device + "p2' -n \"" + mainfs_name + "\"").c_str());

    /* === Adding files in the given ESP directory to ESP === */

    system(("sudo mount -o loop " + loop_device + "p1 loopdir").c_str()); // Mounts the ESP partition to the loop directory

    system(("sudo cp -R " + esp_foldername + "/* loopdir/").c_str()); // Copies the ESP folder into the ESP partition

    system("sudo umount loopdir");

    /* === Adding files in the given main FS directory to main partition === */

    system(("sudo mount -o loop " + loop_device + "p2 loopdir").c_str()); // Mounts the main partition to the loop directory

    system(("sudo cp -R " + main_foldername + "/* loopdir/").c_str()); // Copies the main folder into the main partition

    system("sudo umount loopdir");

    /* === Cleaning up === */
    
    system(("sudo losetup -d " + loop_device).c_str()); // Delete the loop device
    system("sudo rm -rf loopdir"); // Delete the loop directory
    
    return 0;
}
