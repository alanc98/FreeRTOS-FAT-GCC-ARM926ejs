/*
** syscalls.c - glue code for newlib
**              right now this just supports standard output, but it could be expanded
*/

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>
#include "app_config.h"
#include "print.h"
#include "bsp.h"
#include "uart.h"

#define UNUSED(x) (void)(x)

int _read(int file, void *ptr, size_t len) 
{
   UNUSED(ptr);
   UNUSED(len);

   vDirectPrintMsg("_read\n");

   if (file <= STDERR_FILENO) 
   {
       return 1;
   } 
   else 
   {
      /* Need to implement a device and/or FAT file system read here */ 
      return -1;
   }
}

int _write(int file, const void *ptr, size_t len) {

   char *chptr = (char *)ptr;

   UNUSED(ptr);
   UNUSED(len);

   chptr[len] = '\0';

   if (file <= STDERR_FILENO) 
   {
      /* Need to account for messages bigger than PRINT_MESSAGE_SIZE */
      vPrintMsg((const portCHAR *)chptr); 
      return len; 
   }
   else 
   {
      /* Need to implement a device and/or FAT file system write here */ 
      return -1;
   }
}

off_t _lseek(int file, off_t ptr, int dir) 
{
   UNUSED(ptr);
   UNUSED(dir);

   vDirectPrintMsg("_lseek\n");

   if (file <= STDERR_FILENO) 
   {
      return -1;
   } 
   else 
   {
      /* Need to implement a FAT file system lseek here */ 
      return -1;
   }
}

int _close(int file) 
{
   vDirectPrintMsg("_close\n");
   if (file <= STDERR_FILENO) 
   {
      return -1;
   } 
   else 
   {
      /* Need to implement a device and/or FAT file system close here */ 
      return -1;
   }
}

int _open(const char *name, int flags, ...) 
{
   UNUSED(name);
   UNUSED(flags);

   vDirectPrintMsg("_open\n");

   /* dont need to open stdlib/stderr */
   /* Need to implement a device and/or FAT file system open here */ 
   return -1;
}

int _link(const char *oldpath, const char *newpath) 
{
   UNUSED(oldpath);
   UNUSED(newpath);

   vDirectPrintMsg("_link\n");

   /* Need to implement a device and/or FAT file system link here */ 
   return -1;
}

int _rename(const char *oldpath, const char *newpath) 
{
   UNUSED(oldpath);
   UNUSED(newpath);

   vDirectPrintMsg("_rename\n");

   /* Need to implement a device and/or FAT file system rename here */ 
   return -1;
}

int _unlink(const char *pathname) 
{
   UNUSED(pathname);
   /* Need to implement a device and/or FAT file system unlink here */ 
   return -1;
}

int fsync(int fd) 
{
   vDirectPrintMsg("fsync\n");

   if (fd <= STDERR_FILENO) 
   {
      return -1;
   }
   else 
   {
      /* Need to implement a device and/or FAT file system fsync here */ 
      return -1;
   }
}

int _fstat(int fd, struct stat *st) 
{
   UNUSED(st);

   vDirectPrintMsg("fstat\n");

   if (fd <= STDERR_FILENO) 
   {
      return -1;
   }
   else 
   {
      /* Need to implement a device and/or FAT file system fstat here */ 
      return -1;
   }
}

int _stat(const char *path, struct stat *st) 
{
   UNUSED(path);
   UNUSED(st);

   vDirectPrintMsg("_stat\n");

   /* Need to implement a device and/or FAT file system stat here */ 
   return -1;
}

int _isatty(int fd) 
{
   vDirectPrintMsg("_isatty\n");

   if (fd <= STDERR_FILENO)
      return 1;
   else
      return 0;
}

void exit(int n) 
{
   vDirectPrintMsg("exit\n");

   printf("exit %u\r\n", n);
   while(1);
}

void _exit(int n) 
{
   vDirectPrintMsg("_exit\n");

   printf("Exit %u\r\n", n);
   while(1);
}

/*
** Should tie this in with FreeRTOS Malloc!
*/
void *_sbrk(ptrdiff_t increment) 
{
   /* vDirectPrintMsg("_sbrk\n"); */

   extern char __heap_begin, __heap_end;
   static void *cur_heap_pos = 0;

   /* Initialize cur_heap_pos */
   if (cur_heap_pos == 0)
      cur_heap_pos = &__heap_begin;

   if ((cur_heap_pos + increment) > (void *) &__heap_end) 
   {
      printf("Warning: Heap is running full trying to allocate %i bytes!!!!\n", increment);
      printf("\tHeap start address\t= %p\n", &__heap_begin);
      printf("\tHeap end address\t= %p\n", &__heap_end);
      printf("\tCurrent heap address\t= %p\n", cur_heap_pos);
      errno = ENOMEM;
      return (void *) -1;
   }

   void * old_heap_pos = cur_heap_pos;
   cur_heap_pos += increment;
   return old_heap_pos;

   return(0);
}

int _kill(void) 
{
   return -1;
}

int _getpid(void) 
{
   return 1;
}

int _gettimeofday(void) 
{
   return -1;
}
