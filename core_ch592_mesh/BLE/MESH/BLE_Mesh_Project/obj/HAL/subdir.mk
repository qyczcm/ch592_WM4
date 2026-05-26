################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/KEY.c \
e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/LED.c \
e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/MCU.c \
e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/RTC.c \
e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/SLEEP.c 

C_DEPS += \
./HAL/KEY.d \
./HAL/LED.d \
./HAL/MCU.d \
./HAL/RTC.d \
./HAL/SLEEP.d 

OBJS += \
./HAL/KEY.o \
./HAL/LED.o \
./HAL/MCU.o \
./HAL/RTC.o \
./HAL/SLEEP.o 

DIR_OBJS += \
./HAL/*.o \

DIR_DEPS += \
./HAL/*.d \

DIR_EXPANDS += \
./HAL/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
HAL/KEY.o: e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/KEY.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DBLE_BUFF_MAX_LEN=251 -DLIB_FLASH_BASE_ADDRESSS=0x0004E000 -DCH59xBLE_ROM -DBLE_MEMHEAP_SIZE=5632 -DHAL_KEY=1 -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Startup" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/APP/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Profile/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/StdPeriphDriver/inc" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Ld" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/LIB" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/RVMSIS" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/MESH_LIB" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
HAL/LED.o: e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/LED.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DBLE_BUFF_MAX_LEN=251 -DLIB_FLASH_BASE_ADDRESSS=0x0004E000 -DCH59xBLE_ROM -DBLE_MEMHEAP_SIZE=5632 -DHAL_KEY=1 -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Startup" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/APP/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Profile/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/StdPeriphDriver/inc" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Ld" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/LIB" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/RVMSIS" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/MESH_LIB" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
HAL/MCU.o: e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/MCU.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DBLE_BUFF_MAX_LEN=251 -DLIB_FLASH_BASE_ADDRESSS=0x0004E000 -DCH59xBLE_ROM -DBLE_MEMHEAP_SIZE=5632 -DHAL_KEY=1 -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Startup" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/APP/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Profile/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/StdPeriphDriver/inc" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Ld" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/LIB" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/RVMSIS" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/MESH_LIB" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
HAL/RTC.o: e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/RTC.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DBLE_BUFF_MAX_LEN=251 -DLIB_FLASH_BASE_ADDRESSS=0x0004E000 -DCH59xBLE_ROM -DBLE_MEMHEAP_SIZE=5632 -DHAL_KEY=1 -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Startup" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/APP/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Profile/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/StdPeriphDriver/inc" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Ld" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/LIB" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/RVMSIS" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/MESH_LIB" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
HAL/SLEEP.o: e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/SLEEP.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DBLE_BUFF_MAX_LEN=251 -DLIB_FLASH_BASE_ADDRESSS=0x0004E000 -DCH59xBLE_ROM -DBLE_MEMHEAP_SIZE=5632 -DHAL_KEY=1 -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Startup" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/APP/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Profile/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/StdPeriphDriver/inc" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/HAL/include" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/BLE_Mesh_Project/Ld" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/LIB" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/SRC/RVMSIS" -I"e:/wavemail/防跌/V1有人无人传感器/core_ch592_mesh/BLE/MESH/MESH_LIB" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

