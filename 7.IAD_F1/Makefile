progname := firmware
progdir := res
files = main  usb_lib usb_class systick programmer usb_class_msd
srcdir = src
builddir = build

OPTIMIZE = s
DEBUG = dwarf-2

startup = lib/startup_code/startup_stm32f10x_md.s
INCLUDS = -Ilib/include/
LINKERCMDFILE = -Llib/linker_scripts -Tlib/linker_scripts/stm32f10xC8.ld

DEVCLASS =  STM32F103xB

CFLAGS = -fno-builtin -Wall

CFLAGS += -MD -MP -MT $(builddir)/$(*F).o -MF $(builddir)/dep/$(@F).mk
CFLAGS += -mthumb -mcpu=cortex-m3 -g$(DEBUG) -O$(OPTIMIZE) -D$(DEVCLASS) $(INCLUDS) -DGCC_ARMCM3 -DVECT_TAB_FLASH
ASMFLAGS = -ahls -mapcs-32
LFLAGS  = $(LINKERCMDFILE) -nostartfiles -Wl,-Map -Wl,$(builddir)/$(progname).map -mthumb -mcpu=cortex-m3



CC      = arm-none-eabi-gcc
CPP      = arm-none-eabi-g++
#LD      = arm-none-eabi-ld -v
LD      = arm-none-eabi-gcc
AR      = arm-none-eabi-ar
AS      = arm-none-eabi-as
CP      = arm-none-eabi-objcopy
OD	= arm-none-eabi-objdump
SZ	= arm-none-eabi-size

frmname = $(progdir)/$(progname)
objs = $(addprefix $(builddir)/,$(addsuffix .o,$(files)))

all: $(frmname).bin $(frmname).hex $(frmname).lss size
clean:
	rm -rf $(progdir)
	rm -rf $(builddir)
test:
	echo $(calc_zip_size)
	echo
	echo $(CFLAGS)
prog: $(frmname).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "init" -c "reset halt" -c "flash write_image erase "$(frmname).bin" 0x08000000" -c "reset run" -c "exit"
reset: $(frmname).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "init" -c "reset halt" -c "reset run" -c "exit"
ftdi: $(frmname).bin
	openocd -f ft4232.cfg -f target/stm32f1x.cfg -c "init" -c "reset halt" -c "flash write_image erase "$(frmname).bin" 0x08000000" -c "reset run" -c "exit"
	
$(frmname).bin: $(frmname).elf
	$(CP) -Obinary $(frmname).elf $(frmname).bin
$(frmname).hex: $(frmname).elf
	$(CP) -Oihex $(frmname).elf $(frmname).hex
$(frmname).lss: $(frmname).elf
	$(OD) -D -S $(frmname).elf > $(frmname).lss
size: $(frmname).elf
	$(SZ) $(frmname).elf
$(frmname).elf: $(objs) $(builddir)/crt.o
	mkdir -p $(progdir)
	@ echo "..linking"
	$(LD) $(LFLAGS) -o $(frmname).elf $(builddir)/crt.o -L../ld $(objs)
	
$(builddir)/usb_class_msd.o: $(builddir)/src_zip.h
$(builddir)/src_zip.h:
	mkdir -p $(builddir)
	@echo "Build src_zip.h"
	rm -f $(builddir)/src.zip
	zip -r $(builddir)/src.zip src > /dev/null
	zip $(builddir)/src.zip Makefile > /dev/null
	echo "const uint8_t src_zip[] = {" > $@
	hexdump -v -e '1/1 "0x%.2X,"' $(builddir)/src.zip >> $@
	#xxd -i $(builddir)/src.zip >> $@
	echo "};" >> $@
	
$(builddir)/crt.o:
	mkdir -p $(builddir)
	$(AS) -o $(builddir)/crt.o $(startup)
$(builddir)/%.o: $(srcdir)/%.c
	mkdir -p $(builddir)
	$(CC) $(CFLAGS) -c $< -o $@

test_stflash:
	stty -F /dev/tty_STFLASH_0 300
	stty -F /dev/tty_STFLASH_0 50
	echo 'RBU' > /dev/tty_STFLASH_0
	echo 'rBU' > /dev/tty_STFLASH_0
	sleep 1
	stm32flash -r read.bin -S 0x08000000:32768 /dev/tty_STFLASH_0
	sleep 1
	echo 'RbU' > /dev/tty_STFLASH_0
	sleep 1
	echo 'rbuz' > /dev/tty_STFLASH_0
test_arduino:
	stty -F /dev/tty_STFLASH_0 9600
	stty -F /dev/tty_STFLASH_0 50
	avrdude -c arduino -p atmega8 -P /dev/tty_STFLASH_0 -b 115200 -Uflash:r:/dev/null:i
	stty -F /dev/tty_STFLASH_0 50
	echo 'z' > /dev/tty_STFLASH_0
	
-include $(shell mkdir -p $(builddir)/dep) $(wildcard $(builddir)/dep/*)
