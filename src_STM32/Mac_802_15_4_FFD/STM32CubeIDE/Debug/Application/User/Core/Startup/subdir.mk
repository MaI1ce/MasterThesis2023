################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Application/User/Core/Startup/startup_stm32wb55rgvx.s 

OBJS += \
./Application/User/Core/Startup/startup_stm32wb55rgvx.o 

S_DEPS += \
./Application/User/Core/Startup/startup_stm32wb55rgvx.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/Core/Startup/%.o: ../Application/User/Core/Startup/%.s Application/User/Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Application-2f-User-2f-Core-2f-Startup

clean-Application-2f-User-2f-Core-2f-Startup:
	-$(RM) ./Application/User/Core/Startup/startup_stm32wb55rgvx.d ./Application/User/Core/Startup/startup_stm32wb55rgvx.o

.PHONY: clean-Application-2f-User-2f-Core-2f-Startup

