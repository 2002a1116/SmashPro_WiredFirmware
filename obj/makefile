################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
CH32V103C8T6_USB.hex \

SECONDARY_LIST += \
CH32V103C8T6_USB.lst \

SECONDARY_SIZE += \
CH32V103C8T6_USB.siz \


# 所有目标
all: CH32V103C8T6_USB.elf secondary-outputs

# 工具调用
CH32V103C8T6_USB.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-elf-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T "D:\WCH\CH32V103C8T6_USB\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"CH32V103C8T6_USB.map" --specs=nano.specs --specs=nosys.specs  -o "CH32V103C8T6_USB.elf" $(OBJS) $(USER_OBJS) $(LIBS) -lprintfloat
	@	@
CH32V103C8T6_USB.hex: CH32V103C8T6_USB.elf
	@	riscv-none-elf-objcopy -O ihex "CH32V103C8T6_USB.elf"  "CH32V103C8T6_USB.hex"
	@	@
CH32V103C8T6_USB.lst: CH32V103C8T6_USB.elf
	@	riscv-none-elf-objdump --all-headers --demangle --disassemble "CH32V103C8T6_USB.elf" > "CH32V103C8T6_USB.lst"
	@	@
CH32V103C8T6_USB.siz: CH32V103C8T6_USB.elf
	@	riscv-none-elf-size --format=berkeley "CH32V103C8T6_USB.elf"
	@	@
# 其他目标
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) CH32V103C8T6_USB.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
