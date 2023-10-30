################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dilithium5-AES-R/src/aes256ctr.c \
../dilithium5-AES-R/src/dilithium_keys.c \
../dilithium5-AES-R/src/elapsed_time.c \
../dilithium5-AES-R/src/fips202.c \
../dilithium5-AES-R/src/ntt.c \
../dilithium5-AES-R/src/packing.c \
../dilithium5-AES-R/src/poly.c \
../dilithium5-AES-R/src/polyvec.c \
../dilithium5-AES-R/src/randombytes.c \
../dilithium5-AES-R/src/reduce.c \
../dilithium5-AES-R/src/rounding.c \
../dilithium5-AES-R/src/sign.c \
../dilithium5-AES-R/src/symmetric-aes.c \
../dilithium5-AES-R/src/symmetric-aes_stm.c \
../dilithium5-AES-R/src/symmetric-shake.c \
../dilithium5-AES-R/src/time_testbench.c \
../dilithium5-AES-R/src/usb_debug.c 

OBJS += \
./dilithium5-AES-R/src/aes256ctr.o \
./dilithium5-AES-R/src/dilithium_keys.o \
./dilithium5-AES-R/src/elapsed_time.o \
./dilithium5-AES-R/src/fips202.o \
./dilithium5-AES-R/src/ntt.o \
./dilithium5-AES-R/src/packing.o \
./dilithium5-AES-R/src/poly.o \
./dilithium5-AES-R/src/polyvec.o \
./dilithium5-AES-R/src/randombytes.o \
./dilithium5-AES-R/src/reduce.o \
./dilithium5-AES-R/src/rounding.o \
./dilithium5-AES-R/src/sign.o \
./dilithium5-AES-R/src/symmetric-aes.o \
./dilithium5-AES-R/src/symmetric-aes_stm.o \
./dilithium5-AES-R/src/symmetric-shake.o \
./dilithium5-AES-R/src/time_testbench.o \
./dilithium5-AES-R/src/usb_debug.o 

C_DEPS += \
./dilithium5-AES-R/src/aes256ctr.d \
./dilithium5-AES-R/src/dilithium_keys.d \
./dilithium5-AES-R/src/elapsed_time.d \
./dilithium5-AES-R/src/fips202.d \
./dilithium5-AES-R/src/ntt.d \
./dilithium5-AES-R/src/packing.d \
./dilithium5-AES-R/src/poly.d \
./dilithium5-AES-R/src/polyvec.d \
./dilithium5-AES-R/src/randombytes.d \
./dilithium5-AES-R/src/reduce.d \
./dilithium5-AES-R/src/rounding.d \
./dilithium5-AES-R/src/sign.d \
./dilithium5-AES-R/src/symmetric-aes.d \
./dilithium5-AES-R/src/symmetric-aes_stm.d \
./dilithium5-AES-R/src/symmetric-shake.d \
./dilithium5-AES-R/src/time_testbench.d \
./dilithium5-AES-R/src/usb_debug.d 


# Each subdirectory must supply rules for building sources it contributes
dilithium5-AES-R/src/%.o dilithium5-AES-R/src/%.su: ../dilithium5-AES-R/src/%.c dilithium5-AES-R/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB55xx -c -I../USB_Device/App -I../USB_Device/Target -I../Core/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -I"F:/MasterDiploma2/src_STM32/USB_Dongle_only/dilithium5-AES-R" -I"F:/MasterDiploma2/src_STM32/USB_Dongle_only/dilithium5-AES-R/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-dilithium5-2d-AES-2d-R-2f-src

clean-dilithium5-2d-AES-2d-R-2f-src:
	-$(RM) ./dilithium5-AES-R/src/aes256ctr.d ./dilithium5-AES-R/src/aes256ctr.o ./dilithium5-AES-R/src/aes256ctr.su ./dilithium5-AES-R/src/dilithium_keys.d ./dilithium5-AES-R/src/dilithium_keys.o ./dilithium5-AES-R/src/dilithium_keys.su ./dilithium5-AES-R/src/elapsed_time.d ./dilithium5-AES-R/src/elapsed_time.o ./dilithium5-AES-R/src/elapsed_time.su ./dilithium5-AES-R/src/fips202.d ./dilithium5-AES-R/src/fips202.o ./dilithium5-AES-R/src/fips202.su ./dilithium5-AES-R/src/ntt.d ./dilithium5-AES-R/src/ntt.o ./dilithium5-AES-R/src/ntt.su ./dilithium5-AES-R/src/packing.d ./dilithium5-AES-R/src/packing.o ./dilithium5-AES-R/src/packing.su ./dilithium5-AES-R/src/poly.d ./dilithium5-AES-R/src/poly.o ./dilithium5-AES-R/src/poly.su ./dilithium5-AES-R/src/polyvec.d ./dilithium5-AES-R/src/polyvec.o ./dilithium5-AES-R/src/polyvec.su ./dilithium5-AES-R/src/randombytes.d ./dilithium5-AES-R/src/randombytes.o ./dilithium5-AES-R/src/randombytes.su ./dilithium5-AES-R/src/reduce.d ./dilithium5-AES-R/src/reduce.o ./dilithium5-AES-R/src/reduce.su ./dilithium5-AES-R/src/rounding.d ./dilithium5-AES-R/src/rounding.o ./dilithium5-AES-R/src/rounding.su ./dilithium5-AES-R/src/sign.d ./dilithium5-AES-R/src/sign.o ./dilithium5-AES-R/src/sign.su ./dilithium5-AES-R/src/symmetric-aes.d ./dilithium5-AES-R/src/symmetric-aes.o ./dilithium5-AES-R/src/symmetric-aes.su ./dilithium5-AES-R/src/symmetric-aes_stm.d ./dilithium5-AES-R/src/symmetric-aes_stm.o ./dilithium5-AES-R/src/symmetric-aes_stm.su ./dilithium5-AES-R/src/symmetric-shake.d ./dilithium5-AES-R/src/symmetric-shake.o ./dilithium5-AES-R/src/symmetric-shake.su ./dilithium5-AES-R/src/time_testbench.d ./dilithium5-AES-R/src/time_testbench.o ./dilithium5-AES-R/src/time_testbench.su ./dilithium5-AES-R/src/usb_debug.d ./dilithium5-AES-R/src/usb_debug.o ./dilithium5-AES-R/src/usb_debug.su

.PHONY: clean-dilithium5-2d-AES-2d-R-2f-src

