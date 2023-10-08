################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/STM32_WPAN/App/app_ffd_mac_802_15_4.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/STM32_WPAN/App/app_ffd_mac_802_15_4_process.c 

OBJS += \
./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4.o \
./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4_process.o 

C_DEPS += \
./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4.d \
./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4_process.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/STM32_WPAN/App/app_ffd_mac_802_15_4.c Application/User/STM32_WPAN/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4_process.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/STM32_WPAN/App/app_ffd_mac_802_15_4_process.c Application/User/STM32_WPAN/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User-2f-STM32_WPAN-2f-App

clean-Application-2f-User-2f-STM32_WPAN-2f-App:
	-$(RM) ./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4.d ./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4.o ./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4.su ./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4_process.d ./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4_process.o ./Application/User/STM32_WPAN/App/app_ffd_mac_802_15_4_process.su

.PHONY: clean-Application-2f-User-2f-STM32_WPAN-2f-App

