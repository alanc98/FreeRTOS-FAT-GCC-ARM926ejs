# Copyright 2013, Jernej Kovacic
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#
# Type "make help" for more details.
#

TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCOPY = $(TOOLCHAIN)objcopy
AR = $(TOOLCHAIN)ar

# GCC flags
CFLAG = -c
OFLAG = -o
INCLUDEFLAG = -I
CPUFLAG = -mcpu=arm926ej-s  
WFLAG = -Wall -Wextra -Werror
# CFLAGS = $(CPUFLAG) $(WFLAG) --specs=nano.specs
CFLAGS = $(CPUFLAG) $(WFLAG) 

# Additional C compiler flags to produce debugging symbols
DEB_FLAG = -g -DDEBUG

# Compiler/target path in FreeRTOS/Source/portable
PORT_COMP_TARG = GCC/ARM926EJ-S/

# use these on the $(LD) line, as needed:
LIBM = ${shell ${CC} ${CFLAGS} --print-file-name=libm.a}
LIBC = ${shell ${CC} ${CFLAGS} --print-file-name=libc.a}
LIBGCC = ${shell ${CC} ${CFLAGS} --print-libgcc-file-name}

# Intermediate directory for all *.o and other files:
OBJDIR = ./

SRCDIR = ..

# FreeRTOS source base directory
FREERTOS_SRC = $(SRCDIR)/FreeRTOS/Source/

# Directory with memory management source files
FREERTOS_MEMMANG_SRC = $(FREERTOS_SRC)portable/MemMang/

# Directory with platform specific source files
FREERTOS_PORT_SRC = $(FREERTOS_SRC)portable/$(PORT_COMP_TARG)

# file system src
FREERTOS_FAT_SRC = $(SRCDIR)/FreeRTOS-Plus-FAT/

# Directory with HW drivers' source files
DRIVERS_SRC = $(SRCDIR)/drivers/

# Directory with startup source (and header) files. Also linker script
STARTUP_SRC = $(SRCDIR)/startup/

# Directory with demo specific source (and header) files
APP_SRC = $(SRCDIR)/app/

# Directory with demo specific source (and header) files
LIB_SRC = $(SRCDIR)/lib/

# Object files to be linked into an application
# Due to a large number, the .o files are arranged into logical groups:

FREERTOS_OBJS = queue.o list.o tasks.o
# The following o. files are only necessary if
# certain options are enabled in FreeRTOSConfig.h
FREERTOS_OBJS += timers.o
FREERTOS_OBJS += event_groups.o
# FREERTOS_OBJS += croutine.o
# FREERTOS_OBJS += stream_buffer.o

# Only one memory management .o file must be uncommented!
#FREERTOS_MEMMANG_OBJS = heap_1.o
#FREERTOS_MEMMANG_OBJS = heap_2.o
FREERTOS_MEMMANG_OBJS = heap_3.o
#FREERTOS_MEMMANG_OBJS = heap_4.o
#FREERTOS_MEMMANG_OBJS = heap_5.o

FREERTOS_PORT_OBJS = port.o portISR.o

FREERTOS_FAT_OBJS = ff_crc.o ff_dir.o ff_error.o ff_fat.o ff_file.o ff_format.o ff_ioman.o 
FREERTOS_FAT_OBJS += ff_locking.o ff_memory.o ff_stdio.o ff_string.o ff_sys.o ff_time.o  

STARTUP_ASM_OBJ = startup.o
STARTUP_OBJS = init.o main.o print.o receive.o syscalls.o ramdisk.o untar.o
DRIVERS_OBJS = timer.o interrupt.o uart.o ff_ramdisk.o
APP_OBJS = app_init.o

# nostdlib.o must be commented out if standard lib is going to be linked!
# APP_OBJS += nostdlib.o

