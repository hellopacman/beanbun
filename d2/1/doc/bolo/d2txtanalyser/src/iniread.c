#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "filmem.h"
#include "iniread.h"
#include "mpq\mpqtypes.h"
#include "globals.h"


// ========================================================================================
// find & load the value of a keystring from the D2TxtAnalyser ini
// input :
//    * pointer to file in memory
//    * keystring
//    * pointer to the data that will receive a pointer to the value
//      (will be set to NULL if keystring is not found, or value is empty) 
// output :
//    * 0 if ok, non-zero if error
// ========================================================================================
int ini_find_keystring(UBYTE * ini, char * keystring, char ** ret_value)
{
   UBYTE * ptr;
   int   c, i, done, length, skip_line;
   char  * value;


   // init the value
   * ret_value = NULL;

   length = strlen(keystring);
   if (length <= 0)
      return -1;

   value     = NULL;
   ptr       = ini;
   c         = * ptr;
   done      = FALSE;
   skip_line = FALSE;
   for(;;)
   {
      if (skip_line == TRUE)
      {
         // skip this line
         skip_line = FALSE;
         while ((c != 0x00) && (c != 0x0A) && (c != 0x0D))
         {
            ptr++;
            c = * ptr;
         }
         if (c == 0x00)
            return -1;
         while ((c == 0x0A) || (c == 0x0D))
         {
            ptr++;
            c = * ptr;
         }
      }

      // search the first letter
      while ((c != 0x00) && (c <= 32))
      {
         ptr++;
         c = * ptr;
      }
      if (c == 0x00)
         return -1;
      if (c == ';')
      {
         // a comment : next line
         skip_line = TRUE;
      }
      else
      {
         // is it our keystring ? (case insensitive)
         for (i=0; (i < length) && (skip_line == FALSE); i++)
         {
            if (tolower(ptr[i]) != tolower(keystring[i]))
            {
               // not our keystring : next line
               skip_line = TRUE;
            }
         }
         if (skip_line == FALSE)
         {
            if ((ptr[length] == '=') || (ptr[length] <= 32))
            {
               // it should be our key (but only if a '=' after)

               // find the '='
               ptr += length;
               c = * ptr;
               while ((c != 0x00) && (c != '=') && (c != 0x0A) && (c != 0x0D))
               {
                  ptr++;
                  c = * ptr;
               }
               if (c != '=')
               {
                  // skip this line
                  skip_line = TRUE;
               }
               else
               {
                  // it's our key. skip the '='
                  ptr++;
                  c = * ptr;

                  // find the first letter
                  while ((c != 0x00) && (c != 0x0A) && (c != 0x0D) && (c <= 32))
                  {
                     ptr++;
                     c = * ptr;
                  }

                  if (c <= 32)
                  {
                     // the string is there, but the value is not, return an empty value
                     value = NULL;
                  }
                  else
                  {
                     // which length is the value ?
                     i=0;
                     while (ptr[i] >= 32)
                        i++;
                     if (i)
                     {
                        // don't take into account the last spaces
                        i--;
                        while ((ptr[i] == 32) && (i >= 0))
                           i--;
                        i++;
                     }
                     if (i <= 0)
                     {
                        // empty value (shouldn't happen)
                        value = NULL;
                     }
                     else
                     {
                        // create a buffer and copy the value
                        value = (char *) malloc(i+2);
                        if (value == NULL)
                           return -1;
                        strncpy(value, (char *) ptr, i);
                        value[i] = 0x00;
                     }
                  }

                  // return the value
                  * ret_value = value;
                  return 0;
               }
            }
            else
            {
               // false alert : it's a longer key that was starting the same
               skip_line = TRUE;
            }
         }
      }
   }
}


// ========================================================================================
// load D2TxtAnalyser ini & proceed it
// input :
//    * path & name of the ini
// output :
//    * 0 if ok, non-zero if error
// ========================================================================================
int ini_read(char * ini_name)
{
   UBYTE * ini;
   int   i;
   long  dummy;


   // copy the ini in memory
   fprintf(stderr, "Opening %s\n", ini_name);
   ini = filmem_read(ini_name, & dummy);
   if (ini == NULL)
      return -1;
   glb_datas.ini = ini;

   // read all mpq paths
   ini_find_keystring(ini, "d2data",   & glb_datas.mpq_filename[D2DATA]);
   ini_find_keystring(ini, "d2exp",    & glb_datas.mpq_filename[D2EXP]);
   ini_find_keystring(ini, "patch_d2", & glb_datas.mpq_filename[PATCH_D2]);
   ini_find_keystring(ini, "mod_dir",  & glb_datas.mod_dir);
   if (glb_datas.mod_dir)
   {
      i = strlen(glb_datas.mod_dir) - 1; //  position of last character
      if (i >= 0)
      {
         if (glb_datas.mod_dir[i] != '\\')
         {
            // add a '\' to the string (there's enough space)
            glb_datas.mod_dir[i+1] = '\\';
            glb_datas.mod_dir[i+2] = 0x00;
         }
      }
   }

   // read user options
   ini_find_keystring(ini, "be_strict", & glb_datas.be_strict);

   // end
   return 0;
}
