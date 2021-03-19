progname := firmware
progdir := res
files = main usb_lib usb_class_hid
srcdir = src
builddir = build

OPTIMIZE = s
DEBUG = dwarf-2

startup = lib/startup_code/startup_stm32l151xc.s
FWLIB = lib
PHDRIVER=STM32F10x_StdPeriph_Driver
CORESUPPORT=CoreSupport
INCLUDS =  -I$(FWLIB)/$(CORESUPPORT) -I$(FWLIB)/include/
#-I$(FWLIB)/$(PHDRIVER)/inc
LINKERCMDFILE = lib/linker_scripts/STM32L1_256FLASH_32RAM.ld

COMFLAGS = -fno-builtin -Wall

COMFLAGS += -MD -MP -MT $(builddir)/$(*F).o -MF $(builddir)/dep/$(@F).mk
COMFLAGS += -g$(DEBUG) -O$(OPTIMIZE)  $(INCLUDS) -DGCC_ARMCM3 -DVECT_TAB_FLASH
#-D$(DEVCLASS)
CFLAGS = $(COMFLAGS) -mthumb -mcpu=cortex-m3
CXXFLAGS = $(COMFLAGS) -mthumb -mcpu=cortex-m3
ASMFLAGS = -ahls -mapcs-32
LFLAGS  = -T$(LINKERCMDFILE) -nostartfiles -Wl,-Map -Wl,$(builddir)/$(progname).map -mthumb -mcpu=cortex-m3 --specs=nano.specs --specs=nosys.specs

CFLAGS += -DFATSRC='"_binary_src_fatexample_img"'


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
prog2: $(frmname).bin
	openocd -f interface/ft232r.cfg -f target/stm32l1.cfg -c "init" -c "reset halt" -c "flash write_image erase "$(frmname).bin" 0x08000000" -c "reset run" -c "exit"
prog: $(frmname).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32l1.cfg -c "init" -c "reset halt" -c "flash write_image erase "$(frmname).bin" 0x08000000" -c "reset run" -c "exit"
prog3: $(frmname).bin
	-stm32flash -j /dev/ttyUSB0
	sleep 1
	-stm32flash -k /dev/ttyUSB0
	sleep 1
	-stm32flash -e 0 -w $(frmname).bin /dev/ttyUSB0
#target/stm32l1x_dual_bank.cf
$(frmname).bin: $(frmname).elf
	$(CP) -Obinary $(frmname).elf $(frmname).bin
$(frmname).hex: $(frmname).elf
	$(CP) -Oihex $(frmname).elf $(frmname).hex
$(frmname).lss: $(frmname).elf
	$(OD) -D -S $(frmname).elf > $(frmname).lss
size: $(frmname).elf
	$(SZ) $(frmname).elf
$(frmname).elf: $(objs) $(LINKERCMDFILE) $(builddir)/crt.o
	mkdir -p $(progdir)
	@ echo "..linking"
	$(LD) $(LFLAGS) -o $(frmname).elf $(builddir)/crt.o -L../ld $(objs)

$(builddir)/crt.o:
	mkdir -p $(builddir)
	$(AS) -o $(builddir)/crt.o $(startup)
	
$(builddir)/%.o: $(srcdir)/%.c
	mkdir -p $(builddir)
	$(CC) $(CFLAGS) -c $< -o $@
	
$(builddir)/%.o: $(srcdir)/%.cpp
	mkdir -p $(builddir)
	$(CPP) $(CXXFLAGS) -c $< -o $@
	
		
	
-include $(shell mkdir -p $(builddir)/dep) $(wildcard $(builddir)/dep/*)
