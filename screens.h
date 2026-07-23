#ifndef SCREENS_H
#define SCREENS_H


// UI keys
#define ADAPTERCFG_KEY    'P'
#define SCANWIFI_KEY      '2'
#define FILEINFO_KEY      '1'
#define REFRESH_KEY       '1'

extern FN_SSID_DETAIL networks[10];    // ssid display (340 bytes)
extern unsigned char sel_host;         // store sel_host for directory display
extern char dirpath[256];              // directory path to pass to open directory
extern char filename[256];             // filename buffer
extern unsigned char dir_last_page;    // last directory page?

// Screens
bool select_wifi_network(void);
bool select_host(void);
void show_ext_file_info(unsigned char dirpos);
bool get_file(unsigned char disk_slot, unsigned char dirpos);
void select_files(void);
bool select_sdcard_dir(void);

// Fujinet hosts and files helpers
bool read_hosts(void);
void open_dir(void);
bool read_full_dir_entry(unsigned int pos, char *entry, unsigned char ext);
void get_dir_entries(void);


#endif