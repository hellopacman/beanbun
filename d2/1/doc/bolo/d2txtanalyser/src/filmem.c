#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "error.h"
#include "filmem.h"
#include "mpq\mpqtypes.h"
#include "globals.h"


// ========================================================================================
// test if a file exists
// input :
//    * path & ename
// output :
//    * TRUE if exists, FALSE if not found
// ========================================================================================
int filmem_file_exists(char * filename)
{
   FILE * in;

   if (filename == NULL)
      return FALSE;

   if (strlen(filename) <= 0)
      return FALSE;

   in = fopen(filename, "rb");
   if (in == NULL)
      return FALSE;
   fclose(in);
   return TRUE;
}


// ========================================================================================
// copy a file from disk onto memory
// note : the function take care to add a byte of value 0x00 at the end of the buffer.
//        this don't affect the buffer length, but ensure that a text file could be
//        understand as 1 large zero-terminated string
// input :
//    * path & name
//    * pointer to the data that will receive the lenght in bytes of the file
// output :
//    * pointer to the copy of the file in memory
// ========================================================================================
UBYTE * filmem_read(char * filename, long * buffer_length)
{
   FILE  * in;
   UBYTE * ptr = NULL;


   in = fopen(filename, "rb");
   if (in == NULL)
   {
      sprintf(glb_datas.error_strtmp, "filmem_read() : can't open %s\n", filename);
      d2txtanalyser_error(glb_datas.error_strtmp);
   }
   fseek(in, 0, SEEK_END);
   * buffer_length = ftell(in);
   fseek(in, 0, SEEK_SET);

   ptr = (UBYTE *) malloc((* buffer_length) + 1);
   if (ptr == NULL)
   {
      fclose(in);
      sprintf(glb_datas.error_strtmp, "filmem_read() : can't allocate %li bytes to copy %s\n",
         (* buffer_length) + 1, filename);
      d2txtanalyser_error(glb_datas.error_strtmp);
   }
   fread(ptr, * buffer_length, 1, in);
   ptr[* buffer_length] = 0;

   fclose(in);
   return ptr;
}
