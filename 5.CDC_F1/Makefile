progname := firmware
progdir := res
files = main  usb_lib usb_class_cdc
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
prog: $(frmname).bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "init" -c "reset halt" -c "flash write_image erase "$(frmname).bin" 0x08000000" -c "reset run" -c "exit"
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
$(builddir)/crt.o:
	mkdir -p $(builddir)
	$(AS) -o $(builddir)/crt.o $(startup)
	
$(builddir)/%.o: $(srcdir)/%.c
	mkdir -p $(builddir)
	$(CC) $(CFLAGS) -c $< -o $@
	
-include $(shell mkdir -p $(builddir)/dep) $(wildcard $(builddir)/dep/*)
