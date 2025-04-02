################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v10x_it.c \
../User/ch32v10x_usbfs_device.c \
../User/conf.c \
../User/flash.c \
../User/global_api.c \
../User/gpio_adc.c \
../User/gpio_digit.c \
../User/hd_rumble.c \
../User/hd_rumble2.c \
../User/hd_rumble_high_accuracy.c \
../User/i2c.c \
../User/imu.c \
../User/imu_quaternion.c \
../User/main.c \
../User/ns_com.c \
../User/ns_com_mux.c \
../User/pwr.c \
../User/ring_buffer.c \
../User/spi.c \
../User/system_ch32v10x.c \
../User/tick.c \
../User/uart_com.c \
../User/usb_desc.c \
../User/usbd_compatibility_hid.c 

OBJS += \
./User/ch32v10x_it.o \
./User/ch32v10x_usbfs_device.o \
./User/conf.o \
./User/flash.o \
./User/global_api.o \
./User/gpio_adc.o \
./User/gpio_digit.o \
./User/hd_rumble.o \
./User/hd_rumble2.o \
./User/hd_rumble_high_accuracy.o \
./User/i2c.o \
./User/imu.o \
./User/imu_quaternion.o \
./User/main.o \
./User/ns_com.o \
./User/ns_com_mux.o \
./User/pwr.o \
./User/ring_buffer.o \
./User/spi.o \
./User/system_ch32v10x.o \
./User/tick.o \
./User/uart_com.o \
./User/usb_desc.o \
./User/usbd_compatibility_hid.o 

C_DEPS += \
./User/ch32v10x_it.d \
./User/ch32v10x_usbfs_device.d \
./User/conf.d \
./User/flash.d \
./User/global_api.d \
./User/gpio_adc.d \
./User/gpio_digit.d \
./User/hd_rumble.d \
./User/hd_rumble2.d \
./User/hd_rumble_high_accuracy.d \
./User/i2c.d \
./User/imu.d \
./User/imu_quaternion.d \
./User/main.d \
./User/ns_com.d \
./User/ns_com_mux.d \
./User/pwr.d \
./User/ring_buffer.d \
./User/spi.d \
./User/system_ch32v10x.d \
./User/tick.d \
./User/uart_com.d \
./User/usb_desc.d \
./User/usbd_compatibility_hid.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-elf-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\WCH\CH32V103C8T6_USB\Debug" -I"D:\WCH\CH32V103C8T6_USB\Core" -I"D:\WCH\CH32V103C8T6_USB\User" -I"D:\WCH\CH32V103C8T6_USB\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

