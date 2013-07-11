################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../serial.c 

OBJS += \
./serial.o 

C_DEPS += \
./serial.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/src" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/nivel-gui" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/tad_items" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/memoria" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library" -I"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


