/**
 *  for Atari Lynx
 *
 * @brief Fujinet routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef FUJINET_H
#define FUJINET_H

/*
enum fujiCommandID_t {
  FUJICMD_RESET                      = 0xFF,
  FUJICMD_SPECIAL_QUERY              = 0xFF,
  FUJICMD_GET_SSID                   = 0xFE,
  FUJICMD_PASSWORD                   = 0xFE,
  FUJICMD_SCAN_NETWORKS              = 0xFD,
  FUJICMD_USERNAME                   = 0xFD,
  FUJICMD_GET_SCAN_RESULT            = 0xFC,
  FUJICMD_JSON                       = 0xFC,
  FUJICMD_SET_SSID                   = 0xFB,
  FUJICMD_GET_WIFISTATUS             = 0xFA,
  FUJICMD_MOUNT_HOST                 = 0xF9,
  FUJICMD_MOUNT_IMAGE                = 0xF8,
  FUJICMD_OPEN_DIRECTORY             = 0xF7,
  FUJICMD_READ_DIR_ENTRY             = 0xF6,
  FUJICMD_CLOSE_DIRECTORY            = 0xF5,
  FUJICMD_READ_HOST_SLOTS            = 0xF4,
  FUJICMD_WRITE_HOST_SLOTS           = 0xF3,
  FUJICMD_READ_DEVICE_SLOTS          = 0xF2,
  FUJICMD_WRITE_DEVICE_SLOTS         = 0xF1,
  FUJICMD_ENABLE_UDPSTREAM           = 0xF0,
  FUJICMD_SET_BAUDRATE               = 0xEB,
  FUJICMD_GET_WIFI_ENABLED           = 0xEA,
  FUJICMD_UNMOUNT_IMAGE              = 0xE9,
  FUJICMD_GET_ADAPTERCONFIG          = 0xE8,
  FUJICMD_NEW_DISK                   = 0xE7,
  FUJICMD_UNMOUNT_HOST               = 0xE6,
  FUJICMD_GET_DIRECTORY_POSITION     = 0xE5,
  FUJICMD_SET_DIRECTORY_POSITION     = 0xE4,
  FUJICMD_SET_HSIO_INDEX             = 0xE3,
  FUJICMD_SET_DEVICE_FULLPATH        = 0xE2,
  FUJICMD_SET_HOST_PREFIX            = 0xE1,
  FUJICMD_GET_HOST_PREFIX            = 0xE0,
  FUJICMD_SET_SIO_EXTERNAL_CLOCK     = 0xDF,
  FUJICMD_WRITE_APPKEY               = 0xDE,
  FUJICMD_READ_APPKEY                = 0xDD,
  FUJICMD_OPEN_APPKEY                = 0xDC,
  FUJICMD_CLOSE_APPKEY               = 0xDB,
  FUJICMD_GET_DEVICE_FULLPATH        = 0xDA,
  FUJICMD_CONFIG_BOOT                = 0xD9,
  FUJICMD_COPY_FILE                  = 0xD8,
  FUJICMD_MOUNT_ALL                  = 0xD7,
  FUJICMD_HRS232_WRITE               = 0xD7,
  FUJICMD_SET_BOOT_MODE              = 0xD6,
  FUJICMD_ENABLE_DEVICE              = 0xD5,
  FUJICMD_DISABLE_DEVICE             = 0xD4,
  FUJICMD_RANDOM_NUMBER              = 0xD3,
  FUJICMD_HRS232_STATUS              = 0xD3,
  FUJICMD_GET_TIME                   = 0xD2,
  FUJICMD_HRS232_READ                = 0xD2,
  FUJICMD_DEVICE_ENABLE_STATUS       = 0xD1,
  FUJICMD_BASE64_ENCODE_INPUT        = 0xD0,
  FUJICMD_HRS232_PUT                 = 0xD0,
  FUJICMD_BASE64_ENCODE_COMPUTE      = 0xCF,
  FUJICMD_BASE64_ENCODE_LENGTH       = 0xCE,
  FUJICMD_BASE64_ENCODE_OUTPUT       = 0xCD,
  FUJICMD_BASE64_DECODE_INPUT        = 0xCC,
  FUJICMD_BASE64_DECODE_COMPUTE      = 0xCB,
  FUJICMD_BASE64_DECODE_LENGTH       = 0xCA,
  FUJICMD_BASE64_DECODE_OUTPUT       = 0xC9,
  FUJICMD_HASH_INPUT                 = 0xC8,
  FUJICMD_HASH_COMPUTE               = 0xC7,
  FUJICMD_HASH_LENGTH                = 0xC6,
  FUJICMD_HASH_OUTPUT                = 0xC5,
  FUJICMD_GET_ADAPTERCONFIG_EXTENDED = 0xC4,
  FUJICMD_HASH_COMPUTE_NO_CLEAR      = 0xC3,
  FUJICMD_HASH_CLEAR                 = 0xC2,
  FUJICMD_GET_HEAP                   = 0xC1,
  FUJICMD_QRCODE_OUTPUT              = 0xBF,
  FUJICMD_QRCODE_LENGTH              = 0xBE,
  FUJICMD_QRCODE_ENCODE              = 0xBD,
  FUJICMD_QRCODE_INPUT               = 0xBC,
  FUJICMD_GET_DEVICE8_FULLPATH       = 0xA7,
  FUJICMD_GET_DEVICE7_FULLPATH       = 0xA6,
  FUJICMD_GET_DEVICE6_FULLPATH       = 0xA5,
  FUJICMD_GET_DEVICE5_FULLPATH       = 0xA4,
  FUJICMD_GET_DEVICE4_FULLPATH       = 0xA3,
  FUJICMD_GET_DEVICE3_FULLPATH       = 0xA2,
  FUJICMD_HRS232_FORMAT_MEDIUM       = 0xA2,
  FUJICMD_GET_DEVICE2_FULLPATH       = 0xA1,
  FUJICMD_HRS232_FORMAT              = 0xA1,
  FUJICMD_GET_DEVICE1_FULLPATH       = 0xA0,
  FUJICMD_GETTZTIME                  = 0x9A,
  FUJICMD_SETTZ                      = 0x99,
  FUJICMD_GETTIME                    = 0x93,
  FUJICMD_JSON_QUERY                 = 0x81,
  FUJICMD_JSON_PARSE                 = 0x80,
  FUJICMD_GET_REMOTE                 = 0x72, // r
  FUJICMD_CLOSE_CLIENT               = 0x63, // c
  FUJICMD_TIMER                      = 0x5A, // Z
  FUJICMD_STREAM                     = 0x58, // X
  FUJICMD_WRITE                      = 0x57, // W
  FUJICMD_TRANSLATION                = 0x54, // T
  FUJICMD_STATUS                     = 0x53, // S
  FUJICMD_READ                       = 0x52, // R
  FUJICMD_QUERY                      = 0x51, // Q
  FUJICMD_PUT                        = 0x50, // P
  FUJICMD_PARSE                      = 0x50, // P
  FUJICMD_AUTOANSWER                 = 0x4F, // O
  FUJICMD_PERCOM_WRITE               = 0x4F, // O
  FUJICMD_OPEN                       = 0x4F, // O
  FUJICMD_BAUDRATELOCK               = 0x4E, // N
  FUJICMD_PERCOM_READ                = 0x4E, // N
  FUJICMD_UNLISTEN                   = 0x4D, // M
  FUJICMD_LISTEN                     = 0x4C, // L
  FUJICMD_GET_ERROR                  = 0x45, // E
  FUJICMD_SET_DESTINATION            = 0x44, // D
  FUJICMD_SET_DUMP                   = 0x44, // D
  FUJICMD_CLOSE                      = 0x43, // C
  FUJICMD_CONFIGURE                  = 0x42, // B
  FUJICMD_CONTROL                    = 0x41, // A
  FUJICMD_TYPE3_POLL                 = 0x40, // @
  FUJICMD_TYPE1_POLL                 = 0x3F,
  FUJICMD_HSIO_INDEX                 = 0x3F, // ?
  FUJICMD_GETCWD                     = 0x30, // 0
  FUJICMD_CHDIR                      = 0x2C, // ,
  FUJICMD_RMDIR                      = 0x2B, // +
  FUJICMD_MKDIR                      = 0x2A, // *
  FUJICMD_TELL                       = 0x26, // &
  FUJICMD_LOAD_HANDLER               = 0x26, // &
  FUJICMD_SEEK                       = 0x25, // %
  FUJICMD_UNLOCK                     = 0x24, // $
  FUJICMD_LOCK                       = 0x23, // #
  FUJICMD_FORMAT_MEDIUM              = 0x22, // "
  FUJICMD_DELETE                     = 0x21, // !
  FUJICMD_LOAD_RELOCATOR             = 0x21, // !
  FUJICMD_FORMAT                     = 0x21, // !
  FUJICMD_RENAME                     = 0x20,
  FUJICMD_NAK                        = 0x15, // ASCII NAK
  FUJICMD_ACK                        = 0x06, // ASCII ACK
  FUJICMD_SEND_ERROR                 = 0x02,
  FUJICMD_SEND_RESPONSE              = 0x01,
  FUJICMD_DEVICE_READY               = 0x00,
};
*/

