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

#include "app_config.h"
#include "print.h"
#include "receive.h"

/*
 * This diagnostic pragma will suppress the -Wmain warning,
 * raised when main() does not return an int
 * (which is perfectly OK in bare metal programming!).
 *
 * More details about the GCC diagnostic pragmas:
 * https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#pragma GCC diagnostic ignored "-Wmain"

/*
** Startup task
*/
void startupTask(void* pvParameters);

/*
 * A convenience function that is called when a FreeRTOS API call fails
 * and a program cannot continue. It prints a message (if provided) and
 * ends in an infinite loop.
 */
void FreeRTOS_Error(const portCHAR* msg)
{
    if ( NULL != msg )
    {
        vDirectPrintMsg(msg);
    }
    for ( ; ; );
}

void vAssertCalled( const char *pcFile, uint32_t ulLine )
{
   volatile char *pcFileName = ( volatile char *  ) pcFile;
   volatile uint32_t ulLineNumber = ulLine;

        ( void ) pcFileName;
        ( void ) ulLineNumber;

        printf("vAssertCalled: %s, %ld\n", pcFile, ulLine);
        /* Need to use this instead of printf vDirectPrintMsg(msg); */

        taskDISABLE_INTERRUPTS();
        {
                while(1);
        }
        taskENABLE_INTERRUPTS();
}

void vApplicationMallocFailedHook( void )
{
        /* Called if a call to pvPortMalloc() fails because there is insufficient
        free memory available in the FreeRTOS heap.  pvPortMalloc() is called
        internally by FreeRTOS API functions that create tasks, queues, software
        timers, and semaphores.  The size of the FreeRTOS heap is set by the
        configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
        vAssertCalled( __FILE__, __LINE__ );
}

/* 
** Startup function that creates and runs two FreeRTOS tasks 
*/
void main(void)
{
    /* Init of print related tasks: */
    if ( pdFAIL == printInit(PRINT_UART_NR) )
    {
        FreeRTOS_Error("Initialization of print failed\r\n");
    }

    /*
     * I M P O R T A N T :
     * Make sure (in startup.s) that main is entered in Supervisor mode.
     * When vTaskStartScheduler launches the first task, it will switch
     * to System mode and enable interrupt exceptions.
    */
    vDirectPrintMsg("= = = M A I N  S T A R T E D = = =\r\n\r\n");

    /* 
    ** Init of receiver related tasks: 
    */
    if ( pdFAIL == recvInit(RECV_UART_NR) )
    {
        FreeRTOS_Error("Initialization of receiver failed\r\n");
    }

    /* 
    ** Create a print gate keeper task: 
    */
    if ( pdPASS != xTaskCreate(printGateKeeperTask, "gk", 128, NULL,
                               PRIOR_PRINT_GATEKEEPR, NULL) )
    {
        FreeRTOS_Error("Could not create a print gate keeper task\r\n");
    }
    else
    {
       vDirectPrintMsg("Created printGateKeeperTask\n");
    }

    if ( pdPASS != xTaskCreate(recvTask, "recv", 128, NULL, PRIOR_RECEIVER, NULL) )
    {
        FreeRTOS_Error("Could not create a receiver task\r\n");
    }
    else
    {
       vDirectPrintMsg("Created recvTask\n");
    }
    vDirectPrintMsg("A text may be entered using a keyboard.\r\n");
    vDirectPrintMsg("It will be displayed when 'Enter' is pressed.\r\n\r\n");

    /*
    ** Create the Startup Task - This is where all the rest of the Application tasks are created
    */
    if ( pdPASS != xTaskCreate(startupTask, "start", 1024, NULL, PRIOR_START_TASK , NULL))
    {
        FreeRTOS_Error("Could not create Startup task\r\n");
    }
    else
    {
       vDirectPrintMsg("Created startupTask, starting scheduler\n");
    }

    /* 
    ** Start the FreeRTOS scheduler 
    */
    vTaskStartScheduler();

    /*
    ** If all goes well, vTaskStartScheduler should never return.
    ** If it does return, typically not enough heap memory is reserved.
    */
    FreeRTOS_Error("Could not start the scheduler!!!\r\n");

    /* just in case if an infinite loop is somehow omitted in FreeRTOS_Error */
    for ( ; ; );
}
