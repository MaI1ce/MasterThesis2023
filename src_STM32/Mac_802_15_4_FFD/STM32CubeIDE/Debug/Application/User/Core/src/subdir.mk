################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/app_entry.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/hw_uart.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/main.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/stm32_lpm_if.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/stm32wbxx_it.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/stm_logging.c \
../Application/User/Core/src/syscalls.c \
../Application/User/Core/src/sysmem.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/system_stm32wbxx.c 

OBJS += \
./Application/User/Core/src/app_entry.o \
./Application/User/Core/src/hw_uart.o \
./Application/User/Core/src/main.o \
./Application/User/Core/src/stm32_lpm_if.o \
./Application/User/Core/src/stm32wbxx_it.o \
./Application/User/Core/src/stm_logging.o \
./Application/User/Core/src/syscalls.o \
./Application/User/Core/src/sysmem.o \
./Application/User/Core/src/system_stm32wbxx.o 

C_DEPS += \
./Application/User/Core/src/app_entry.d \
./Application/User/Core/src/hw_uart.d \
./Application/User/Core/src/main.d \
./Application/User/Core/src/stm32_lpm_if.d \
./Application/User/Core/src/stm32wbxx_it.d \
./Application/User/Core/src/stm_logging.d \
./Application/User/Core/src/syscalls.d \
./Application/User/Core/src/sysmem.d \
./Application/User/Core/src/system_stm32wbxx.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Core/src/app_entry.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/app_entry.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/hw_uart.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/hw_uart.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/main.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/main.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/stm32_lpm_if.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/stm32_lpm_if.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/stm32wbxx_it.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/stm32wbxx_it.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/stm_logging.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/stm_logging.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/%.o Application/User/Core/src/%.su: ../Application/User/Core/src/%.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/Core/src/system_stm32wbxx.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Core/Src/system_stm32wbxx.c Application/User/Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User-2f-Core-2f-src

clean-Application-2f-User-2f-Core-2f-src:
	-$(RM) ./Application/User/Core/src/app_entry.d ./Application/User/Core/src/app_entry.o ./Application/User/Core/src/app_entry.su ./Application/User/Core/src/hw_uart.d ./Application/User/Core/src/hw_uart.o ./Application/User/Core/src/hw_uart.su ./Application/User/Core/src/main.d ./Application/User/Core/src/main.o ./Application/User/Core/src/main.su ./Application/User/Core/src/stm32_lpm_if.d ./Application/User/Core/src/stm32_lpm_if.o ./Application/User/Core/src/stm32_lpm_if.su ./Application/User/Core/src/stm32wbxx_it.d ./Application/User/Core/src/stm32wbxx_it.o ./Application/User/Core/src/stm32wbxx_it.su ./Application/User/Core/src/stm_logging.d ./Application/User/Core/src/stm_logging.o ./Application/User/Core/src/stm_logging.su ./Application/User/Core/src/syscalls.d ./Application/User/Core/src/syscalls.o ./Application/User/Core/src/syscalls.su ./Application/User/Core/src/sysmem.d ./Application/User/Core/src/sysmem.o ./Application/User/Core/src/sysmem.su ./Application/User/Core/src/system_stm32wbxx.d ./Application/User/Core/src/system_stm32wbxx.o ./Application/User/Core/src/system_stm32wbxx.su

.PHONY: clean-Application-2f-User-2f-Core-2f-src

