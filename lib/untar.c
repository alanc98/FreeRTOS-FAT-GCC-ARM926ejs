/**
 * @file

 * @brief Untar an Image
 * @ingroup libmisc_untar_img Untar Image

 * FIXME:
 *   1. Symbolic links are not created.
 *   2. Untar_FromMemory uses FILE *fp.
 *   3. How to determine end of archive?

 */

/*
 *  Written by: Jake Janovetz <janovetz@tempest.ece.uiuc.edu>

 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "untar.h"
#include "ff_headers.h"
#include "ff_stdio.h"


/*
 * TAR file format:

 *   Offset   Length   Contents
 *     0    100 bytes  File name ('\0' terminated, 99 maxmum length)
 *   100      8 bytes  File mode (in octal ascii)
 *   108      8 bytes  User ID (in octal ascii)
 *   116      8 bytes  Group ID (in octal ascii)
 *   124     12 bytes  File size (s) (in octal ascii)
 *   136     12 bytes  Modify time (in octal ascii)
 *   148      8 bytes  Header checksum (in octal ascii)
 *   156      1 bytes  Link flag
 *   157    100 bytes  Linkname ('\0' terminated, 99 maxmum length)
 *   257      8 bytes  Magic PAX ("ustar\0" + 2 bytes padding)
 *   257      8 bytes  Magic GNU tar ("ustar  \0")
 *   265     32 bytes  User name ('\0' terminated, 31 maxmum length)
 *   297     32 bytes  Group name ('\0' terminated, 31 maxmum length)
 *   329      8 bytes  Major device ID (in octal ascii)
 *   337      8 bytes  Minor device ID (in octal ascii)
 *   345    155 bytes  Prefix
 *   512   (s+p)bytes  File contents (s+p) := (((s) + 511) & ~511),
 *                     round up to 512 bytes
 *
 *   Checksum:
 *   int i, sum;
 *   char* header = tar_header_pointer;
 *   sum = 0;
 *   for(i = 0; i < 512; i++)
 *       sum += 0xFF & header[i];
 */ 

#define MAX_NAME_FIELD_SIZE      99

/*
** List a directory
*/
void ListFATDir( const char *pcDirectoryToScan )
{
    FF_FindData_t *pxFindStruct;
    const char  *pcAttrib,
                *pcWritableFile = "writable file",
                *pcReadOnlyFile = "read only file",
                *pcDirectory = "directory";

    /* FF_FindData_t can be large, so it is best to allocate the structure
    dynamically, rather than declare it as a stack variable. */
    pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );

    /* FF_FindData_t must be cleared to 0. */
    memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );

    /* The first parameter to ff_findfist() is the directory being searched.  Do
    not add wildcards to the end of the directory name. */
    if( ff_findfirst( pcDirectoryToScan, pxFindStruct ) == 0 )
    {
        do
        {
            /* Point pcAttrib to a string that describes the file. */
            if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0 )
            {
                pcAttrib = pcDirectory;
            }
            else if( pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY )
            {
                pcAttrib = pcReadOnlyFile;
            }
            else
            {
                pcAttrib = pcWritableFile;
            }

            /* Print the files name, size, and attribute string. */
            printf("%s [%s] [size=%ld]\n", pxFindStruct->pcFileName,
                                          pcAttrib,
                                          pxFindStruct->ulFileSize);

        } while( ff_findnext( pxFindStruct ) == 0 );
    }

    /* Free the allocated FF_FindData_t structure. */
    vPortFree( pxFindStruct );
}

/*
 * This converts octal ASCII number representations into an
 * unsigned long.  Only support 32-bit numbers for now.
 */ 
unsigned long
_rtems_octal2ulong(
  const char *octascii,
  size_t len
)
{
  size_t        i;
  unsigned long num;

  num = 0;
  for (i=0; i < len; i++) {
    if ((octascii[i] < '0') || (octascii[i] > '9')) {
      continue;
    }
    num  = num * 8 + ((unsigned long)(octascii[i] - '0'));
  }
  return(num);
}

/*
 * Function: Untar_FromMemory
 *
 * Description:
 *
 *    This is a simple subroutine used to rip links, directories, and
 *    files out of a block of memory.
 *
 *
 * Inputs:
 *
 *    void *  tar_buf    - Pointer to TAR buffer.
 *    size_t  size       - Length of TAR buffer.
 *
 *
 * Output:
 *
 *    int - UNTAR_SUCCESSFUL (0)    on successful completion.
 *          UNTAR_INVALID_CHECKSUM  for an invalid header checksum.
 *          UNTAR_INVALID_HEADER    for an invalid header.
 *
 */
