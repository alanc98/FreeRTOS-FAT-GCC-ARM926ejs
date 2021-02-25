/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file
 * A simple demo application.
 *
 * @author Jernej Kovacic
 */

#include <stddef.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_ramdisk.h"

#include "app_config.h"
#include "print.h"
#include "receive.h"

#include "untar.h"

/*
 * This diagnostic pragma will suppress the -Wmain warning,
 * raised when main() does not return an int
 * (which is perfectly OK in bare metal programming!).
 *
 * More details about the GCC diagnostic pragmas:
 * https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#pragma GCC diagnostic ignored "-Wmain"

/* function to create the RAM disk */
extern void CreateRamDisk( void );

/* FreeRTOS error function */
void FreeRTOS_Error(const portCHAR* msg);

/*
** Binary tar file address and size
*/
extern unsigned int _binary_tarfile_size;
extern unsigned int _binary_tarfile_start;

/*
** Startup task 
*/
void startupTask(void* pvParameters)
{
    int     status;
    int32_t lResult;

    printf("--- startupTask starting! ---\n");

    /*
    ** Create the RAM disk
    */
    printf("Creating RAM Disk\n");
    CreateRamDisk();

    /*
    ** Ensure in the root of the mount being used.
    */
    lResult = ff_chdir( "/ram" );
    printf("Status of ff_chdir = %ld\n",lResult);

    /*
    ** Unpack tar file
    */
    printf("Address of tarfile data = 0x%X\n",(unsigned int)&_binary_tarfile_start);
    printf("Size of tarfile data = %lu\n",(unsigned long) &_binary_tarfile_size);
    printf("Size of tarfile data (hex)= 0x%lX\n",(unsigned long) &_binary_tarfile_size);

    printf("Caling Untar_FromMemory\n"); 
    status = Untar_FromMemory(
                (unsigned char *)(&_binary_tarfile_start),
                (unsigned long)&_binary_tarfile_size);

    printf("Utar_FromMemory returned - status = %d\n",status); 

    /* 
    ** For this demo, create two tasks: 
    */
    printf("Call OSAL Entry point!\n");

    for(;;)
    {
       vTaskDelay( 1000 / portTICK_RATE_MS );
       printf(" --- In startup task\n");
    }

    printf(" --- startupTask jobs complete - exiting task\n");
    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) pvParameters;

}

