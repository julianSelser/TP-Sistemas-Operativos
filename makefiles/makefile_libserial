################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libso-serial-library.so

dependents:
	-cd /home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug && $(MAKE) all

# Tool invocations
libso-serial-library.so: $(OBJS) $(USER_OBJS) /home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug/libso-commons-library.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug" -shared -o "libso-serial-library.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libso-serial-library.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/julian/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug/libso-commons-library.so:

-include ../makefile.targets
