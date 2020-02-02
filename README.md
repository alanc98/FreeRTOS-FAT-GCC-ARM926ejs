# About
This is a modified version of the ARM FreeRTOS QEMU port by Jernej Kovacic, the original project is here:
https://github.com/jkovacic/FreeRTOS-GCC-ARM926ejs

## This is a partial list of the changes:
* Added Newlib C library support.
* Re-arranged and re-named a few directories.
* Added the FreeRTOS FAT file system.
* Added an untar feature that includes a tar file and un-tars it into the RAM disk.
* Created a startup task that iniitalizes the file system (ramdisk) and starts up the Application tasks.
* Changed the Gatekeeper UART output function/task to copy a whole string to the queue rather than just a pointer. When using newlib, the printf pointer was the same, causing the same string to be printed over.

For the build, I used the GNU Tools for Arm Embedded Processors 9-2019-q4-major release:
https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm

My FreeRTOS configuration and RAM disk uses quite a bit of RAM. I increased the stack sizes, buffers, etc. It's intended for a system with plenty of memory (like the 128M on this versatilepb QEMU target). Ultimately, I would like to use it as a reference system for a cFS FreeRTOS port (https://cfs.gsfc.nasa.gov)

Future plans include:
* Creating a cFS port
* Integrating the LWIP network stack - There is a bare metal LWIP port for this qemu target here:
https://github.com/tomx4096/baremetal-lwip
* Creating a Cmake based build system

## To build:
- cd to the 'build' directory
- You may need to adjust the compiler path in the Makefile
- type make

## To Run (same as the original project):
- make sure you have qemu-system-arm installed. I just used the package available on Ubuntu 18.04
- run the start_qemu.sh script

## To Stop running (same as the original project):
- run the stop_qemu.sh script

## License Info:
See the license information at the end of this file. The original C files by Jernej Kovacic are licensed under Apache 2.0.
The FreeRTOS files should be licensed under the MIT license, but some of the files are still marked as modified GPL.
Any original code by myself is licensed Apache 2.0.

## Original Readme from Jernej Kovacic:
-------------

## About
[FreeRTOS](http://www.freertos.org/) ported to [ARM Versatile Platform Baseboard](http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf),
based on the ARM926EJ-S CPU.

The current version is based on FreeRTOS 8.2.3. The port will be regularly
updated with newer versions of FreeRTOS when they are released.

The port is still at an early development stage and includes only very basic
demo tasks. More complex tasks will be included in the future.

## Prerequisites
* _Sourcery CodeBench Lite Edition for ARM EABI_ toolchain (now owned by Mentor Graphics),
based on GCC. See comments in _setenv.sh_ for more details about download and installation.
* _GNU Make_
* _Qemu_ (version 1.3 or newer, older versions do not emulate the interrupt controller properly!)

## Build
A convenience Bash script _setenv.sh_ is provided to set paths to toolchain's commands
and libraries. You may edit it and adjust the paths according to your setup. To set up
the necessary paths, simply type:

`. ./setenv.sh`

If you wish to run the image anywhere else except in Qemu, you will probably have to
edit the linker script _qemu.ld_ and adjust the startup address properly.

To build the image with the test application, just run _make_ or _make rebuild_.
If the build process is successful, the image file _image.bin_ will be ready to boot.

# Run
To run the target image in Qemu, enter the following command:

`qemu-system-arm -M versatilepb -nographic -m 128 -kernel image.bin`

A convenience Bash script _start\_qemu.sh_ is provided. If necessary, you may
edit it and adjust paths to Qemu and/or target image.

The demo application will run infinitely so it must be stopped manually by
"killing" the instance of Qemu (an "equivalent" to switching off the board).
A convenience Bash script _stop\_qemu.sh_ (it must be run in another shell)
is provided to automate the process. Note that it may not work properly if
multiple instances of _qemu-system-arm_ are running.

For more details, see extensive comments in both scripts.

## License
All source and header files in FreeRTOS/ and its subdirectiories are licensed under
the [modified GPL license](http://www.freertos.org/license.txt).
All other files that are not derived from the FreeRTOS source distribution are licensed
under the [Apache 2.0 license](http://www.apache.org/licenses/LICENSE-2.0).

For the avoidance of any doubt refer to the comment included at the top of each source and
header file for license and copyright information.
