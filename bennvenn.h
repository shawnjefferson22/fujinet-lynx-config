#ifndef BENNVENN_H
#define BENNVEN_H



// File Index Structure: 64 bytes total. LFN=[0:46]; SFN [47:54] File/Folder Marker[55] ('1'=File, '2'=folder); File Size [56:59]; Cluster Address on SD [60:63]
typedef struct {
  char long_name[47];				// probably not null terminated
  char short_name[8];				// definitely not null terminated
  unsigned char type;				// 1 = File, 2 = Folder
  unsigned long	size;				// file size
  unsigned long cluster;			// cluster address on SD
} BV_FILE_STRUCT;

extern unsigned int bennvenn_num_files;
extern unsigned int bennvenn_num_folders;

// eeprom function declarations, in assembler source file
extern unsigned int __fastcall__ lynx_eewrite_BV(unsigned int cell, unsigned int val);
extern unsigned int __fastcall__ lynx_eeread_BV(unsigned char cell);


// bennvenn sdcard functions
void bennvenn_get_response(void);
unsigned char bennvenn_send_command(char data[64], unsigned char length);
void bennvenn_open(unsigned int num);
void bennvenn_file_count(void);
void bennvenn_read_next_dir_entry(char *entry);
void bennvenn_set_dir_pos(unsigned int pos);
void bennvenn_get_dir_entries(void);
void bennvenn_get_dir_entry(unsigned char num, char *entry);


void test_bennvenn_dir(void);


#endif
