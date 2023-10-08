################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/dbg_trace.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/otp.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/stm_list.c \
F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/stm_queue.c 

OBJS += \
./Middlewares/STM32_WPAN/utilities/dbg_trace.o \
./Middlewares/STM32_WPAN/utilities/otp.o \
./Middlewares/STM32_WPAN/utilities/stm_list.o \
./Middlewares/STM32_WPAN/utilities/stm_queue.o 

C_DEPS += \
./Middlewares/STM32_WPAN/utilities/dbg_trace.d \
./Middlewares/STM32_WPAN/utilities/otp.d \
./Middlewares/STM32_WPAN/utilities/stm_list.d \
./Middlewares/STM32_WPAN/utilities/stm_queue.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/STM32_WPAN/utilities/dbg_trace.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/dbg_trace.c Middlewares/STM32_WPAN/utilities/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_WPAN/utilities/otp.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/otp.c Middlewares/STM32_WPAN/utilities/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_WPAN/utilities/stm_list.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/stm_list.c Middlewares/STM32_WPAN/utilities/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_WPAN/utilities/stm_queue.o: F:/MasterDiploma2/src_STM32/Mac_802_15_4_FFD/Middlewares/ST/STM32_WPAN/utilities/stm_queue.c Middlewares/STM32_WPAN/utilities/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_STM32WBXX_NUCLEO -DMAC_802_15_4_WB -DCORE_CM4 -DDEBUG -DSTM32WB55xx -c -I../../Core/Inc -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../../Utilities/sequencer -I../../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../../Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -I../../STM32_WPAN/App -I../../Middlewares/ST/STM32_WPAN -I../../Drivers/BSP/P-NUCLEO-WB55.Nucleo -I../../STM32_WPAN/Target -I../../Drivers/STM32WBxx_HAL_Driver/Inc -I../../Middlewares/ST/STM32_WPAN/utilities -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-STM32_WPAN-2f-utilities

clean-Middlewares-2f-STM32_WPAN-2f-utilities:
	-$(RM) ./Middlewares/STM32_WPAN/utilities/dbg_trace.d ./Middlewares/STM32_WPAN/utilities/dbg_trace.o ./Middlewares/STM32_WPAN/utilities/dbg_trace.su ./Middlewares/STM32_WPAN/utilities/otp.d ./Middlewares/STM32_WPAN/utilities/otp.o ./Middlewares/STM32_WPAN/utilities/otp.su ./Middlewares/STM32_WPAN/utilities/stm_list.d ./Middlewares/STM32_WPAN/utilities/stm_list.o ./Middlewares/STM32_WPAN/utilities/stm_list.su ./Middlewares/STM32_WPAN/utilities/stm_queue.d ./Middlewares/STM32_WPAN/utilities/stm_queue.o ./Middlewares/STM32_WPAN/utilities/stm_queue.su

.PHONY: clean-Middlewares-2f-STM32_WPAN-2f-utilities