int
Untar_FromMemory(
  void   *tar_buf,
  size_t  size
)
{
  FF_FILE        *fp;
  const char     *tar_ptr = (const char *)tar_buf;
  const char     *bufr;
  size_t         n;
  char           fname[100];
  char           linkname[100];
  int            sum;
  int            hdr_chksum;
  int            retval;
  unsigned long  ptr;
  unsigned long  i;
  unsigned long  nblocks;
  unsigned long  file_size;
  unsigned char  linkflag;

  char pcBuffer[ 50 ];

  printf("untar: memory at %p (%zu)\n", tar_buf, size);
  ff_getcwd( pcBuffer, sizeof( pcBuffer ) );
  printf("CWD = %s\n", pcBuffer );

  ptr = 0;
  while (1) {
    if (ptr + 512 > size) {
      retval = UNTAR_SUCCESSFUL;
      break;
    }

    /* Read the header */
    bufr = &tar_ptr[ptr];
    ptr += 512;
    if (strncmp(&bufr[257], "ustar", 5)) {
      retval = UNTAR_SUCCESSFUL;
      break;
    }

    strncpy(fname, bufr, MAX_NAME_FIELD_SIZE);
    fname[MAX_NAME_FIELD_SIZE] = '\0';

    linkflag   = bufr[156];
    file_size  = _rtems_octal2ulong(&bufr[124], 12);

    /*
     * Compute the TAR checksum and check with the value in
     * the archive.  The checksum is computed over the entire
     * header, but the checksum field is substituted with blanks.
     */
    hdr_chksum = _rtems_octal2ulong(&bufr[148], 8);
    sum = _rtems_tar_header_checksum(bufr);

    if (sum != hdr_chksum) {
      retval = UNTAR_INVALID_CHECKSUM;
      break;
    }

    /*
     * We've decoded the header, now figure out what it contains and
     * do something with it.
     */
    if (linkflag == SYMTYPE) {
      strncpy(linkname, &bufr[157], MAX_NAME_FIELD_SIZE);
      linkname[MAX_NAME_FIELD_SIZE] = '\0';
      printf("FAT File system does not support links\n");
      /* symlink(linkname, fname); */
    } else if (linkflag == REGTYPE) {
      nblocks = (((file_size) + 511) & ~511) / 512;
      if ((fp = ff_fopen(fname, "w")) == NULL) {
        printf("Untar: failed to create file %s\n", fname);
        ptr += 512 * nblocks;
      } else {
        printf("Untar: Opened %s\n",fname);
        unsigned long sizeToGo = file_size;
        size_t len;

        /*
         * Read out the data.  There are nblocks of data where nblocks
         * is the file_size rounded to the nearest 512-byte boundary.
         */
        for (i=0; i<nblocks; i++) {
          len = ((sizeToGo < 512L)?(sizeToGo):(512L));
          n = ff_fwrite(&tar_ptr[ptr], 1, len, fp);
          if (n != len) {
            printf("untar: Error during write\n");
            retval  = UNTAR_FAIL;
            break;
          }
          ptr += 512;
          sizeToGo -= n;
        }
        ff_fclose(fp);
        printf("Untar: Closed %s\n",fname);
      }
    } else if (linkflag == DIRTYPE) {
      if ( ff_mkdir(fname) != 0 ) {
        if (errno == EEXIST) {
          FF_Stat_t stat_buf;
          if ( ff_stat(fname, &stat_buf) == 0 ) {
            if ((stat_buf.st_mode & FF_IFDIR) == FF_IFDIR) {
              continue;
            } else {
              if ( ff_remove(fname) != -1 ) {
                if ( ff_mkdir(fname) == 0 )
                  continue;
              }
            }
          }
        }
        printf("Untar: failed to create directory %s\n", fname);
        retval = UNTAR_FAIL;
        break;
      }
    }
  }

  return(retval);
}

/*
 * Compute the TAR checksum and check with the value in
 * the archive.  The checksum is computed over the entire
 * header, but the checksum field is substituted with blanks.
 */
int
_rtems_tar_header_checksum(
  const char *bufr
)
{
  int  i, sum;

  sum = 0;
  for (i=0; i<512; i++) {
    if ((i >= 148) && (i < 156))
      sum += 0xff & ' ';
    else
     sum += 0xff & bufr[i];
  }
  return(sum);
}
