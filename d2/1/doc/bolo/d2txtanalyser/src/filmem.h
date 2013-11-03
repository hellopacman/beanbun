#ifndef _FILMEM_H_

#define _FILMEM_H_

int   filmem_file_exists (char * filename);
UBYTE * filmem_read      (char * filename, long * buffer_length);

#endif
