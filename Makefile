# Makefile for fujinet-lynx-config

# Supported sdcard targets: bennvenn, gamedrive, none 
TARGET ?= no_sdcard

# Toolchain
CC65 = cc65
CA65 = ca65
LD65 = ld65
CL65 = cl65

# Source files
C_SOURCES := $(wildcard *.c)
ASM_SOURCES := $(wildcard *.s)

# Object files (from both C and ASM)
OBJECTS := $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)

# Output name
OUTPUT = lynxcfg

# Define macro for target
ifeq ($(TARGET),bv_sdcard)
    TARGET_DEFINE = SDCARD_BENNVENN
else ifeq ($(TARGET),gd_sdcard)
    TARGET_DEFINE = SDCARD_GAMEDRIVE
	HEADER_FIX = 1
else ifeq ($(TARGET),no_sdcard)
    TARGET_DEFINE = SDCARD_NONE
else
    $(error Invalid TARGET specified. Choose from: bv_sdcard, gd_sdcard, no_sdcard)
endif

# Compiler flags
CFLAGS = -t lynx -Osir -D$(TARGET_DEFINE)
AFLAGS = -t lynx

.PHONY: all clean

all: $(OUTPUT).lnx

$(OUTPUT).lnx: $(OBJECTS)
	$(CL65) -t lynx -m $(OUTPUT).map -o $@ $^

ifdef HEADER_FIX
	# Fix the LNX file header to tell the GameDrive Loader we want to use EEPROM
	# \100 is octal for 0x40
	printf '\100' | dd of="lynxcfg.lnx" bs=1 seek=60 count=1 conv=notrunc
endif

%.o: %.c
	$(CL65) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CA65) $(AFLAGS) -o $@ $<

clean:
	rm -f *.o *.lnx *.lst *.map