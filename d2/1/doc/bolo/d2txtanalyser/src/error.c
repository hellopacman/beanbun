#include <stdio.h>
#include <stdlib.h>

#include "error.h"


// ========================================================================================
// display a critical error, and exit
// input :
//    * pointer to the string with error to display
// ========================================================================================
void d2txtanalyser_error(char * strtmp)
{
   printf(strtmp);
   printf("\n");
   fflush(stdout);
   exit(-1);
}
