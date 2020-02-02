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
 * Implementation of functions that perform printing messages to a UART
 *
 * @author Jernej Kovacic
 */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>

#include "app_config.h"
#include "bsp.h"
#include "uart.h"
#include "print.h"

/* Allocate the buffer for printing individual characters */
static portCHAR printChBuf[PRINT_CHR_BUF_SIZE][PRINT_MESSAGE_SIZE];

/* Position of the currently available "slot" in the buffer */
static uint16_t chBufCntr = 0;

/* UART number: */
static uint8_t printUartNr = (uint8_t) -1;

/* Messages to be printed will be pushed to this queue */
static QueueHandle_t printQueue;

/*
 * Initializes all print related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be printed
 * via vPrintMsg or vPrintChar!
 *
 * @param uart_nr - number of the UART
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
int16_t printInit(uint8_t uart_nr)
{
    uint16_t i;

    /* Check if UART number is valid */
    if ( uart_nr >= BSP_NR_UARTS )
    {
        return pdFAIL;
    }

    printUartNr = uart_nr;

    /*
    ** Initialize the character print buffer.
    ** It is sufficient to set each string's second character to '\0'.
    */
    for ( i=0; i<PRINT_CHR_BUF_SIZE; ++i )
    {
        memset(printChBuf[i], 0, PRINT_MESSAGE_SIZE); 
    }

    /* Create and assert a queue for the gate keeper task */
    printQueue = xQueueCreate(PRINT_QUEUE_SIZE, PRINT_MESSAGE_SIZE);
    if ( 0 == printQueue )
    {
        return pdFAIL;
    }

    /* Enable the UART for transmission */
    uart_enableTx(printUartNr);

    return pdPASS;
}


/*
** A gate keeper task that waits for messages to appear in the print queue and
** prints them. This prevents corruption of printed messages if a task that
** actually attempts to print, is preempted.
**
** + @param params - ignored
*/
void printGateKeeperTask(void* params)
{
    static portCHAR message[PRINT_MESSAGE_SIZE];

    for ( ; ; )
    {
        /* The task is blocked until something appears in the queue */
        xQueueReceive(printQueue, (void*) &message, portMAX_DELAY);
        /* Print the message in the queue */
        uart_print(printUartNr, message);
    }

    /* if it ever breaks out of the infinite loop... */
    vTaskDelete(NULL);

    /* suppress a warning since 'params' is ignored */
    (void) params;
}

/**
 * Prints a message in a thread safe manner - even if the calling task is preempted,
 * the entire message will be printed.
 *
 * Nothing is printed if 'msg' equals NULL.
 *
 * @note This function may only be called when the FreeRTOS scheduler is running!
 *
 * @param msg - a message to be printed
 */
void vPrintMsg(const portCHAR* msg)
{
    BaseType_t qRetStatus;

    if ( NULL != msg )
    {
        qRetStatus = xQueueSendToBack(printQueue, (void*)msg, 0);
        if (qRetStatus != pdTRUE )
        {
           uart_print(printUartNr, "GateKeeper Print Queue Full!\n");
        }
    }
}

/**
 * Prints a character in a thread safe manner - even if the calling task preempts
 * another printing task, its message will not be corrupted. Additionally, if another
 * task attempts to print a character, the buffer will not be corrupted.
 *
 * @note This function may only be called when the FreeRTOS scheduler is running!
 *
 * @param ch - a character to be printed
 */
void vPrintChar(portCHAR ch)
{
    /*
     * Put 'ch' to the first character of the current buffer string,
     * note that the second character has been initialized to '\0'.
     */
    printChBuf[chBufCntr][0] = ch;

    /* Now the current buffer string may be sent to the printing queue */
    vPrintMsg(printChBuf[chBufCntr]);

    /*
     * Update chBufCntr and make sure it always
     * remains between 0 and CHR_PRINT_BUF_SIZE-1
     */
    ++chBufCntr;
    chBufCntr %= PRINT_CHR_BUF_SIZE;
}


/**
 * Prints a message directly to the UART. The function is not thread safe
 * and corruptions are possible when multiple tasks attempt to print "simultaneously"
 *
 * Nothing is printed if 'msg' equals NULL.
 *
 * @note This function should only be called when the FreeRTOS scheduler is not running!
 *
 * @param msg - a message to be printed
 */
void vDirectPrintMsg(const portCHAR* msg)
{
    if ( NULL != msg )
    {
        uart_print(printUartNr, msg);
    }
}

/**
 * Prints a character directly to the UART. The function is not thread safe and
 * corruptions are possible when multiple tasks attempt to print "simultaneously".
 *
 * @note his function should only be called when the FreeRTOS scheduler is not running!
 *
 * @param ch - a character to be printed
 */
void vDirectPrintCh(portCHAR ch)
{
    uart_printChar(printUartNr, ch);
}
