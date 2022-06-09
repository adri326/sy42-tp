# === User section ===

# Defines the name of the output binary
PROJECT = project

# When set to 1, enables debug flags and disables optimization
DEBUG = 1

# Input here the list of C files to compile
SRC = $(wildcard src/*.c)

# Optional assembly files to build
# ASM_SRC = your_file.s

# Path to the STM32F4XX library
STM32F4XX_DIR = /opt/STM32/STM32F4/
INCLUDE_PATHS += -I$(STM32F4XX_DIR)

TARGET_VERSION = STM32F401xE

# === Additional options ===

# Linker script
LINKER_SCRIPT = ./stm32f4/stm32f401retx_flash.ld

# Additional files required for the main code to run
SRC += $(wildcard stm32f4/*.c)
ASM_SRC += ./stm32f4/startup_stm32f401xe.s

# Additional flags to specify the CPU options
CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

CC_FLAGS += $(CPU_FLAGS) -specs=nano.specs -D$(TARGET_VERSION)

CPP_FLAGS += $(CPU_FLAGS) -specs=nano.specs -D$(TARGET_VERSION) -std=gnu++98

AS_FLAGS += $(CPU_FLAGS)

LD_FLAGS += $(CPU_FLAGS) -Wl,--gc-sections,-Map=$(PROJECT).map,--cref -specs=nano.specs
LD_FLAGS += -lm -lc -lgcc -lnosys -lstdc++ -lsupc++

# === Derived options section ===

# Tool definitions
PREFIX = arm-none-eabi
AS = $(PREFIX)-as
CC = $(PREFIX)-gcc
CPP = $(PREFIX)-g++
LD = $(PREFIX)-gcc
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
SIZE = $(PREFIX)-size
GDB = $(PREFIX)-gdb

# Debug flags
ifeq ($(DEBUG),1)
	CC_FLAGS += -O0 -g3 -DDEBUG -DUSE_FULL_ASSERT
	AS_FLAGS += -gdwarf-2
else
	CC_FLAGS += -O3
endif

# Object files
OBJ = $(SRC:%.c=%.o)
CPP_OBJ = $(CPP_SRC:%.cpp=%.o)
ASM_OBJ = $(ASM_SRC:%.s=%.o)

# === Compilation section ===

all: $(PROJECT).hex $(PROJECT).bin size
.PHONY: all

lst: $(PROJECT).lst
.PHONY: lst

size:
	@[ -f $(PROJECT).elf ] && $(SIZE) $(PROJECT).elf || \
	@echo "Couldn't find $(PROJECT).elf, please compile the project first!"

clean:
	rm -f $(PROJECT).elf $(PROJECT).bin $(PROJECT).hex $(PROJECT).map $(OBJ) $(ASM_OBJ)
.PHONY: clean

openocd:
	openocd -f board/st_nucleo_f4.cfg
# -c "program $(PROJECT).bin verify reset"
.PHONY: openocd

flash:
	openocd -f board/st_nucleo_f4.cfg -c "program $(PROJECT).elf reset" -c exit
.PHONY: flash

gdb:
	$(GDB) -ex "target extended-remote localhost:3333" -ex "monitor reset halt" $(PROJECT).elf
.PHONY: gdb

$(OBJ):%.o:%.c
	$(CC) $(CC_FLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(CPP_OBJ):%.o:%.cpp
	$(CPP) $(CPP_FLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(ASM_OBJ):%.o:%.s
	$(AS) $(AS_FLAGS) $< -o $@


# For some ungodly reason the `-o` option needs to be put before the other parameters
$(PROJECT).elf: $(OBJ) $(ASM_OBJ)
	$(LD) -o $@ $^ $(LD_FLAGS) -T$(LINKER_SCRIPT)


$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -O ihex $< $@

$(PROJECT).lst: $(PROJECT).elf
	$(OBJDUMP) -Sdh $< > $@
