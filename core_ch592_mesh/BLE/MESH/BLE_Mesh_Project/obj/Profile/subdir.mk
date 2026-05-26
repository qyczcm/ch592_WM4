################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Profile/devinfoservice.c \
../Profile/gattprofile.c 

C_DEPS += \
./Profile/devinfoservice.d \
./Profile/gattprofile.d 

OBJS += \
./Profile/devinfoservice.o \
./Profile/gattprofile.o 

DIR_OBJS += \
./Profile/*.o \

DIR_DEPS += \
./Profile/*.d \

DIR_EXPANDS += \
./Profile/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Profile/%.o: ../Profile/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DBLE_BUFF_MAX_LEN=251 -DLIB_FLASH_BASE_ADDRESSS=0x0004E000 -DCH59xBLE_ROM -DBLE_MEMHEAP_SIZE=5632 -DHAL_KEY=1 -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Startup" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/APP/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Profile/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/StdPeriphDriver/inc" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Ld" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/LIB" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/RVMSIS" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/MESH_LIB" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

