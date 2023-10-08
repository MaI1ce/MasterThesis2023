################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/mac_802_15_4/core/src/mac_802_15_4_core_wb.c 

OBJS += \
./Middlewares/STM32_WPAN/mac_802_15_4/src/mac_802_15_4_core_wb.o 

C_DEPS += \
./Middlewares/STM32_WPAN/mac_802_15_4/src/mac_802_15_4_core_wb.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/STM32_WPAN/mac_802_15_4/src/mac_802_15_4_core_wb.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/mac_802_15_4/core/src/mac_802_15_4_core_wb.c Middlewares/STM32_WPAN/mac_802_15_4/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-STM32_WPAN-2f-mac_802_15_4-2f-src

clean-Middlewares-2f-STM32_WPAN-2f-mac_802_15_4-2f-src:
	-$(RM) ./Middlewares/STM32_WPAN/mac_802_15_4/src/mac_802_15_4_core_wb.d ./Middlewares/STM32_WPAN/mac_802_15_4/src/mac_802_15_4_core_wb.o ./Middlewares/STM32_WPAN/mac_802_15_4/src/mac_802_15_4_core_wb.su

.PHONY: clean-Middlewares-2f-STM32_WPAN-2f-mac_802_15_4-2f-src

