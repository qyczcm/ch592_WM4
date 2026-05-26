################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include StdPeriphDriver/subdir.mk
-include Startup/subdir.mk
-include RVMSIS/subdir.mk
-include Profile/subdir.mk
-include LIB/subdir.mk
-include HAL/subdir.mk
-include APP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
BLE_Mesh_App.hex \

SECONDARY_LIST += \
BLE_Mesh_App.lst \

SECONDARY_SIZE += \
BLE_Mesh_App.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 
	$(MAKE) --no-print-directory post-build 

main-build: BLE_Mesh_App.elf secondary-outputs

post-build: 
	Combine.bat

# Tool invocations
BLE_Mesh_App.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -T "e:\wavemail\防跌\V1有人无人传感器\core_ch592_mesh\BLE\MESH\BLE_Mesh_Project\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"e:\wavemail\防跌\V1有人无人传感器\core_ch592_mesh\BLE\MESH\BLE_Mesh_Project" -L"e:\wavemail\防跌\V1有人无人传感器\core_ch592_mesh\BLE\MESH\MESH_LIB" -L"e:\wavemail\防跌\V1有人无人传感器\core_ch592_mesh\BLE\LIB" -L"e:\wavemail\防跌\V1有人无人传感器\core_ch592_mesh\SRC\StdPeriphDriver" -Xlinker --print-memory-usage -Wl,-Map,"BLE_Mesh_App.map" --specs=nano.specs --specs=nosys.specs -o "BLE_Mesh_App.elf" $(OBJS) $(LIBS)
BLE_Mesh_App.hex: BLE_Mesh_App.elf
	@	riscv-none-embed-objcopy -O ihex "BLE_Mesh_App.elf" "BLE_Mesh_App.hex"
BLE_Mesh_App.lst: BLE_Mesh_App.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "BLE_Mesh_App.elf" > "BLE_Mesh_App.lst"
BLE_Mesh_App.siz: BLE_Mesh_App.elf
	riscv-none-embed-size --format=berkeley "BLE_Mesh_App.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) BLE_Mesh_App.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets