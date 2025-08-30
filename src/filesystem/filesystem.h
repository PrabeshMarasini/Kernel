#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

#define ATA_SECTOR_SIZE 512
#define MAX_FILES 512              
#define FILENAME_LENGTH 64        
#define MAX_FILE_CONTENT ATA_SECTOR_SIZE      

typedef struct {
    char filename[FILENAME_LENGTH];    
    int is_occupied;
    uint32_t size;                
    uint32_t start_sector;    
    char content[MAX_FILE_CONTENT];  
    int is_open;      
} FileEntry;

extern FileEntry file_table[MAX_FILES];


typedef struct {
    int index;                     
    uint32_t pos;                  
} File;

void init_fs();                                          
int create_file(const char* filename, const uint8_t* content, uint32_t size); 
int delete_file(const char* name);                        
int fs_open(const char* name);                            
int fs_read(int file_index, uint8_t* buffer, uint32_t size); 
char* list_files();                                       
void save_file_table();                                   
void load_file_table();
void ensure_file_table_loaded();
void mark_file_table_dirty();
int save_file(const char* filename, const char* content, uint32_t size);
int fs_close(int file_index);

#endif 