# All object files specified above are prefixed the intermediate directory
OBJS = $(addprefix $(OBJDIR), $(STARTUP_ASM_OBJ) $(STARTUP_OBJS) $(FREERTOS_OBJS) $(FREERTOS_MEMMANG_OBJS) $(FREERTOS_PORT_OBJS) $(FREERTOS_FAT_OBJS) $(DRIVERS_OBJS) $(APP_OBJS) tarfile.o)

# Definition of the linker script and final targets
LINKER_SCRIPT = $(addprefix $(STARTUP_SRC), qemu.ld)
ELF_IMAGE = image.elf
TARGET = image.bin

# Include paths to be passed to $(CC) where necessary
INC_FREERTOS = $(FREERTOS_SRC)include/
INC_FREERTOS_FAT = $(FREERTOS_FAT_SRC)include/
INC_DRIVERS = $(DRIVERS_SRC)

# Complete include flags to be passed to $(CC) where necessary
INC_FLAGS = $(INCLUDEFLAG)$(INC_FREERTOS) $(INCLUDEFLAG)$(STARTUP_SRC) $(INCLUDEFLAG)$(APP_SRC) $(INCLUDEFLAG)$(FREERTOS_PORT_SRC) $(INCLUDEFLAG)$(LIB_SRC) $(INCLUDEFLAG)$(INC_FREERTOS_FAT)
INC_FLAG_DRIVERS = $(INCLUDEFLAG)$(INC_DRIVERS)

# Dependency on HW specific settings
DEP_BSP = $(INC_DRIVERS)bsp.h

#
# Make rules:
#

# printlibs::
#	echo $(LIBC)
#	echo $(LIBM)
#	echo $(LIBGCC)
#

all : $(TARGET)

rebuild : clean all

$(TARGET) : $(OBJDIR) $(ELF_IMAGE)
	$(OBJCOPY) -O binary $(word 2,$^) $@

$(OBJDIR) :
	mkdir -p $@

$(ELF_IMAGE) : $(OBJS) $(LINKER_SCRIPT)
	$(LD) -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@  $(LIBC) $(LIBM) $(LIBGCC) 
#	$(LD) -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@  $(LIBDIR)/libc.a $(LIBDIR)/libm.a $(GCCLIBDIR)/libgcc.a 
#	$(LD) -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@  $(LIBDIR)/libc_nano.a 
#	$(LD) -nostdlib -L $(OBJDIR) -T $(LINKER_SCRIPT) $(OBJS) $(OFLAG) $@ 

debug : _debug_flags all

debug_rebuild : _debug_flags rebuild

_debug_flags :
	$(eval CFLAGS += $(DEB_FLAG))

# Startup code, implemented in assembler

$(OBJDIR)startup.o : $(STARTUP_SRC)startup.s
	$(AS) $(CPUFLAG) $< $(OFLAG) $@

# FreeRTOS core

$(OBJDIR)queue.o : $(FREERTOS_SRC)queue.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)list.o : $(FREERTOS_SRC)list.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)tasks.o : $(FREERTOS_SRC)tasks.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)timers.o : $(FREERTOS_SRC)timers.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)croutine.o : $(FREERTOS_SRC)croutine.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)event_groups.o : $(FREERTOS_SRC)event_groups.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

# HW specific part, in FreeRTOS/Source/portable/$(PORT_COMP_TARGET)

$(OBJDIR)port.o : $(FREERTOS_PORT_SRC)port.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)portISR.o : $(FREERTOS_PORT_SRC)portISR.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

# Rules for all MemMang implementations are provided
# Only one of these object files must be linked to the final target

$(OBJDIR)heap_1.o : $(FREERTOS_MEMMANG_SRC)heap_1.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_2.o : $(FREERTOS_MEMMANG_SRC)heap_2.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_3.o : $(FREERTOS_MEMMANG_SRC)heap_3.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_4.o : $(FREERTOS_MEMMANG_SRC)heap_4.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)heap_5.o : $(FREERTOS_MEMMANG_SRC)heap_5.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

# FreeRTOS FAT 
$(OBJDIR)ff_crc.o : $(FREERTOS_FAT_SRC)ff_crc.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

