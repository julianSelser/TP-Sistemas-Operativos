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
all: Nivel

dependents:
	-cd /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug && $(MAKE) clean all
	-cd /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/Debug && $(MAKE) clean all
	-cd /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/tad_items/Debug && $(MAKE) clean all
	-cd /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library/Debug && $(MAKE) clean all

# Tool invocations
Nivel: $(OBJS) $(USER_OBJS) /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug/libso-commons-library.so /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/Debug/libso-nivel-gui-library.so /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/tad_items/Debug/libtad_items.so /home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library/Debug/libso-serial-library.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library/Debug" -L"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/tad_items/Debug" -L"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/Debug" -L"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug" -o "Nivel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Nivel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug/libso-commons-library.so:
/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/Debug/libso-nivel-gui-library.so:
/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/tad_items/Debug/libtad_items.so:
/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library/Debug/libso-serial-library.so:

-include ../makefile.targets
