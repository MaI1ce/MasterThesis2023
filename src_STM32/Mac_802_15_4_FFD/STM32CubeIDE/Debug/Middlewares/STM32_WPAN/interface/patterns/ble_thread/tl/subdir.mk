################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.c 

OBJS += \
./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.o \
./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.o \
./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.o 

C_DEPS += \
./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.d \
./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.d \
./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.c Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.c Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.c Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-STM32_WPAN-2f-interface-2f-patterns-2f-ble_thread-2f-tl

clean-Middlewares-2f-STM32_WPAN-2f-interface-2f-patterns-2f-ble_thread-2f-tl:
	-$(RM) ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.d ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.o ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl.su ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.d ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.o ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/shci_tl_if.su ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.d ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.o ./Middlewares/STM32_WPAN/interface/patterns/ble_thread/tl/tl_mbox.su

.PHONY: clean-Middlewares-2f-STM32_WPAN-2f-interface-2f-patterns-2f-ble_thread-2f-tl