// Fujinet commands (not all are implemented for Lynx)
/*
#define FUJICMD_RESET				        0xFF			// Lynx
#define FUJICMD_GET_SSID			        0xFE			// Lynx
#define FUJICMD_SCAN_NETWORKS			    0xFD			// Lynx
#define FUJICMD_GET_SCAN_RESULT			    0xFC			// Lynx
#define FUJICMD_SET_SSID			        0xFB			// Lynx
#define FUJICMD_GET_WIFISTATUS			    0xFA			// Lynx
#define FUJICMD_MOUNT_HOST			        0xF9			//
#define FUJICMD_MOUNT_IMAGE			        0xF8			//
#define FUJICMD_OPEN_DIRECTORY			    0xF7			//
#define FUJICMD_READ_DIR_ENTRY			    0xF6			//
#define FUJICMD_CLOSE_DIRECTORY			    0xF5			//
#define FUJICMD_READ_HOST_SLOTS			    0xF4			//
#define FUJICMD_WRITE_HOST_SLOTS		    0xF3			//
#define FUJICMD_READ_DEVICE_SLOTS		    0xF2			//
#define FUJICMD_WRITE_DEVICE_SLOTS		    0xF1			//
#define FUJICMD_GET_WIFI_ENABLED		    0xEA
#define FUJICMD_UNMOUNT_IMAGE			    0xE9			//
#define FUJICMD_GET_ADAPTERCONFIG		    0xE8
#define FUJICMD_NEW_DISK			        0xE7
#define FUJICMD_UNMOUNT_HOST			    0xE6			//
#define FUJICMD_GET_DIRECTORY_POSITION	   	0xE5			//
#define FUJICMD_SET_DIRECTORY_POSITION	   	0xE4			//
#define FUJICMD_SET_HSIO_INDEX			    0xE3
#define FUJICMD_SET_DEVICE_FULLPATH		    0xE2
#define FUJICMD_SET_HOST_PREFIX			    0xE1
#define FUJICMD_GET_HOST_PREFIX			    0xE0
#define FUJICMD_SET_SIO_EXTERNAL_CLOCK	   	0xDF
#define FUJICMD_WRITE_APPKEY			    0xDE
#define FUJICMD_READ_APPKEY			        0xDD
#define FUJICMD_OPEN_APPKEY			        0xDC
#define FUJICMD_CLOSE_APPKEY			    0xDB
#define FUJICMD_GET_DEVICE_FULLPATH		    0xDA
#define FUJICMD_GET_DEVICE1_FULLPATH	   	0xA0
#define FUJICMD_GET_DEVICE2_FULLPATH	    0xA1
#define FUJICMD_GET_DEVICE3_FULLPATH	   	0xA2
#define FUJICMD_GET_DEVICE4_FULLPATH	   	0xA3
#define FUJICMD_GET_DEVICE5_FULLPATH	   	0xA4
#define FUJICMD_GET_DEVICE6_FULLPATH	   	0xA5
#define FUJICMD_GET_DEVICE7_FULLPATH	   	0xA6
#define FUJICMD_GET_DEVICE8_FULLPATH	   	0xA7
#define FUJICMD_CONFIG_BOOT			        0xD9
#define FUJICMD_COPY_FILE			        0xD8
#define FUJICMD_MOUNT_ALL			        0xD7
#define FUJICMD_SET_BOOT_MODE			    0xD6
#define FUJICMD_STATUS				        0x53
#define FUJICMD_HSIO_INDEX			        0x3F
#define FUJICMD_ENABLE_UDPSTREAM		    0xF0
#define FUJICMD_ENABLE_DEVICE			    0xD5
#define FUJICMD_DISABLE_DEVICE			    0xD4
#define FUJICMD_RANDOM_NUMBER			    0xD3
#define FUJICMD_GET_TIME			        0xD2
#define FUJICMD_DEVICE_ENABLE_STATUS	   	0xD1
#define FUJICMD_QRCODE_INPUT               	0xBC
#define FUJICMD_QRCODE_ENCODE              	0xBD
#define FUJICMD_QRCODE_LENGTH              	0xBE
#define FUJICMD_QRCODE_OUTPUT              	0xBF
#define FUJICMD_BASE64_ENCODE_INPUT		    0xD0
#define FUJICMD_BASE64_ENCODE_COMPUTE	   	0xCF
#define FUJICMD_BASE64_ENCODE_LENGTH	    0xCE
#define FUJICMD_BASE64_ENCODE_OUTPUT	   	0xCD
#define FUJICMD_BASE64_DECODE_INPUT		    0xCC
#define FUJICMD_BASE64_DECODE_COMPUTE	   	0xCB
#define FUJICMD_BASE64_DECODE_LENGTH	   	0xCA
#define FUJICMD_BASE64_DECODE_OUTPUT	   	0xC9
#define FUJICMD_HASH_INPUT			        0xC8
#define FUJICMD_HASH_COMPUTE			    0xC7
#define FUJICMD_HASH_LENGTH			        0xC6
#define FUJICMD_HASH_OUTPUT			        0xC5
#define FUJICMD_GET_ADAPTERCONFIG_EXTENDED 	0xC4
#define FUJICMD_HASH_COMPUTE_NO_CLEAR	   	0xC3
#define FUJICMD_HASH_CLEAR			        0xC2
#define FUJICMD_SEND_ERROR			        0x02
#define FUJICMD_SEND_RESPONSE			    0x01
#define FUJICMD_DEVICE_READY			    0x00
*/

