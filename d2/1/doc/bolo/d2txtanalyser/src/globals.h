#ifndef _GLOBALS_H_

#define _GLOBALS_H_

typedef struct
{
   char            error_strtmp[2048];
   char            strtmp[1024];
   GLB_MPQ_S       mpq[MPQ_MAX];
   char            * mpq_filename[MPQ_MAX];
   char            * mod_dir;
   char            * be_strict;
   STR_CMP_FUNC    str_cmp_func;
   UBYTE           * ini;
   TXT_S           txt[TXT_MAX];
   TXT_COL_NAMES_S txt_col_names;
   FILE            * std_warning;

} GLB_DATAS_S;

extern GLB_DATAS_S glb_datas;

#endif
