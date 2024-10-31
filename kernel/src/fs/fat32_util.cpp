#include "fat32.h"

#include <vector.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#include "../ahci/ahci.h"

bool FAT32::exists(const char* filename) {
    DirectoryEntryFat32* dirent = get_dirent(filename);

    if(dirent == nullptr) {
        return false;
    } else {
        free((void*) dirent);
        return true;
    }
}

DirectoryEntryFat32* FAT32::get_dirent(const char* filename) {
    
    DirectoryEntryFat32* return_dirent = (DirectoryEntryFat32*) malloc(sizeof(DirectoryEntryFat32));
    
    char* ptr = strtok((char*) filename, '/');
    
    uint32_t address = root_start;
    uint16_t entries = 16;

    bool found = false;
    file_t* file;

    Vector* lfns = new Vector();

    // While ptr is not NULL
    while (ptr != nullptr) {
        // Tokenize the string 
        // Print the string

        if(!found) {
            DirectoryEntryFat32 dirent[16];
            //ahci::driver->read(0, address, 1, (uint8_t*) &dirent[0]);

            for(int i = 0; i < 16; i++) {

                if(dirent[i].name[0] == '\0') break;

                if(dirent[i].attributes == 0x0F) { // Is a LFN entry
                    LFNEntryFat32 lfn_entry = *(LFNEntryFat32*) &dirent[i];
                    lfns->add((void*) &dirent[i]);
                } else if(dirent[i].attributes & (1 << 4)) { // Is a directory (0x10 - directory, and 0x10 == 0b10000)
                    
                    bool is_match = false;

                    if(lfns->size() != 0) {
                        is_match = is_lfn_equal(lfns, ptr);
                        lfns->delete_all();
                    } else {
                        is_match = is_sfn_equal((char*) dirent[i].name, (char*) dirent[i].ext, ptr);
                    }

                    if(is_match) {
                        uint32_t file_cluster = ((uint32_t) dirent[i].first_cluster_high) << 16
                                            | ((uint32_t) dirent[i].first_cluster_low);
                        
                        address = root_start + bpb->sectors_per_cluster * (file_cluster-2); // Finding the new directory's start address from its cluster

                        i = 1000000; // Set to an arbitrarily large value to exit the loop
                    }
                } else {
                    bool is_match = false;

                    if(lfns->size() != 0) {
                        is_match = is_lfn_equal(lfns, ptr);
                        lfns->delete_all();
                    } else {
                        is_match = is_sfn_equal((char*) dirent[i].name, (char*) dirent[i].ext, ptr);
                    }

                    if(is_match) {
                        memcpy((void*) return_dirent, (void*) &dirent[i], sizeof(DirectoryEntryFat32));
                        found = true;
                    }
                }
            }

            lfns->delete_all();
        }

        free((void*) ptr);
        ptr = strtok(nullptr, '/');
    }

    delete lfns;

    if(found) {
        return return_dirent;
    } else {
        free((void*) return_dirent);
        return nullptr;
    }
}

bool FAT32::is_sfn_equal(char* sfname, char* sfext, char* filename) {
    bool has_dot = false;

    uint8_t filename_length = strlen(filename);

    // Determining whether or not the filename has a '.'
    for(uint8_t i = 0; i < filename_length; i++) {
        if(filename[i] == '.') {
            has_dot = true;
            break;
        }
    }

    if(has_dot) {
        // Checking the file name
        uint8_t i = 0;

        while(true) {
            if(filename[i] == '.') { // If we meet a '.', we have reached the extension
                i++;
                break;
            }
            
            if(tolower(sfname[i]) != tolower(filename[i])) return false; // Check if each char is equal
            i++;
        }

        // Ensuring that the other characters are spaces
        uint8_t j = i;
        while(j < 8) {
            if(sfname[j] != ' ') return false;
            j++;
        }

        // Checking the file extension
        uint8_t filename_ext_len = strlen(filename) - i;
        uint8_t ext_i = 0; // Index of the file extension

        // Ensuring that the extension is equal
        while(ext_i < filename_ext_len) {
            if(tolower(filename[i + ext_i]) != tolower(sfext[ext_i])) return false;
            ext_i++;
        }

        // If there is need for whitespace padding, we check it
        while(ext_i < 3) {
            if(sfext[ext_i] != ' ') return false;
            ext_i++;
        }

        return true;
    } else { // Else there is no extension
        if(sfext[0] != ' ' || sfext[1] != ' ' || sfext[2] != ' ') return false; // There shouldn't be an extension

        uint8_t i = 0;
        while(i < filename_length) { // We iterate through the characters present in the filename
            if(tolower(filename[i]) != tolower(sfname[i])) return false;
            i++;
        }

        while(i < 8) { // Then, we ensure the rest of the characters are spaces
            if(sfname[i] != ' ') return false;
            i++;
        }

        return true;
    }

    return false;
}

bool FAT32::is_lfn_equal(Vector* backwards_lfn, char* filename) {
    uint8_t lfn_vlen = backwards_lfn->size();
    uint8_t filename_len = strlen(filename);
    uint8_t filename_i = 0;

    for(uint8_t i = lfn_vlen - 1; i >= 0; i--) {
        LFNEntryFat32 lfn = *(LFNEntryFat32*) backwards_lfn->get(i);
        for(int j = 0; j < 10; j += 2) {
            if(lfn.first_chars[j] == 0) goto returnTrue;
            if(lfn.first_chars[j] != filename[filename_i]) return false;
            filename_i++;
        }

        for(int j = 0; j < 12; j += 2) {
            if(lfn.second_chars[j] == 0) goto returnTrue;
            if(lfn.second_chars[j] != filename[filename_i]) return false;
            filename_i++;
        }

        for(int j = 0; j < 4; j += 2) {
            if(lfn.third_chars[j] == 0) goto returnTrue;
            if(lfn.third_chars[j] != filename[filename_i]) return false;
            filename_i++;
        }

        if(filename_i == filename_len && i == 0) goto returnTrue;
    }

    returnTrue:
    if(filename_i != filename_len) return false;
    return true;
}