// Fujinet defines
#define MAX_SSID_LEN        32
#define MAX_WIFI_PASS_LEN   64
#define MAX_HOSTS			8
#define MAX_HOSTNAME_LEN	32
#define DIRPATH_LEN			256

// disk image access mode
#define DISK_ACCESS_MODE_READ 1
#define DISK_ACCESS_MODE_WRITE 2
#define DISK_ACCESS_MODE_FETCH 128

// device slot defines
#define MAX_DISPLAY_FILENAME_LEN    36
#define MAX_DISK_SLOTS              8


// Data structs
typedef struct
{
    char ssid[33];
    char hostname[64];
    unsigned char localIP[4];
    unsigned char gateway[4];
    unsigned char netmask[4];
    unsigned char dnsIP[4];
    unsigned char macAddress[6];
    unsigned char bssid[6];
    char fn_version[15];
} FN_ADAPTER_CONFIG;

typedef struct
{
    char ssid[MAX_SSID_LEN+1];
    unsigned char rssi;
} FN_SSID_DETAIL;

typedef struct
{
    char ssid[MAX_SSID_LEN+1];
    char password[MAX_WIFI_PASS_LEN];
} FN_SSID_PASS;

typedef struct
{
    unsigned char hostSlot;
    unsigned char mode;
    char filename[MAX_DISPLAY_FILENAME_LEN];
} FN_DISK_SLOT;

