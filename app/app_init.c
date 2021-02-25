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

/* function to test the RAM disk */
void vCreateAndVerifyExampleFiles( const char *pcMountPath );

void ListFATDir( const char *pcDirectoryToScan );

/* FreeRTOS error function */
void FreeRTOS_Error(const portCHAR* msg);

/*
** Binary tar file address and size
*/
extern unsigned int _binary_tarfile_size;
extern unsigned int _binary_tarfile_start;

/* 
** Struct with settings for each task 
*/
typedef struct _paramStruct
{
    portCHAR    *text;           /* text to be printed by the task */
    UBaseType_t  delay;          /* delay in milliseconds */
} paramStruct;

/* Parameters for two tasks */
static const paramStruct tParam[2] =
{
    (paramStruct) { 
                    .text="Reg Task\r\n", 
                    .delay=1000 
                  },
    (paramStruct) { 
                    .text="Periodic Task\r\n", 
                    .delay=3000 
                  }
};

/* 
** Default parameters if no parameter struct is available 
*/
static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000;


/* 
** Task function - may be instantiated in multiple tasks 
*/
void vTaskFunction( void *pvParameters )
{
    UBaseType_t  delay;
    paramStruct *params     = (paramStruct*) pvParameters;
    char        *osTaskName = pcTaskGetName(NULL);

    delay = ( NULL==params ? defaultDelay : params->delay);

    for( ; ; )
    {
        /* 
        ** Print out the name of this task. 
        */
        printf("%s\n",osTaskName);
        vTaskDelay( delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}

/* 
** Fixed frequency periodic task function - may be instantiated in multiple tasks 
*/
void vPeriodicTaskFunction(void* pvParameters)
{
    const portCHAR* taskName;
    UBaseType_t delay;
    paramStruct* params = (paramStruct*) pvParameters;
    TickType_t lastWakeTime;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    /*
     * This variable must be initialized once.
     * Then it will be updated automatically by vTaskDelayUntil().
     */
    lastWakeTime = xTaskGetTickCount();


    for( ; ; )
    {
        /* 
        ** Print out the name of this task. 
        */
        printf("%s",taskName);

        /*
        ** The task will unblock exactly after 'delay' milliseconds (actually
        ** after the appropriate number of ticks), relative from the moment
        ** it was last unblocked.
        */
        vTaskDelayUntil( &lastWakeTime, delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}

/*
** Startup task 
*/
void startupTask(void* pvParameters)
{
    char  string1[32];
    char  string2[32];
    int   status;

    printf("--- startupTask starting! ---\n");

    /*
    ** Test strncpy - see if newlib is working
    */
    strncpy(string2, "Hello", 32);
    strncpy(string1, string2, 32);

    /*
    ** Create the RAM disk
    */
    printf("Creating RAM Disk\n");
    CreateRamDisk();

    #if 1
       /*
       ** Test the RAM disk
       */
       printf("Testing the RAM Disk\n");
       vCreateAndVerifyExampleFiles("/ram");
    #endif

    printf("Get directory for /ram - before untar\n");
    ListFATDir("/ram");

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

    printf("Get directory for /ram after untar\n");
    ListFATDir("/ram");
    printf("Get directory for /ram/rootfs/cf after untar\n");
    ListFATDir("/ram/rootfs/cf");

    /* 
    ** For this demo, create two tasks: 
    */
    printf("Creating task1-3\n");
    if ( pdPASS != xTaskCreate(vTaskFunction, "task1", 1024, (void*) &tParam[0], PRIOR_PERIODIC, NULL))
        FreeRTOS_Error("Could not create task1\r\n");

    if ( pdPASS != xTaskCreate(vTaskFunction, "task2", 1024, (void*) &tParam[0], PRIOR_PERIODIC, NULL))
        FreeRTOS_Error("Could not create task2\r\n");

    if ( pdPASS != xTaskCreate(vTaskFunction, "task3", 1024, (void*) &tParam[0], PRIOR_PERIODIC, NULL))
        FreeRTOS_Error("Could not create task3\r\n");

    printf("Creating task10\n");
    if ( pdPASS != xTaskCreate(vPeriodicTaskFunction, "task10", 1024, (void*) &tParam[1],
                               PRIOR_FIX_FREQ_PERIODIC, NULL) )
        FreeRTOS_Error("Could not create task2\r\n");

    printf(" --- startupTask jobs complete - exiting task\n");
    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) pvParameters;

}

