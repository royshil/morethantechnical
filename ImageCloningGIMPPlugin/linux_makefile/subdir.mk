################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ImageEditingUtils.cpp \
../plugin.cpp 

OBJS += \
./ImageEditingUtils.o \
./plugin.o 

CPP_DEPS += \
./ImageEditingUtils.d \
./plugin.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/gimp-2.0 -I/usr/include/gmm -I/usr/lib/glib-2.0/include/ -I/usr/include/glib-2.0 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