# $(OBJDIR)ff_dev_support.o : $(FREERTOS_FAT_SRC)ff_dev_support.c
#	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_dir.o : $(FREERTOS_FAT_SRC)ff_dir.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_error.o : $(FREERTOS_FAT_SRC)ff_error.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_fat.o : $(FREERTOS_FAT_SRC)ff_fat.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_file.o : $(FREERTOS_FAT_SRC)ff_file.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_format.o : $(FREERTOS_FAT_SRC)ff_format.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_ioman.o : $(FREERTOS_FAT_SRC)ff_ioman.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_locking.o : $(FREERTOS_FAT_SRC)ff_locking.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_memory.o : $(FREERTOS_FAT_SRC)ff_memory.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_stdio.o : $(FREERTOS_FAT_SRC)ff_stdio.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_string.o : $(FREERTOS_FAT_SRC)ff_string.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_sys.o : $(FREERTOS_FAT_SRC)ff_sys.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)ff_time.o : $(FREERTOS_FAT_SRC)ff_time.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

# Drivers

$(OBJDIR)timer.o : $(DRIVERS_SRC)timer.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)interrupt.o : $(DRIVERS_SRC)interrupt.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)uart.o : $(DRIVERS_SRC)uart.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)ff_ramdisk.o : $(DRIVERS_SRC)ff_ramdisk.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $(INC_FLAGS) $< $(OFLAG) $@

# Startup files

$(OBJDIR)main.o : $(STARTUP_SRC)main.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

$(OBJDIR)init.o : $(STARTUP_SRC)init.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)ramdisk.o : $(STARTUP_SRC)ramdisk.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

# App files

$(OBJDIR)app_init.o : $(APP_SRC)app_init.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $< $(OFLAG) $@

# Lib functions
$(OBJDIR)print.o : $(LIB_SRC)print.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)receive.o : $(LIB_SRC)receive.c $(DEP_BSP)
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)nostdlib.o : $(LIB_SRC)nostdlib.c
	$(CC) $(CFLAG) $(CFLAGS) $< $(OFLAG) $@

$(OBJDIR)syscalls.o : $(LIB_SRC)syscalls.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

$(OBJDIR)untar.o : $(LIB_SRC)untar.c
	$(CC) $(CFLAG) $(CFLAGS) $(INC_FLAGS) $(INC_FLAG_DRIVERS) $< $(OFLAG) $@

# Tar file
$(OBJDIR)tarfile.o ::
	cd ../rootfs; tar cf ../build/tarfile $(shell ls ../rootfs)
	$(LD) -r --noinhibit-exec -o $(OBJDIR)tarfile.o -b binary tarfile

# Cleanup directives:

clean_obj :
	$(RM) -f *.o 

clean_intermediate : clean_obj
	$(RM) *.elf
	$(RM) *.img

clean : clean_intermediate
	$(RM) *.bin
	$(RM) tarfile

# Short help instructions:

help :
	@echo
	@echo Valid targets:
	@echo - all: builds missing dependencies and creates the target image \'$(TARGET)\'.
	@echo - rebuild: rebuilds all dependencies and creates the target image \'$(TARGET)\'.
	@echo - debug: same as \'all\', also includes debugging symbols to \'$(ELF_IMAGE)\'.
	@echo - debug_rebuild: same as \'rebuild\', also includes debugging symbols to \'$(ELF_IMAGE)\'.
	@echo - clean_obj: deletes all object files, only keeps \'$(ELF_IMAGE)\' and \'$(TARGET)\'.
	@echo - clean_intermediate: deletes all intermediate binaries, only keeps the target image \'$(TARGET)\'.
	@echo - clean: deletes all intermediate binaries, incl. the target image \'$(TARGET)\'.
	@echo - help: displays these help instructions.
	@echo

.PHONY : all rebuild clean clean_obj clean_intermediate debug debug_rebuild _debug_flags help
