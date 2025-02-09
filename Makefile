# (C) 2024, Cornell University
# All rights reserved.

QEMU        = qemu-system-riscv32
RISCV_CC    = riscv-none-elf-gcc
OBJDUMP     = riscv-none-elf-objdump
OBJCOPY     = riscv-none-elf-objcopy

LDFLAGS     = -nostdlib -lc -lgcc
CFLAGS      = -march=rv32ima_zicsr -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

all:
	@echo "$(YELLOW)-------- Compile Multi-threading --------$(END)"
	$(RISCV_CC) $(CFLAGS) thread.s context.s thread.c -Tthread.lds $(LDFLAGS) -o thread.elf
	$(OBJDUMP) $(DEBUG_FLAGS) thread.elf > thread.lst
	$(OBJCOPY) -O binary thread.elf thread.bin

qemu: all
	@echo "$(YELLOW)-------- Run Multi-threading on QEMU --------$(END)"
	$(QEMU) -nographic -machine virt -smp 1 -bios thread.bin

clean:
	rm -f hello.bin hello.lst hello.elf
	rm -f thread.bin thread.lst thread.elf
	rm -rf build earth/kernel_entry.lds tools/mkfs tools/mkrom tools/qemu/egos.bin tools/disk.img tools/bootROM.bin

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
