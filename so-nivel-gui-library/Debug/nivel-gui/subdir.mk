################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../nivel-gui/nivel.c 

OBJS += \
./nivel-gui/nivel.o 

C_DEPS += \
./nivel-gui/nivel.d 


# Each subdirectory must supply rules for building sources it contributes
nivel-gui/%.o: ../nivel-gui/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/src" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/nivel-gui" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/tad_items" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


