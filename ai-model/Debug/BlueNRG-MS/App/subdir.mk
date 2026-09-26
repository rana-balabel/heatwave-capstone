################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BlueNRG-MS/App/app_bluenrg_ms.c \
../BlueNRG-MS/App/heatstroke_service.c 

OBJS += \
./BlueNRG-MS/App/app_bluenrg_ms.o \
./BlueNRG-MS/App/heatstroke_service.o 

C_DEPS += \
./BlueNRG-MS/App/app_bluenrg_ms.d \
./BlueNRG-MS/App/heatstroke_service.d 


# Each subdirectory must supply rules for building sources it contributes
BlueNRG-MS/App/%.o BlueNRG-MS/App/%.su BlueNRG-MS/App/%.cyclo: ../BlueNRG-MS/App/%.c BlueNRG-MS/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../BlueNRG-MS/App -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../BlueNRG-MS/Target -I../Middlewares/ST/BlueNRG-MS/utils -I../Middlewares/ST/BlueNRG-MS/includes -I../Middlewares/ST/BlueNRG-MS/hci/hci_tl_patterns/Basic -I../Middlewares/ST/AI/Inc -I../X-CUBE-AI/App -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BlueNRG-2d-MS-2f-App

clean-BlueNRG-2d-MS-2f-App:
	-$(RM) ./BlueNRG-MS/App/app_bluenrg_ms.cyclo ./BlueNRG-MS/App/app_bluenrg_ms.d ./BlueNRG-MS/App/app_bluenrg_ms.o ./BlueNRG-MS/App/app_bluenrg_ms.su ./BlueNRG-MS/App/heatstroke_service.cyclo ./BlueNRG-MS/App/heatstroke_service.d ./BlueNRG-MS/App/heatstroke_service.o ./BlueNRG-MS/App/heatstroke_service.su

.PHONY: clean-BlueNRG-2d-MS-2f-App

