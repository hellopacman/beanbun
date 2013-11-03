#ifndef _TXTREAD_H_

#define _TXTREAD_H_

UBYTE * txt_extract              (char * filename, long * buffer_length);
int     txt_read                 (char * filename, TXT_ENUM txt_idx);
int     txt_display_infos        (TXT_ENUM txt_idx);
int     txt_replace_tab_by_zero  (TXT_ENUM txt_idx);
void    txt_init_header_names    (void);
int     txt_find_column_position (TXT_S * ptr_txt, int idx);
int     txt_init_values_pointers (TXT_ENUM txt_idx);

#endif