typedef struct
{
	unsigned char year;
	unsigned char month;
    unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	unsigned long size;
	unsigned char isdir;
	unsigned char type;
    unsigned char type_char;
	char filename[117];             // 116 + room for '/' to denote directory
} FILE_INFO_EXT;


// Static data structures
extern FN_SSID_DETAIL wifi;
extern FN_ADAPTER_CONFIG fncfg;
extern FN_SSID_PASS ssid_pass;
extern char host_slots[MAX_HOSTS][MAX_HOSTNAME_LEN];
extern FN_DISK_SLOT disk_slots[MAX_DISK_SLOTS];

// helper functions
unsigned char _send_cmd(unsigned int l);
unsigned char _send_cmd_and_recv_buf(unsigned int l, char *buf);

// Fujinet functions
unsigned char fujinet_get_wifi_status(void);
unsigned char fujinet_scan_networks(void);
unsigned char fujinet_scan_results(unsigned char n);
unsigned char fujinet_get_adapter_config(void);
unsigned char fujinet_get_ssid(void);
unsigned char fujinet_set_ssid(char *ssid, char *password);
unsigned char fujinet_read_host_slots(void);
unsigned char fujinet_write_host_slots(unsigned char *host_slots);
unsigned char fujinet_mount_host(unsigned char host);
unsigned char fujinet_unmount_host(unsigned char host);
unsigned char fujinet_open_directory(unsigned char host_slot, char *dirpath);
unsigned char fujinet_close_directory(void);
unsigned char fujinet_get_directory_position(unsigned int *pos);
unsigned char fujinet_set_directory_position(unsigned int pos);
unsigned char fujinet_read_directory_entry(unsigned char maxlen, unsigned char extra, char *dir_entry);
unsigned char fujinet_set_device_filename(unsigned char ds, char *filename);
unsigned char fujinet_mount_image(unsigned char ds, unsigned char options);
unsigned char fujinet_unmount_image(unsigned char ds);
unsigned char fujinet_mount_all(void);
unsigned char fujinet_read_device_slots(FN_DISK_SLOT *slots);
unsigned char fujinet_write_device_slots(FN_DISK_SLOT *slots);

#endif /* FUJINET_H */
