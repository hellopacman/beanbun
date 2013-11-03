#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "error.h"
#include "filmem.h"
#include "txtread.h"
#include "mpq\mpqtypes.h"
#include "mpq\mpqview.h"
#include "globals.h"


// ========================================================================================
// load a file first from mod_dir, else from the most recent mpq
// input :
//    * path & filename (inside a mpq)
//    * pointer to a data that will receive the file length
// output :
//    * pointer to the buffer if ok, NULL if error
// ========================================================================================
UBYTE * txt_extract(char * filename, long * buffer_length)
{
   int   i, mpq_idx, num_entry;
   UBYTE * buffer;
   char  mpq_name[3][15] = {
            {"patch_d2.mpq"},
            {"d2exp.mpq   "},
            {"d2data.mpq  "}
         };


   * buffer_length = 0;

   // first, let's try in the Mod Directory
   if (glb_datas.mod_dir != NULL)
   {
      strcpy(glb_datas.strtmp, glb_datas.mod_dir);
      strcat(glb_datas.strtmp, filename);
      if (filmem_file_exists(glb_datas.strtmp))
      {
         buffer = filmem_read(glb_datas.strtmp, buffer_length);
         if (buffer)
         {
            printf("extracted from mod_dir      : %s\n", filename);
            return buffer;
         }
         else
         {
            printf("ERROR : %s can't be open\n", glb_datas.strtmp);
            return NULL;
         }
      }
   }

   // since not found in mod directory, try in all mpq now
   mpq_idx = 0;
   for (i=0; i < 3; i++)
   {
      switch(i)
      {
         case 0 : mpq_idx = PATCH_D2; break;
         case 1 : mpq_idx = D2EXP;    break;
         case 2 : mpq_idx = D2DATA;   break;
      }
      glb_mpq = & glb_datas.mpq[mpq_idx];
      if (glb_mpq->is_open == TRUE)
      {
         num_entry = mpq_batch_load_in_mem(filename, & buffer, buffer_length, FALSE);
         if (num_entry != -1)
         {
            printf("extracted from %s : %s\n", mpq_name[i], filename);
            return buffer;
         }
      }
   }

   // not found
   printf("ERROR : %s was not found in a MPQ or in mod_dir\n", filename);
   return NULL;
}


// ========================================================================================
// load a tabulated text file, & init some values
// input :
//    * path & file (inside a mpq)
//    * txt ID
// output :
//    * 0 if ok, non-zero if error
// ========================================================================================
int txt_read(char * filename, TXT_ENUM txt_idx)
{
   TXT_S * ptr_txt;
   int   done, c, curr_col, is_ok = TRUE;
   UBYTE * ptr;


   if ((txt_idx < 0) || (txt_idx >= TXT_MAX))
      return -1;
   ptr_txt = & glb_datas.txt[txt_idx];
   ptr_txt->buffer = txt_extract(filename, & ptr_txt->buffer_length);
   if (ptr_txt->buffer == NULL)
      return -1;

   // count how many columns & rows
   ptr = ptr_txt->buffer;
   c = * ptr;
   done = FALSE;
   while ( ! done)
   {
      if (c == 0x00)
         done = TRUE;
      else
      {
         // start a new line
         curr_col = 1;
         ptr_txt->nb_rows++;

         // how many columns ?
         while ((c != 0x00) && (c != 0x0D) && (c != 0x0A))
         {
            if (c == '\t')
               curr_col++;
            ptr++;
            c = * ptr;
         }
         if (c == 0x00)
            done = TRUE;
         else
         {
            if (ptr_txt->nb_rows > 1)
            {
               // at least 2nd line, compare if same # of columns
               if (curr_col != ptr_txt->nb_columns)
               {
                  // nope, so wrong .txt format
                  printf("ERROR : line %5li has %3li column%s. Expected : %3li\n",
                     ptr_txt->nb_rows,
                     curr_col,
                     curr_col > 1 ? "s" : "",
                     ptr_txt->nb_columns
                  );
                  is_ok = FALSE;
               }
            }
            else
            {
               // 1st line ended, set # of columns
               ptr_txt->nb_columns = curr_col;
            }
         }

         // search real end of line
         while ((c != 0x00) && (c != 0x0A))
         {
            ptr++;
            c = * ptr;
         }

         // skip this char
         if (c != 0x00)
         {
            ptr++;
            c = * ptr;
         }
      }
   }

   // end
   if (is_ok == TRUE)
   {
      ptr_txt->txt_idx = txt_idx; 
      strcpy(ptr_txt->filename, filename);
      return 0;
   }
   return -1;
}


// ========================================================================================
// display basic infos of a .txt file
// input :
//    * txt ID
// output :
//    * 0 if ok, non-zero if error
// ========================================================================================
int txt_display_infos(TXT_ENUM txt_idx)
{
   TXT_S * ptr_txt;
   long  length;


   if ((txt_idx < 0) || (txt_idx >= TXT_MAX))
      return -1;
   ptr_txt = & glb_datas.txt[txt_idx];

   if (ptr_txt->buffer == NULL)
   {
      sprintf(glb_datas.error_strtmp, "txt_display_infos() : ptr_txt->buffer == NULL");
      d2txtanalyser_error(glb_datas.error_strtmp);
   }

   length = ptr_txt->buffer_length;
   printf("   * %li bytes", length);
   if (length >= 1024L)
   {
      if (length >= 1048576L)
         printf(" (%li MB)", length / 1048576L);
      else
         printf(" (%li KB)", length / 1024L);
   }
   printf("\n   * %li column%s * %li row%s\n",
      ptr_txt->nb_columns,
      ptr_txt->nb_columns > 1 ? "s" : "",
      ptr_txt->nb_rows,
      ptr_txt->nb_rows > 1 ? "s" : ""
   );

   // end
   return 0;
}


// ========================================================================================
// replace all \t characters (and 0x0A) by a 0x00 in a .txt file
// input :
//    * txt ID
// output :
//    * 0 if ok, non-zero if error
// ========================================================================================
int txt_replace_tab_by_zero(TXT_ENUM txt_idx)
{
   TXT_S * ptr_txt;
   int   c;
   UBYTE * ptr;


   if ((txt_idx < 0) || (txt_idx >= TXT_MAX))
      return -1;
   ptr_txt = & glb_datas.txt[txt_idx];
   if (ptr_txt->buffer == NULL)
      return -1;

   // replace all tabs by 0x00
   ptr = ptr_txt->buffer;
   c = * ptr;
   while (c != 0x00)
   {
      if ((c == '\t') || (c == 0x0D))
         * ptr = 0x00;
      ptr++;
      c = * ptr;
   }

   // end
   return 0;
}


// ========================================================================================
// search a header column position in a .txt file
// input :
//    * txt pointer
//    * column ID to search
// output :
//    * -1 if error, else column position of the header
// ========================================================================================
int txt_find_column_position(TXT_S * ptr_txt, int idx)
{
   int  length, i;
   char * ptr;


   if (ptr_txt == NULL)
      return -1;

   if (ptr_txt->buffer == NULL)
      return -1;

   if (ptr_txt->header == NULL)
      return -1;

   if ((idx < 0) || (idx >= ptr_txt->nb_header))
      return -1;

   if (ptr_txt->header[idx] == NULL)
   {
      printf("column index %i has a NULL header\n", idx);
      return -1;
   }

   // scan the 1st line and find the header string
   ptr = (char *) ptr_txt->buffer;
   if ((ptr_txt->txt_idx == AUTOMAP) && (idx == AUTOMAP_TYPE3))
   {
      // handle this damn typo error in the header line
      for (i=0; i < ptr_txt->nb_columns; i++)
      {
         if (_stricmp(ptr, ptr_txt->header[idx]) == 0)
         {
            // found it
            // skip this header
            length = strlen(ptr);
            ptr += (length + 1);
            i++;

            // now, search the 2nd "Type2"
            for (; i < ptr_txt->nb_columns; i++)
            {
               if (_stricmp(ptr, ptr_txt->header[idx]) == 0)
               {
                  // found it
                  return i;
               }

               // skip this header
               length = strlen(ptr);
               ptr += (length + 1);
            }
         }
         else
         {
            // skip this header
            length = strlen(ptr);
            ptr += (length + 1);
         }
      }
   }
   else
   {
      for (i=0; i < ptr_txt->nb_columns; i++)
      {
         if (_stricmp(ptr, ptr_txt->header[idx]) == 0)
         {
            // found it
            return i;
         }
         else
         {
            // skip this header
            length = strlen(ptr);
            ptr += (length + 1);
         }
      }
   }

   // end (column header not found)
   return -1;
}


// ========================================================================================
// necessary inits of columns header of all txt
// ========================================================================================
void txt_init_header_names(void)
{
   // Armor.txt + Weapons.txt + Misc.txt
   glb_datas.txt_col_names.awm_header[AWM_NAME]             = "name";
   glb_datas.txt_col_names.awm_header[AWM_VERSION]          = "version";
   glb_datas.txt_col_names.awm_header[AWM_COMPACTSAVE]      = "compactsave";
   glb_datas.txt_col_names.awm_header[AWM_RARITY]           = "rarity";
   glb_datas.txt_col_names.awm_header[AWM_SPAWNABLE]        = "spawnable";
   glb_datas.txt_col_names.awm_header[AWM_MINAC]            = "minac";
   glb_datas.txt_col_names.awm_header[AWM_MAXAC]            = "maxac";
   glb_datas.txt_col_names.awm_header[AWM_ABSORBS]          = "absorbs";
   glb_datas.txt_col_names.awm_header[AWM_SPEED]            = "speed";
   glb_datas.txt_col_names.awm_header[AWM_REQSTR]           = "reqstr";
   glb_datas.txt_col_names.awm_header[AWM_BLOCK]            = "block";
   glb_datas.txt_col_names.awm_header[AWM_DURABILITY]       = "durability";
   glb_datas.txt_col_names.awm_header[AWM_NODURABILITY]     = "nodurability";
   glb_datas.txt_col_names.awm_header[AWM_LEVEL]            = "level";
   glb_datas.txt_col_names.awm_header[AWM_LEVELREQ]         = "levelreq";
   glb_datas.txt_col_names.awm_header[AWM_COST]             = "cost";
   glb_datas.txt_col_names.awm_header[AWM_GAMBLECOST]       = "gamble cost";
   glb_datas.txt_col_names.awm_header[AWM_CODE]             = "code";
   glb_datas.txt_col_names.awm_header[AWM_NAMESTR]          = "namestr";
   glb_datas.txt_col_names.awm_header[AWM_MAGICLVL]         = "magic lvl";
   glb_datas.txt_col_names.awm_header[AWM_AUTOPREFIX]       = "auto prefix";
   glb_datas.txt_col_names.awm_header[AWM_ALTERNATEGFX]     = "alternategfx";
   glb_datas.txt_col_names.awm_header[AWM_OPENBETAGFX]      = "OpenBetaGfx";
   glb_datas.txt_col_names.awm_header[AWM_NORMCODE]         = "normcode";
   glb_datas.txt_col_names.awm_header[AWM_UBERCODE]         = "ubercode";
   glb_datas.txt_col_names.awm_header[AWM_ULTRACODE]        = "ultracode";
   glb_datas.txt_col_names.awm_header[AWM_SPELLOFFSET]      = "spelloffset";
   glb_datas.txt_col_names.awm_header[AWM_COMPONENT]        = "component";
   glb_datas.txt_col_names.awm_header[AWM_INVWIDTH]         = "invwidth";
   glb_datas.txt_col_names.awm_header[AWM_INVHEIGHT]        = "invheight";
   glb_datas.txt_col_names.awm_header[AWM_HASINV]           = "hasinv";
   glb_datas.txt_col_names.awm_header[AWM_GEMSOCKETS]       = "gemsockets";
   glb_datas.txt_col_names.awm_header[AWM_GEMAPPLYTYPE]     = "gemapplytype";
   glb_datas.txt_col_names.awm_header[AWM_FLIPPYFILE]       = "flippyfile";
   glb_datas.txt_col_names.awm_header[AWM_INVFILE]          = "invfile";
   glb_datas.txt_col_names.awm_header[AWM_UNIQUEINVFILE]    = "uniqueinvfile";
   glb_datas.txt_col_names.awm_header[AWM_SETINVFILE]       = "setinvfile";
   glb_datas.txt_col_names.awm_header[AWM_RARM]             = "rArm";
   glb_datas.txt_col_names.awm_header[AWM_LARM]             = "lArm";
   glb_datas.txt_col_names.awm_header[AWM_TORSO]            = "Torso";
   glb_datas.txt_col_names.awm_header[AWM_LEGS]             = "Legs";
   glb_datas.txt_col_names.awm_header[AWM_RSPAD]            = "rSPad";
   glb_datas.txt_col_names.awm_header[AWM_LSPAD]            = "lSPad";
   glb_datas.txt_col_names.awm_header[AWM_USEABLE]          = "useable";
   glb_datas.txt_col_names.awm_header[AWM_THROWABLE]        = "throwable";
   glb_datas.txt_col_names.awm_header[AWM_STACKABLE]        = "stackable";
   glb_datas.txt_col_names.awm_header[AWM_MINSTACK]         = "minstack";
   glb_datas.txt_col_names.awm_header[AWM_MAXSTACK]         = "maxstack";
   glb_datas.txt_col_names.awm_header[AWM_TYPE]             = "type";
   glb_datas.txt_col_names.awm_header[AWM_TYPE2]            = "type2";
   glb_datas.txt_col_names.awm_header[AWM_DROPSOUND]        = "dropsound";
   glb_datas.txt_col_names.awm_header[AWM_DROPSFXFRAME]     = "dropsfxframe";
   glb_datas.txt_col_names.awm_header[AWM_USESOUND]         = "usesound";
   glb_datas.txt_col_names.awm_header[AWM_UNIQUE]           = "unique";
   glb_datas.txt_col_names.awm_header[AWM_TRANSPARENT]      = "transparent";
   glb_datas.txt_col_names.awm_header[AWM_TRANSTBL]         = "transtbl";
   glb_datas.txt_col_names.awm_header[AWM_QUIVERED]         = "quivered";
   glb_datas.txt_col_names.awm_header[AWM_LIGHTRADIUS]      = "lightradius";
   glb_datas.txt_col_names.awm_header[AWM_BELT]             = "belt";
   glb_datas.txt_col_names.awm_header[AWM_QUEST]            = "quest";
   glb_datas.txt_col_names.awm_header[AWM_MISSILETYPE]      = "missiletype";
   glb_datas.txt_col_names.awm_header[AWM_DURWARNING]       = "durwarning";
   glb_datas.txt_col_names.awm_header[AWM_QNTWARNING]       = "qntwarning";
   glb_datas.txt_col_names.awm_header[AWM_MINDAM]           = "mindam";
   glb_datas.txt_col_names.awm_header[AWM_MAXDAM]           = "maxdam";
   glb_datas.txt_col_names.awm_header[AWM_STRBONUS]         = "StrBonus";
   glb_datas.txt_col_names.awm_header[AWM_DEXBONUS]         = "DexBonus";
   glb_datas.txt_col_names.awm_header[AWM_GEMOFFSET]        = "gemoffset";
   glb_datas.txt_col_names.awm_header[AWM_BITFIELD1]        = "bitfield1";
   glb_datas.txt_col_names.awm_header[AWM_CHARSIMIN]        = "CharsiMin";
   glb_datas.txt_col_names.awm_header[AWM_CHARSIMAX]        = "CharsiMax";
   glb_datas.txt_col_names.awm_header[AWM_CHARSIMAGICMIN]   = "CharsiMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_CHARSIMAGICMAX]   = "CharsiMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_CHARSIMAGICLVL]   = "CharsiMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_GHEEDMIN]         = "GheedMin";
   glb_datas.txt_col_names.awm_header[AWM_GHEEDMAX]         = "GheedMax";
   glb_datas.txt_col_names.awm_header[AWM_GHEEDMAGICMIN]    = "GheedMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_GHEEDMAGICMAX]    = "GheedMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_GHEEDMAGICLVL]    = "GheedMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_AKARAMIN]         = "AkaraMin";
   glb_datas.txt_col_names.awm_header[AWM_AKARAMAX]         = "AkaraMax";
   glb_datas.txt_col_names.awm_header[AWM_AKARAMAGICMIN]    = "AkaraMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_AKARAMAGICMAX]    = "AkaraMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_AKARAMAGICLVL]    = "AkaraMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_FARAMIN]          = "FaraMin";
   glb_datas.txt_col_names.awm_header[AWM_FARAMAX]          = "FaraMax";
   glb_datas.txt_col_names.awm_header[AWM_FARAMAGICMIN]     = "FaraMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_FARAMAGICMAX]     = "FaraMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_FARAMAGICLVL]     = "FaraMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_LYSANDERMIN]      = "LysanderMin";
   glb_datas.txt_col_names.awm_header[AWM_LYSANDERMAX]      = "LysanderMax";
   glb_datas.txt_col_names.awm_header[AWM_LYSANDERMAGICMIN] = "LysanderMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_LYSANDERMAGICMAX] = "LysanderMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_LYSANDERMAGICLVL] = "LysanderMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_DROGNANMIN]       = "DrognanMin";
   glb_datas.txt_col_names.awm_header[AWM_DROGNANMAX]       = "DrognanMax";
   glb_datas.txt_col_names.awm_header[AWM_DROGNANMAGICMIN]  = "DrognanMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_DROGNANMAGICMAX]  = "DrognanMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_DROGNANMAGICLVL]  = "DrognanMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_HRATLIMIN]        = "HraltiMin";      // typo
   glb_datas.txt_col_names.awm_header[AWM_HRATLIMAX]        = "HraltiMax";      // typo
   glb_datas.txt_col_names.awm_header[AWM_HRATLIMAGICMIN]   = "HraltiMagicMin"; // typo
   glb_datas.txt_col_names.awm_header[AWM_HRATLIMAGICMAX]   = "HraltiMagicMax"; // typo
   glb_datas.txt_col_names.awm_header[AWM_HRATLIMAGICLVL]   = "HratliMagicLvl"; // ok
   glb_datas.txt_col_names.awm_header[AWM_ALKORMIN]         = "AlkorMin";
   glb_datas.txt_col_names.awm_header[AWM_ALKORMAX]         = "AlkorMax";
   glb_datas.txt_col_names.awm_header[AWM_ALKORMAGICMIN]    = "AlkorMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_ALKORMAGICMAX]    = "AlkorMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_ALKORMAGICLVL]    = "AlkorMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_ORMUSMIN]         = "OrmusMin";
   glb_datas.txt_col_names.awm_header[AWM_ORMUSMAX]         = "OrmusMax";
   glb_datas.txt_col_names.awm_header[AWM_ORMUSMAGICMIN]    = "OrmusMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_ORMUSMAGICMAX]    = "OrmusMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_ORMUSMAGICLVL]    = "OrmusMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_ELZIXMIN]         = "ElzixMin";
   glb_datas.txt_col_names.awm_header[AWM_ELZIXMAX]         = "ElzixMax";
   glb_datas.txt_col_names.awm_header[AWM_ELZIXMAGICMIN]    = "ElzixMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_ELZIXMAGICMAX]    = "ElzixMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_ELZIXMAGICLVL]    = "ElzixMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_ASHEARAMIN]       = "AshearaMin";
   glb_datas.txt_col_names.awm_header[AWM_ASHEARAMAX]       = "AshearaMax";
   glb_datas.txt_col_names.awm_header[AWM_ASHEARAMAGICMIN]  = "AshearaMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_ASHEARAMAGICMAX]  = "AshearaMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_ASHEARAMAGICLVL]  = "AshearaMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_CAINMIN]          = "CainMin";
   glb_datas.txt_col_names.awm_header[AWM_CAINMAX]          = "CainMax";
   glb_datas.txt_col_names.awm_header[AWM_CAINMAGICMIN]     = "CainMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_CAINMAGICMAX]     = "CainMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_CAINMAGICLVL]     = "CainMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_HALBUMIN]         = "HalbuMin";
   glb_datas.txt_col_names.awm_header[AWM_HALBUMAX]         = "HalbuMax";
   glb_datas.txt_col_names.awm_header[AWM_HALBUMAGICMIN]    = "HalbuMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_HALBUMAGICMAX]    = "HalbuMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_HALBUMAGICLVL]    = "HalbuMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_JAMELLAMIN]       = "JamellaMin";
   glb_datas.txt_col_names.awm_header[AWM_JAMELLAMAX]       = "JamellaMax";
   glb_datas.txt_col_names.awm_header[AWM_JAMELLAMAGICMIN]  = "JamellaMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_JAMELLAMAGICMAX]  = "JamellaMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_JAMELLAMAGICLVL]  = "JamellaMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_LARZUKMIN]        = "LarzukMin";
   glb_datas.txt_col_names.awm_header[AWM_LARZUKMAX]        = "LarzukMax";
   glb_datas.txt_col_names.awm_header[AWM_LARZUKMAGICMIN]   = "LarzukMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_LARZUKMAGICMAX]   = "LarzukMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_LARZUKMAGICLVL]   = "LarzukMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_MALAHMIN]         = "MalahMin";
   glb_datas.txt_col_names.awm_header[AWM_MALAHMAX]         = "MalahMax";
   glb_datas.txt_col_names.awm_header[AWM_MALAHMAGICMIN]    = "MalahMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_MALAHMAGICMAX]    = "MalahMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_MALAHMAGICLVL]    = "MalahMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_DREHYAMIN]        = "DrehyaMin";
   glb_datas.txt_col_names.awm_header[AWM_DREHYAMAX]        = "DrehyaMax";
   glb_datas.txt_col_names.awm_header[AWM_DREHYAMAGICMIN]   = "DrehyaMagicMin";
   glb_datas.txt_col_names.awm_header[AWM_DREHYAMAGICMAX]   = "DrehyaMagicMax";
   glb_datas.txt_col_names.awm_header[AWM_DREHYAMAGICLVL]   = "DrehyaMagicLvl";
   glb_datas.txt_col_names.awm_header[AWM_SOURCEART]        = "Source Art";
   glb_datas.txt_col_names.awm_header[AWM_GAMEART]          = "Game Art";
   glb_datas.txt_col_names.awm_header[AWM_TRANSFORM]        = "Transform";
   glb_datas.txt_col_names.awm_header[AWM_INVTRANS]         = "InvTrans";
   glb_datas.txt_col_names.awm_header[AWM_SKIPNAME]         = "SkipName";
   glb_datas.txt_col_names.awm_header[AWM_NIGHTMAREUPGRADE] = "NightmareUpgrade";
   glb_datas.txt_col_names.awm_header[AWM_HELLUPGRADE]      = "HellUpgrade";
   glb_datas.txt_col_names.awm_header[AWM_NAMEABLE]         = "nameable";

   glb_datas.txt_col_names.awm_header[AWM_1OR2HANDED]       = "1or2handed";
   glb_datas.txt_col_names.awm_header[AWM_2HANDED]          = "2handed";
   glb_datas.txt_col_names.awm_header[AWM_2HANDMINDAM]      = "2handmindam";
   glb_datas.txt_col_names.awm_header[AWM_2HANDMAXDAM]      = "2handmaxdam";
   glb_datas.txt_col_names.awm_header[AWM_MINMISDAM]        = "minmisdam";
   glb_datas.txt_col_names.awm_header[AWM_MAXMISDAM]        = "maxmisdam";
   glb_datas.txt_col_names.awm_header[AWM_RANGEADDER]       = "rangeadder";
   glb_datas.txt_col_names.awm_header[AWM_REQDEX]           = "reqdex";
   glb_datas.txt_col_names.awm_header[AWM_WCLASS]           = "wclass";
   glb_datas.txt_col_names.awm_header[AWM_2HANDEDWCLASS]    = "2handedwclass";
   glb_datas.txt_col_names.awm_header[AWM_HITCLASS]         = "hit class";
   glb_datas.txt_col_names.awm_header[AWM_SPAWNSTACK]       = "spawnstack";
   glb_datas.txt_col_names.awm_header[AWM_QUESTDIFFCHECK]   = "questdiffcheck";
   glb_datas.txt_col_names.awm_header[AWM_PERMSTOREITEM]    = "PermStoreItem";

   glb_datas.txt_col_names.awm_header[AWM_SZFLAVORTEXT]     = "szFlavorText";
   glb_datas.txt_col_names.awm_header[AWM_SPECIAL]          = "special";
   glb_datas.txt_col_names.awm_header[AWM_TRANSMOGRIFY]     = "Transmogrify";
   glb_datas.txt_col_names.awm_header[AWM_TMOGTYPE]         = "TMogType";
   glb_datas.txt_col_names.awm_header[AWM_TMOGMIN]          = "TMogMin";
   glb_datas.txt_col_names.awm_header[AWM_TMOGMAX]          = "TMogMax";
   glb_datas.txt_col_names.awm_header[AWM_AUTOBELT]         = "autobelt";
   glb_datas.txt_col_names.awm_header[AWM_SPELLICON]        = "spellicon";
   glb_datas.txt_col_names.awm_header[AWM_PSPELL]           = "pSpell";
   glb_datas.txt_col_names.awm_header[AWM_STATE]            = "state";
   glb_datas.txt_col_names.awm_header[AWM_CSTATE1]          = "cstate1";
   glb_datas.txt_col_names.awm_header[AWM_CSTATE2]          = "cstate2";
   glb_datas.txt_col_names.awm_header[AWM_LEN]              = "len";
   glb_datas.txt_col_names.awm_header[AWM_STAT1]            = "stat1";
   glb_datas.txt_col_names.awm_header[AWM_CALC1]            = "calc1";
   glb_datas.txt_col_names.awm_header[AWM_STAT2]            = "stat2";
   glb_datas.txt_col_names.awm_header[AWM_CALC2]            = "calc2";
   glb_datas.txt_col_names.awm_header[AWM_STAT3]            = "stat3";
   glb_datas.txt_col_names.awm_header[AWM_CALC3]            = "calc3";
   glb_datas.txt_col_names.awm_header[AWM_SPELLDESC]        = "spelldesc";
   glb_datas.txt_col_names.awm_header[AWM_SPELLDESCSTR]     = "spelldescstr";
   glb_datas.txt_col_names.awm_header[AWM_SPELLDESCCALC]    = "spelldesccalc";
   glb_datas.txt_col_names.awm_header[AWM_BETTERGEM]        = "BetterGem";
   glb_datas.txt_col_names.awm_header[AWM_MULTIBUY]         = "multibuy";

   glb_datas.txt_col_names.awm_header[AWM_SUBTYPE]          = "subtype";

   
   // ArmType.txt
   glb_datas.txt_col_names.armtype_header[ARMTYPE_NAME]  = "Name";
   glb_datas.txt_col_names.armtype_header[ARMTYPE_TOKEN] = "Token";

   // AutoMagic.txt
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_NAME]           = "Name";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_VERSION]        = "version";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_SPAWNABLE]      = "spawnable";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_RARE]           = "rare";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_LEVEL]          = "level";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MAXLEVEL]       = "maxlevel";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_LEVELREQ]       = "levelreq";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_CLASSSPECIFIC]  = "classspecific";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_CLASS]          = "class";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_CLASSLEVELREQ]  = "classlevelreq";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_FREQUENCY]      = "frequency";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_GROUP]          = "group";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD1CODE]       = "mod1code";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD1PARAM]      = "mod1param";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD1MIN]        = "mod1min";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD1MAX]        = "mod1max";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD2CODE]       = "mod2code";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD2PARAM]      = "mod2param";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD2MIN]        = "mod2min";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD2MAX]        = "mod2max";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD3CODE]       = "mod3code";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD3PARAM]      = "mod3param";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD3MIN]        = "mod3min";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MOD3MAX]        = "mod3max";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_TRANSFORM]      = "transform";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_TRANSFORMCOLOR] = "transformcolor";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE1]         = "itype1";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE2]         = "itype2";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE3]         = "itype3";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE4]         = "itype4";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE5]         = "itype5";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE6]         = "itype6";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ITYPE7]         = "itype7";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ETYPE1]         = "etype1";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ETYPE2]         = "etype2";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ETYPE3]         = "etype3";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ETYPE4]         = "etype4";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ETYPE5]         = "etype5";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_DIVIDE]         = "divide";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_MULTIPLY]       = "multiply";
   glb_datas.txt_col_names.automagic_header[AUTOMAGIC_ADD]            = "add";

   // AutoMap.txt
   glb_datas.txt_col_names.automap_header[AUTOMAP_LEVELNAME]     = "LevelName";
   glb_datas.txt_col_names.automap_header[AUTOMAP_TILENAME]      = "TileName";
   glb_datas.txt_col_names.automap_header[AUTOMAP_STYLE]         = "Style";
   glb_datas.txt_col_names.automap_header[AUTOMAP_STARTSEQUENCE] = "StartSequence";
   glb_datas.txt_col_names.automap_header[AUTOMAP_ENDSEQUENCE]   = "EndSequence";
   glb_datas.txt_col_names.automap_header[AUTOMAP_TYPE1]         = "Type1";
   glb_datas.txt_col_names.automap_header[AUTOMAP_CEL1]          = "Cel1";
   glb_datas.txt_col_names.automap_header[AUTOMAP_TYPE2]         = "Type2";
   glb_datas.txt_col_names.automap_header[AUTOMAP_CEL2]          = "Cel2";
   glb_datas.txt_col_names.automap_header[AUTOMAP_TYPE3]         = "Type2"; // bad typo
   glb_datas.txt_col_names.automap_header[AUTOMAP_CEL3]          = "Cel3";
   glb_datas.txt_col_names.automap_header[AUTOMAP_TYPE4]         = "Type4";
   glb_datas.txt_col_names.automap_header[AUTOMAP_CEL4]          = "Cel4";

   // belts.txt
   glb_datas.txt_col_names.belts_header[BELTS_NAME]       = "name";
   glb_datas.txt_col_names.belts_header[BELTS_NUMBOXES]   = "numboxes";
   glb_datas.txt_col_names.belts_header[BELTS_BOXWIDTH]   = "boxwidth";
   glb_datas.txt_col_names.belts_header[BELTS_BOXHEIGHT]  = "boxheight";
   glb_datas.txt_col_names.belts_header[BELTS_BOX1LEFT]   = "box1left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX1RIGHT]  = "box1right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX1TOP]    = "box1top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX1BOTTOM] = "box1bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX2LEFT]   = "box2left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX2RIGHT]  = "box2right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX2TOP]    = "box2top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX2BOTTOM] = "box2bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX3LEFT]   = "box3left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX3RIGHT]  = "box3right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX3TOP]    = "box3top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX3BOTTOM] = "box3bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX4LEFT]   = "box4left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX4RIGHT]  = "box4right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX4TOP]    = "box4top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX4BOTTOM] = "box4bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX5LEFT]   = "box5left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX5RIGHT]  = "box5right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX5TOP]    = "box5top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX5BOTTOM] = "box5bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX6LEFT]   = "box6left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX6RIGHT]  = "box6right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX6TOP]    = "box6top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX6BOTTOM] = "box6bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX7LEFT]   = "box7left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX7RIGHT]  = "box7right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX7TOP]    = "box7top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX7BOTTOM] = "box7bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX8LEFT]   = "box8left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX8RIGHT]  = "box8right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX8TOP]    = "box8top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX8BOTTOM] = "box8bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX9LEFT]   = "box9left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX9RIGHT]  = "box9right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX9TOP]    = "box9top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX9BOTTOM] = "box9bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX10LEFT]   = "box10left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX10RIGHT]  = "box10right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX10TOP]    = "box10top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX10BOTTOM] = "box10bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX11LEFT]   = "box11left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX11RIGHT]  = "box11right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX11TOP]    = "box11top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX11BOTTOM] = "box11bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX12LEFT]   = "box12left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX12RIGHT]  = "box12right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX12TOP]    = "box12top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX12BOTTOM] = "box12bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX13LEFT]   = "box13left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX13RIGHT]  = "box13right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX13TOP]    = "box13top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX13BOTTOM] = "box13bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX14LEFT]   = "box14left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX14RIGHT]  = "box14right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX14TOP]    = "box14top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX14BOTTOM] = "box14bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX15LEFT]   = "box15left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX15RIGHT]  = "box15right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX15TOP]    = "box15top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX15BOTTOM] = "box15bottom";
   glb_datas.txt_col_names.belts_header[BELTS_BOX16LEFT]   = "box16left";
   glb_datas.txt_col_names.belts_header[BELTS_BOX16RIGHT]  = "box16right";
   glb_datas.txt_col_names.belts_header[BELTS_BOX16TOP]    = "box16top";
   glb_datas.txt_col_names.belts_header[BELTS_BOX16BOTTOM] = "box16bottom";

   // BodyLocs.txt
   glb_datas.txt_col_names.bodylocs_header[BODYLOCS_BODYLOCATION] = "Body Location";
   glb_datas.txt_col_names.bodylocs_header[BODYLOCS_CODE]         = "Code";

   // Books.txt
   glb_datas.txt_col_names.books_header[BOOKS_NAME]            = "Name";
   glb_datas.txt_col_names.books_header[BOOKS_NAMCO]           = "Namco";
   glb_datas.txt_col_names.books_header[BOOKS_COMPLETED]       = "Completed";
   glb_datas.txt_col_names.books_header[BOOKS_SCROLLSPELLCODE] = "ScrollSpellCode";
   glb_datas.txt_col_names.books_header[BOOKS_BOOKSPELLCODE]   = "BookSpellCode";
   glb_datas.txt_col_names.books_header[BOOKS_PSPELL]          = "pSpell";
   glb_datas.txt_col_names.books_header[BOOKS_SPELLICON]       = "SpellIcon";
   glb_datas.txt_col_names.books_header[BOOKS_SCROLLSKILL]     = "ScrollSkill";
   glb_datas.txt_col_names.books_header[BOOKS_BOOKSKILL]       = "BookSkill";
   glb_datas.txt_col_names.books_header[BOOKS_BASECOST]        = "BaseCost";
   glb_datas.txt_col_names.books_header[BOOKS_COSTPERCHARGE]   = "CostPerCharge";

   // CharStats.txt
   glb_datas.txt_col_names.charstats_header[CHARSTATS_CLASS]              = "class";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STR]                = "str";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_DEX]                = "dex";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_INT]                = "int";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_VIT]                = "vit";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_TOT]                = "tot";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STAMINA]            = "stamina";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_HPADD]              = "hpadd";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_PERCENTSTR]         = "PercentStr";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_PERCENTDEX]         = "PercentDex";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_PERCENTINT]         = "PercentInt";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_PERCENTVIT]         = "PercentVit";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_MANAREGEN]          = "ManaRegen";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_TOHITFACTOR]        = "ToHitFactor";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_WALKVELOCITY]       = "WalkVelocity";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_RUNVELOCITY]        = "RunVelocity";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_RUNDRAIN]           = "RunDrain";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_COMMENT]            = "Comment";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_LIFEPERLEVEL]       = "LifePerLevel";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STAMINAPERLEVEL]    = "StaminaPerLevel";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_MANAPERLEVEL]       = "ManaPerLevel";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_LIFEPERVITALITY]    = "LifePerVitality";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STAMINAPERVITALITY] = "StaminaPerVitality";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_MANAPERMAGIC]       = "ManaPerMagic";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STATPERLEVEL]       = "StatPerLevel";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_WALK]               = "#walk";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_RUN]                = "#run";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SWING]              = "#swing";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SPELL]              = "#spell";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_GETHIT]             = "#gethit";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_BOW]                = "#bow";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_BLOCKFACTOR]        = "BlockFactor";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STARTSKILL]         = "StartSkill";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL1]             = "Skill 1";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL2]             = "Skill 2";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL3]             = "Skill 3";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL4]             = "Skill 4";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL5]             = "Skill 5";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL6]             = "Skill 6";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL7]             = "Skill 7";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL8]             = "Skill 8";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL9]             = "Skill 9";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_SKILL10]            = "Skill 10";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STRALLSKILLS]       = "StrAllSkills";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STRSKILLTAB1]       = "StrSkillTab1";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STRSKILLTAB2]       = "StrSkillTab2";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STRSKILLTAB3]       = "StrSkillTab3";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_STRCLASSONLY]       = "StrClassOnly";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_BASEWCLASS]         = "baseWClass";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM1]              = "item1";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM1LOC]           = "item1loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM1COUNT]         = "item1count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM2]              = "item2";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM2LOC]           = "item2loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM2COUNT]         = "item2count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM3]              = "item3";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM3LOC]           = "item3loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM3COUNT]         = "item3count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM4]              = "item4";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM4LOC]           = "item4loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM4COUNT]         = "item4count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM5]              = "item5";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM5LOC]           = "item5loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM5COUNT]         = "item5count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM6]              = "item6";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM6LOC]           = "item6loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM6COUNT]         = "item6count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM7]              = "item7";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM7LOC]           = "item7loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM7COUNT]         = "item7count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM8]              = "item8";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM8LOC]           = "item8loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM8COUNT]         = "item8count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM9]              = "item9";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM9LOC]           = "item9loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM9COUNT]         = "item9count";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM10]             = "item10";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM10LOC]          = "item10loc";
   glb_datas.txt_col_names.charstats_header[CHARSTATS_ITEM10COUNT]        = "item10count";

   // Colors.txt
   glb_datas.txt_col_names.colors_header[COLORS_TRANSFORMCOLOR] = "Transform Color";
   glb_datas.txt_col_names.colors_header[COLORS_CODE]           = "Code";

   // CompCode.txt
   glb_datas.txt_col_names.compcode_header[COMPCODE_COMPONENT] = "component";
   glb_datas.txt_col_names.compcode_header[COMPCODE_CODE]      = "code";

   // Composit.txt
   glb_datas.txt_col_names.composit_header[COMPOSIT_NAME]  = "Name";
   glb_datas.txt_col_names.composit_header[COMPOSIT_TOKEN] = "Token";

   // CubeMain.txt
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_DESCRIPTION] = "description";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_ENABLED]     = "enabled";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_LADDER]      = "ladder";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MINDIFF]     = "min diff";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_VERSION]     = "version";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_OP]          = "op";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_PARAM]       = "param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_VALUE]       = "value";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CLASS]       = "class";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_NUMINPUTS]   = "numinputs";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT1]      = "input 1";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT2]      = "input 2";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT3]      = "input 3";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT4]      = "input 4";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT5]      = "input 5";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT6]      = "input 6";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_INPUT7]      = "input 7";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_OUTPUT]      = "output";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_LVL]         = "lvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_PLVL]        = "plvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_ILVL]        = "ilvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD1]        = "mod 1";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD1CHANCE]  = "mod 1 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD1PARAM]   = "mod 1 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD1MIN]     = "mod 1 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD1MAX]     = "mod 1 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD2]        = "mod 2";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD2CHANCE]  = "mod 2 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD2PARAM]   = "mod 2 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD2MIN]     = "mod 2 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD2MAX]     = "mod 2 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD3]        = "mod 3";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD3CHANCE]  = "mod 3 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD3PARAM]   = "mod 3 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD3MIN]     = "mod 3 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD3MAX]     = "mod 3 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD4]        = "mod 4";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD4CHANCE]  = "mod 4 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD4PARAM]   = "mod 4 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD4MIN]     = "mod 4 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD4MAX]     = "mod 4 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD5]        = "mod 5";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD5CHANCE]  = "mod 5 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD5PARAM]   = "mod 5 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD5MIN]     = "mod 5 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_MOD5MAX]     = "mod 5 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_OUTPUTB]     = "output b";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BLVL]        = "b lvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BPLVL]       = "b plvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BILVL]       = "b ilvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD1]       = "b mod 1";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD1CHANCE] = "b mod 1 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD1PARAM]  = "b mod 1 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD1MIN]    = "b mod 1 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD1MAX]    = "b mod 1 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD2]       = "b mod 2";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD2CHANCE] = "b mod 2 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD2PARAM]  = "b mod 2 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD2MIN]    = "b mod 2 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD2MAX]    = "b mod 2 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD3]       = "b mod 3";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD3CHANCE] = "b mod 3 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD3PARAM]  = "b mod 3 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD3MIN]    = "b mod 3 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD3MAX]    = "b mod 3 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD4]       = "b mod 4";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD4CHANCE] = "b mod 4 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD4PARAM]  = "b mod 4 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD4MIN]    = "b mod 4 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD4MAX]    = "b mod 4 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD5]       = "b mod 5";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD5CHANCE] = "b mod 5 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD5PARAM]  = "b mod 5 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD5MIN]    = "b mod 5 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_BMOD5MAX]    = "b mod 5 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_OUTPUTC]     = "output c";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CLVL]        = "c lvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CPLVL]       = "c plvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CILVL]       = "c ilvl";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD1]       = "c mod 1";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD1CHANCE] = "c mod 1 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD1PARAM]  = "c mod 1 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD1MIN]    = "c mod 1 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD1MAX]    = "c mod 1 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD2]       = "c mod 2";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD2CHANCE] = "c mod 2 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD2PARAM]  = "c mod 2 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD2MIN]    = "c mod 2 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD2MAX]    = "c mod 2 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD3]       = "c mod 3";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD3CHANCE] = "c mod 3 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD3PARAM]  = "c mod 3 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD3MIN]    = "c mod 3 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD3MAX]    = "c mod 3 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD4]       = "c mod 4";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD4CHANCE] = "c mod 4 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD4PARAM]  = "c mod 4 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD4MIN]    = "c mod 4 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD4MAX]    = "c mod 4 max";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD5]       = "c mod 5";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD5CHANCE] = "c mod 5 chance";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD5PARAM]  = "c mod 5 param";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD5MIN]    = "c mod 5 min";
   glb_datas.txt_col_names.cubemain_header[CUBEMAIN_CMOD5MAX]    = "c mod 5 max";

   // CubeMod.txt
   glb_datas.txt_col_names.cubemod_header[CUBEMOD_CUBEMODIFIERTYPE] = "cube modifier type";
   glb_datas.txt_col_names.cubemod_header[CUBEMOD_CODE]             = "Code";

   // CubeType.txt
   glb_datas.txt_col_names.cubetype_header[CUBETYPE_CUBEITEMCLASS] = "cube item class";
   glb_datas.txt_col_names.cubetype_header[CUBETYPE_CODE]          = "Code";

   // DiffiultyLevels.txt
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_NAME]                      = "Name";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_RESISTPENALTY]             = "ResistPenalty";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_DEATHEXPPENALTY]           = "DeathExpPenalty";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_UBERCODEODDSNORMAL]        = "UberCodeOddsNormal";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_UBERCODEODDSGOOD]          = "UberCodeOddsGood";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_ULTRACODEODDSNORMAL]       = "UltraCodeOddsNormal";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_ULTRACODEODDSGOOD]         = "UltraCodeOddsGood";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_MONSTERSKILLBONUS]         = "MonsterSkillBonus";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_MONSTERFREEZEDIVIZOR]      = "MonsterFreezeDivisor";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_MONSTERCOLDDIVISOR]        = "MonsterColdDivisor";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_AICURSEDIVISOR]            = "AiCurseDivisor";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_LIFESTEALDIVISOR]          = "LifeStealDivisor";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_MANASTEALDIVISOR]          = "ManaStealDivisor";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_UNIQUEDAMAGEBONUS]         = "UniqueDamageBonus";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_CHAMPIONDAMAGEBONUS]       = "ChampionDamageBonus";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_HIREABLEBOSSDAMAGEPERCENT] = "HireableBossDamagePercent";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_MONSTERCEDAMAGEPERCENT]    = "MonsterCEDamagePercent";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_STATICFIELDMIN]            = "StaticFieldMin";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_GAMBLERARE]                = "GambleRare";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_GAMBLESET]                 = "GambleSet";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_GAMBLEUNIQUE]              = "GambleUnique";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_GAMBLEUBER]                = "GambleUber";
   glb_datas.txt_col_names.difficultylevels_header[DIFFICULTYLEVELS_GAMBLEULTRA]               = "GambleUltra";

   // ElemTypes.txt
   glb_datas.txt_col_names.elemtypes_header[ELEMTYPES_ELEMENTALTYPE] = "Elemental Type";
   glb_datas.txt_col_names.elemtypes_header[ELEMTYPES_CODE]          = "Code";
   
   // Events.txt
   glb_datas.txt_col_names.events_header[EVENTS_EVENT] = "event";

   // Experience.txt
   glb_datas.txt_col_names.experience_header[EXPERIENCE_LEVEL]       = "Level";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_AMAZON]      = "Amazon";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_SORCERESS]   = "Sorceress";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_NECROMANCER] = "Necromancer";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_PALADIN]     = "Paladin";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_BARBARIAN]   = "Barbarian";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_DRUID]       = "Druid";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_ASSASSIN]    = "Assassin";
   glb_datas.txt_col_names.experience_header[EXPERIENCE_EXPRATIO]    = "ExpRatio";

   // Gamble.txt
   glb_datas.txt_col_names.gamble_header[GAMBLE_NAME] = "name";
   glb_datas.txt_col_names.gamble_header[GAMBLE_CODE] = "code";

   // Gems.txt
   glb_datas.txt_col_names.gems_header[GEMS_NAME]            = "name";
   glb_datas.txt_col_names.gems_header[GEMS_LETTER]          = "letter";
   glb_datas.txt_col_names.gems_header[GEMS_TRANSFORM]       = "transform";
   glb_datas.txt_col_names.gems_header[GEMS_CODE]            = "code";
   glb_datas.txt_col_names.gems_header[GEMS_NUMMODS]         = "nummods";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD1CODE]  = "weaponMod1Code";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD1PARAM] = "weaponMod1Param";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD1MIN]   = "weaponMod1Min";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD1MAX]   = "weaponMod1Max";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD2CODE]  = "weaponMod2Code";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD2PARAM] = "weaponMod2Param";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD2MIN]   = "weaponMod2Min";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD2MAX]   = "weaponMod2Max";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD3CODE]  = "weaponMod3Code";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD3PARAM] = "weaponMod3Param";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD3MIN]   = "weaponMod3Min";
   glb_datas.txt_col_names.gems_header[GEMS_WEAPONMOD3MAX]   = "weaponMod3Max";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD1CODE]    = "helmMod1Code";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD1PARAM]   = "helmMod1Param";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD1MIN]     = "helmMod1Min";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD1MAX]     = "helmMod1Max";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD2CODE]    = "helmMod2Code";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD2PARAM]   = "helmMod2Param";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD2MIN]     = "helmMod2Min";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD2MAX]     = "helmMod2Max";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD3CODE]    = "helmMod3Code";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD3PARAM]   = "helmMod3Param";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD3MIN]     = "helmMod3Min";
   glb_datas.txt_col_names.gems_header[GEMS_HELMMOD3MAX]     = "helmMod3Max";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD1CODE]  = "shieldMod1Code";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD1PARAM] = "shieldMod1Param";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD1MIN]   = "shieldMod1Min";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD1MAX]   = "shieldMod1Max";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD2CODE]  = "shieldMod2Code";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD2PARAM] = "shieldMod2Param";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD2MIN]   = "shieldMod2Min";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD2MAX]   = "shieldMod2Max";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD3CODE]  = "shieldMod3Code";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD3PARAM] = "shieldMod3Param";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD3MIN]   = "shieldMod3Min";
   glb_datas.txt_col_names.gems_header[GEMS_SHIELDMOD3MAX]   = "shieldMod3Max";

   // HireDesc.txt
   glb_datas.txt_col_names.hiredesc_header[HIREDESC_HIRELINGDESCRIPTION] = "Hireling Description";
   glb_datas.txt_col_names.hiredesc_header[HIREDESC_CODE]                = "Code";
   
   // Hireling.txt
   glb_datas.txt_col_names.hireling_header[HIRELING_HIRELING]      = "Hireling";
   glb_datas.txt_col_names.hireling_header[HIRELING_SUBTYPE]       = "SubType";
   glb_datas.txt_col_names.hireling_header[HIRELING_VERSION]       = "Version";
   glb_datas.txt_col_names.hireling_header[HIRELING_ID]            = "Id";
   glb_datas.txt_col_names.hireling_header[HIRELING_CLASS]         = "Class";
   glb_datas.txt_col_names.hireling_header[HIRELING_ACT]           = "Act";
   glb_datas.txt_col_names.hireling_header[HIRELING_DIFFICULTY]    = "Difficulty";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL]         = "Level";
   glb_datas.txt_col_names.hireling_header[HIRELING_SELLER]        = "Seller";
   glb_datas.txt_col_names.hireling_header[HIRELING_NAMEFIRST]     = "NameFirst";
   glb_datas.txt_col_names.hireling_header[HIRELING_NAMELAST]      = "NameLast";
   glb_datas.txt_col_names.hireling_header[HIRELING_GOLD]          = "Gold";
   glb_datas.txt_col_names.hireling_header[HIRELING_EXPLVL]        = "Exp/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_HP]            = "HP";
   glb_datas.txt_col_names.hireling_header[HIRELING_HPLVL]         = "HP/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_DEFENSE]       = "Defense";
   glb_datas.txt_col_names.hireling_header[HIRELING_DEFLVL]        = "Def/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_STR]           = "Str";
   glb_datas.txt_col_names.hireling_header[HIRELING_STRLVL]        = "Str/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_DEX]           = "Dex";
   glb_datas.txt_col_names.hireling_header[HIRELING_DEXLVL]        = "Dex/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_AR]            = "AR";
   glb_datas.txt_col_names.hireling_header[HIRELING_ARLVL]         = "AR/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_SHARE]         = "Share";
   glb_datas.txt_col_names.hireling_header[HIRELING_DMGMIN]        = "Dmg-Min";
   glb_datas.txt_col_names.hireling_header[HIRELING_DMGMAX]        = "Dmg-Max";
   glb_datas.txt_col_names.hireling_header[HIRELING_DMGLVL]        = "Dmg/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_RESIST]        = "Resist";
   glb_datas.txt_col_names.hireling_header[HIRELING_RESISTLVL]     = "Resist/Lvl";
   glb_datas.txt_col_names.hireling_header[HIRELING_WTYPE1]        = "WType1";
   glb_datas.txt_col_names.hireling_header[HIRELING_WTYPE2]        = "WType2";
   glb_datas.txt_col_names.hireling_header[HIRELING_HIREDESC]      = "HireDesc";
   glb_datas.txt_col_names.hireling_header[HIRELING_DEFAULTCHANCE] = "DefaultChance";
   glb_datas.txt_col_names.hireling_header[HIRELING_SKILL1]        = "Skill1";
   glb_datas.txt_col_names.hireling_header[HIRELING_MODE1]         = "Mode1";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCE1]       = "Chance1";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCEPERLVL1] = "ChancePerLvl1";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL1]        = "Level1";
   glb_datas.txt_col_names.hireling_header[HIRELING_LVLPERLVL1]    = "LvlPerLvl1";
   glb_datas.txt_col_names.hireling_header[HIRELING_SKILL2]        = "Skill2";
   glb_datas.txt_col_names.hireling_header[HIRELING_MODE2]         = "Mode2";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCE2]       = "Chance2";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCEPERLVL2] = "ChancePerLvl2";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL2]        = "Level2";
   glb_datas.txt_col_names.hireling_header[HIRELING_LVLPERLVL2]    = "LvlPerLvl2";
   glb_datas.txt_col_names.hireling_header[HIRELING_SKILL3]        = "Skill3";
   glb_datas.txt_col_names.hireling_header[HIRELING_MODE3]         = "Mode3";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCE3]       = "Chance3";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCEPERLVL3] = "ChancePerLvl3";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL3]        = "Level3";
   glb_datas.txt_col_names.hireling_header[HIRELING_LVLPERLVL3]    = "LvlPerLvl3";
   glb_datas.txt_col_names.hireling_header[HIRELING_SKILL4]        = "Skill4";
   glb_datas.txt_col_names.hireling_header[HIRELING_MODE4]         = "Mode4";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCE4]       = "Chance4";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCEPERLVL4] = "ChancePerLvl4";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL4]        = "Level4";
   glb_datas.txt_col_names.hireling_header[HIRELING_LVLPERLVL4]    = "LvlPerLvl4";
   glb_datas.txt_col_names.hireling_header[HIRELING_SKILL5]        = "Skill5";
   glb_datas.txt_col_names.hireling_header[HIRELING_MODE5]         = "Mode5";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCE5]       = "Chance5";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCEPERLVL5] = "ChancePerLvl5";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL5]        = "Level5";
   glb_datas.txt_col_names.hireling_header[HIRELING_LVLPERLVL5]    = "LvlPerLvl5";
   glb_datas.txt_col_names.hireling_header[HIRELING_SKILL6]        = "Skill6";
   glb_datas.txt_col_names.hireling_header[HIRELING_MODE6]         = "Mode6";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCE6]       = "Chance6";
   glb_datas.txt_col_names.hireling_header[HIRELING_CHANCEPERLVL6] = "ChancePerLvl6";
   glb_datas.txt_col_names.hireling_header[HIRELING_LEVEL6]        = "Level6";
   glb_datas.txt_col_names.hireling_header[HIRELING_LVLPERLVL6]    = "LvlPerLvl6";   
   glb_datas.txt_col_names.hireling_header[HIRELING_HEAD]          = "Head";
   glb_datas.txt_col_names.hireling_header[HIRELING_TORSO]         = "Torso";
   glb_datas.txt_col_names.hireling_header[HIRELING_WEAPON]        = "Weapon";
   glb_datas.txt_col_names.hireling_header[HIRELING_SHIELD]        = "Shield";

   // HitClass.txt
   glb_datas.txt_col_names.hitclass_header[HITCLASS_HITCLASS] = "Hit Class";
   glb_datas.txt_col_names.hitclass_header[HITCLASS_CODE]     = "Code";

   // Inventory.txt
   glb_datas.txt_col_names.inventory_header[INVENTORY_CLASS]         = "class";
   glb_datas.txt_col_names.inventory_header[INVENTORY_INVLEFT]       = "invLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_INVRIGHT]      = "invRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_INVTOP]        = "invTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_INVBOTTOM]     = "invBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDX]         = "gridX";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDY]         = "gridY";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDLEFT]      = "gridLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDRIGHT]     = "gridRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDTOP]       = "gridTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDBOTTOM]    = "gridBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDBOXWIDTH]  = "gridBoxWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GRIDBOXHEIGHT] = "gridBoxHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RARMLEFT]      = "rArmLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RARMRIGHT]     = "rArmRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RARMTOP]       = "rArmTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RARMBOTTOM]    = "rArmBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RARMWIDTH]     = "rArmWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RARMHEIGHT]    = "rArmHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_TORSOLEFT]     = "torsoLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_TORSORIGHT]    = "torsoRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_TORSOTOP]      = "torsoTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_TORSOBOTTOM]   = "torsoBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_TORSOWIDTH]    = "torsoWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_TORSOHEIGHT]   = "torsoHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LARMLEFT]      = "lArmLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LARMRIGHT]     = "lArmRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LARMTOP]       = "lArmTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LARMBOTTOM]    = "lArmBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LARMWIDTH]     = "lArmWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LARMHEIGHT]    = "lArmHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_HEADLEFT]      = "headLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_HEADRIGHT]     = "headRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_HEADTOP]       = "headTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_HEADBOTTOM]    = "headBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_HEADWIDTH]     = "headWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_HEADHEIGHT]    = "headHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_NECKLEFT]      = "neckLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_NECKRIGHT]     = "neckRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_NECKTOP]       = "neckTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_NECKBOTTOM]    = "neckBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_NECKWIDTH]     = "neckWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_NECKHEIGHT]    = "neckHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RHANDLEFT]     = "rHandLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RHANDRIGHT]    = "rHandRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RHANDTOP]      = "rHandTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RHANDBOTTOM]   = "rHandBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RHANDWIDTH]    = "rHandWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_RHANDHEIGHT]   = "rHandHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LHANDLEFT]     = "lHandLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LHANDRIGHT]    = "lHandRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LHANDTOP]      = "lHandTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LHANDBOTTOM]   = "lHandBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LHANDWIDTH]    = "lHandWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_LHANDHEIGHT]   = "lHandHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_BELTLEFT]      = "beltLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_BELTRIGHT]     = "beltRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_BELTTOP]       = "beltTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_BELTBOTTOM]    = "beltBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_BELTWIDTH]     = "beltWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_BELTHEIGHT]    = "beltHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_FEETLEFT]      = "feetLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_FEETRIGHT]     = "feetRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_FEETTOP]       = "feetTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_FEETBOTTOM]    = "feetBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_FEETWIDTH]     = "feetWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_FEETHEIGHT]    = "feetHeight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GLOVESLEFT]    = "glovesLeft";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GLOVESRIGHT]   = "glovesRight";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GLOVESTOP]     = "glovesTop";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GLOVESBOTTOM]  = "glovesBottom";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GLOVESWIDTH]   = "glovesWidth";
   glb_datas.txt_col_names.inventory_header[INVENTORY_GLOVESHEIGHT]  = "glovesHeight";

   // ItemRatio.txt
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_FUNCTION]         = "Function";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_VERSION]          = "Version";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_UBER]             = "Uber";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_CLASSSPECIFIC]    = "Class Specific";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_UNIQUE]           = "Unique";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_UNIQUEDIVISOR]    = "UniqueDivisor";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_UNIQUEMIN]        = "UniqueMin";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_RARE]             = "Rare";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_RAREDIVISOR]      = "RareDivisor";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_RAREMIN]          = "RareMin";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_SET]              = "Set";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_SETDIVISOR]       = "SetDivisor";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_SETMIN]           = "SetMin";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_MAGIC]            = "Magic";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_MAGICDIVISOR]     = "MagicDivisor";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_MAGICMIN]         = "MagicMin";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_HIQUALITY]        = "HiQuality";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_HIQUALITYDIVISOR] = "HiQualityDivisor";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_NORMAL]           = "Normal";
   glb_datas.txt_col_names.itemratio_header[ITEMRATIO_NORMALDIVISOR]    = "NormalDivisor";

   // ItemStatCost.txt
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_STAT]           = "Stat";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ID]             = "ID";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SENDOTHER]      = "Send Other";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SIGNED]         = "Signed";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SENDBITS]       = "Send Bits";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SENDPARAMBITS]  = "Send Param Bits";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_UPDATEANIMRATE] = "UpdateAnimRate";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SAVED]          = "Saved";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_CSVSIGNED]      = "CSvSigned";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_CSVBITS]        = "CSvBits";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_CSVPARAM]       = "CSvParam";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_FCALLBACK]      = "fCallback";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_FMIN]           = "fMin";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_MINACCR]        = "MinAccr";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ENCODE]         = "Encode";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ADD]            = "Add";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_MULTIPLY]       = "Multiply";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DIVIDE]         = "Divide";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_VALSHIFT]       = "ValShift";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_109SAVEBITS]    = "1.09-Save Bits";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_109SAVEADD]     = "1.09-Save Add";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SAVEBITS]       = "Save Bits";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SAVEADD]        = "Save Add";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_SAVEPARAMBITS]  = "Save Param Bits";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_KEEPZERO]       = "keepzero";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_OP]             = "op";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_OPPARAM]        = "op param";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_OPBASE]         = "op base";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_OPSTAT1]        = "op stat1";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_OPSTAT2]        = "op stat2";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_OPSTAT3]        = "op stat3";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DIRECT]         = "direct";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_MAXSTAT]        = "maxstat";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ITEMSPECIFIC]   = "itemspecific";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DAMAGERELATED]  = "damagerelated";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ITEMEVENT1]     = "itemevent1";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ITEMEVENTFUNC1] = "itemeventfunc1";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ITEMEVENT2]     = "itemevent2";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_ITEMEVENTFUNC2] = "itemeventfunc2";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DESCPRIORITY]   = "descpriority";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DESCFUNC]       = "descfunc";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DESCVAL]        = "descval";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DESCSTRPOS]     = "descstrpos";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DESCSTRNEG]     = "descstrneg";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DESCSTR2]       = "descstr2";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DGRP]           = "dgrp";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DGRPFUNC]       = "dgrpfunc";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DGRPVAL]        = "dgrpval";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DGRPSTRPOS]     = "dgrpstrpos";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DGRPSTRNEG]     = "dgrpstrneg";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_DGRPSTR2]       = "dgrpstr2";
   glb_datas.txt_col_names.itemstatcost_header[ITEMSTATCOST_STUFF]          = "stuff";

   // ItemTypes.txt
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_ITEMTYPE]      = "ItemType";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_CODE]          = "Code";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_EQUIV1]        = "Equiv1";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_EQUIV2]        = "Equiv2";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_REPAIR]        = "Repair";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_BODY]          = "Body";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_BODYLOC1]      = "BodyLoc1";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_BODYLOC2]      = "BodyLoc2";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_SHOOTS]        = "Shoots";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_QUIVER]        = "Quiver";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_THROWABLE]     = "Throwable";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_RELOAD]        = "Reload";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_REEQUIP]       = "ReEquip";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_AUTOSTACK]     = "AutoStack";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_MAGIC]         = "Magic";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_RARE]          = "Rare";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_NORMAL]        = "Normal";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_CHARM]         = "Charm";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_GEM]           = "Gem";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_BELTABLE]      = "Beltable";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_MAXSOCK1]      = "MaxSock1";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_MAXSOCK25]     = "MaxSock25";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_MAXSOCK40]     = "MaxSock40";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_TREASURECLASS] = "TreasureClass";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_RARITY]        = "Rarity";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_STAFFMODS]     = "StaffMods";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_COSTFORMULA]   = "CostFormula";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_CLASS]         = "Class";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_VARINVGFX]     = "VarInvGfx";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_INVGFX1]       = "InvGfx1";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_INVGFX2]       = "InvGfx2";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_INVGFX3]       = "InvGfx3";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_INVGFX4]       = "InvGfx4";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_INVGFX5]       = "InvGfx5";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_INVGFX6]       = "InvGfx6";
   glb_datas.txt_col_names.itemtypes_header[ITEMTYPES_STOREPAGE]     = "StorePage";

   // Levels.txt
   glb_datas.txt_col_names.levels_header[LEVELS_NAME]         = "Name";
   glb_datas.txt_col_names.levels_header[LEVELS_ID]           = "Id";
   glb_datas.txt_col_names.levels_header[LEVELS_PAL]          = "Pal";
   glb_datas.txt_col_names.levels_header[LEVELS_ACT]          = "Act";
   glb_datas.txt_col_names.levels_header[LEVELS_QUESTFLAG]    = "QuestFlag";
   glb_datas.txt_col_names.levels_header[LEVELS_QUESTFLAGEX]  = "QuestFlagEx";
   glb_datas.txt_col_names.levels_header[LEVELS_LAYER]        = "Layer";
   glb_datas.txt_col_names.levels_header[LEVELS_SIZEX]        = "SizeX";
   glb_datas.txt_col_names.levels_header[LEVELS_SIZEY]        = "SizeY";
   glb_datas.txt_col_names.levels_header[LEVELS_SIZEXN]       = "SizeX(N)";
   glb_datas.txt_col_names.levels_header[LEVELS_SIZEYN]       = "SizeY(N)";
   glb_datas.txt_col_names.levels_header[LEVELS_SIZEXH]       = "SizeX(H)";
   glb_datas.txt_col_names.levels_header[LEVELS_SIZEYH]       = "SizeY(H)";
   glb_datas.txt_col_names.levels_header[LEVELS_OFFSETX]      = "OffsetX";
   glb_datas.txt_col_names.levels_header[LEVELS_OFFSETY]      = "OffsetY";
   glb_datas.txt_col_names.levels_header[LEVELS_DEPEND]       = "Depend";
   glb_datas.txt_col_names.levels_header[LEVELS_TELEPORT]     = "Teleport";
   glb_datas.txt_col_names.levels_header[LEVELS_RAIN]         = "Rain";
   glb_datas.txt_col_names.levels_header[LEVELS_MUD]          = "Mud";
   glb_datas.txt_col_names.levels_header[LEVELS_NOPER]        = "NoPer";
   glb_datas.txt_col_names.levels_header[LEVELS_LOSDRAW]      = "LOSDraw";
   glb_datas.txt_col_names.levels_header[LEVELS_FLOORFILTER]  = "FloorFilter";
   glb_datas.txt_col_names.levels_header[LEVELS_BLANKSCREEN]  = "BlankScreen";
   glb_datas.txt_col_names.levels_header[LEVELS_DRAWEDGES]    = "DrawEdges";
   glb_datas.txt_col_names.levels_header[LEVELS_ISINSIDE]     = "IsInside";
   glb_datas.txt_col_names.levels_header[LEVELS_DRLGTYPE]     = "DrlgType";
   glb_datas.txt_col_names.levels_header[LEVELS_LEVELTYPE]    = "LevelType";
   glb_datas.txt_col_names.levels_header[LEVELS_SUBTYPE]      = "SubType";
   glb_datas.txt_col_names.levels_header[LEVELS_SUBTHEME]     = "SubTheme";
   glb_datas.txt_col_names.levels_header[LEVELS_SUBWAYPOINT]  = "SubWaypoint";
   glb_datas.txt_col_names.levels_header[LEVELS_SUBSHRINE]    = "SubShrine";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS0]         = "Vis0";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS1]         = "Vis1";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS2]         = "Vis2";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS3]         = "Vis3";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS4]         = "Vis4";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS5]         = "Vis5";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS6]         = "Vis6";
   glb_datas.txt_col_names.levels_header[LEVELS_VIS7]         = "Vis7";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP0]        = "Warp0";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP1]        = "Warp1";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP2]        = "Warp2";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP3]        = "Warp3";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP4]        = "Warp4";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP5]        = "Warp5";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP6]        = "Warp6";
   glb_datas.txt_col_names.levels_header[LEVELS_WARP7]        = "Warp7";
   glb_datas.txt_col_names.levels_header[LEVELS_INTENSITY]    = "Intensity";
   glb_datas.txt_col_names.levels_header[LEVELS_RED]          = "Red";
   glb_datas.txt_col_names.levels_header[LEVELS_GREEN]        = "Green";
   glb_datas.txt_col_names.levels_header[LEVELS_BLUE]         = "Blue";
   glb_datas.txt_col_names.levels_header[LEVELS_PORTAL]       = "Portal";
   glb_datas.txt_col_names.levels_header[LEVELS_POSITION]     = "Position";
   glb_datas.txt_col_names.levels_header[LEVELS_SAVEMONSTERS] = "SaveMonsters";
   glb_datas.txt_col_names.levels_header[LEVELS_QUEST]        = "Quest";
   glb_datas.txt_col_names.levels_header[LEVELS_WARPDIST]     = "WarpDist";
   glb_datas.txt_col_names.levels_header[LEVELS_MONLVL1]      = "MonLvl1";
   glb_datas.txt_col_names.levels_header[LEVELS_MONLVL2]      = "MonLvl2";
   glb_datas.txt_col_names.levels_header[LEVELS_MONLVL3]      = "MonLvl3";
   glb_datas.txt_col_names.levels_header[LEVELS_MONLVL1EX]    = "MonLvl1Ex";
   glb_datas.txt_col_names.levels_header[LEVELS_MONLVL2EX]    = "MonLvl2Ex";
   glb_datas.txt_col_names.levels_header[LEVELS_MONLVL3EX]    = "MonLvl3Ex";
   glb_datas.txt_col_names.levels_header[LEVELS_MONDEN]       = "MonDen";
   glb_datas.txt_col_names.levels_header[LEVELS_MONDENN]      = "MonDen(N)";
   glb_datas.txt_col_names.levels_header[LEVELS_MONDENH]      = "MonDen(H)";
   glb_datas.txt_col_names.levels_header[LEVELS_MONUMIN]      = "MonUMin";
   glb_datas.txt_col_names.levels_header[LEVELS_MONUMAX]      = "MonUMax";
   glb_datas.txt_col_names.levels_header[LEVELS_MONUMINN]     = "MonUMin(N)";
   glb_datas.txt_col_names.levels_header[LEVELS_MONUMAXN]     = "MonUMax(N)";
   glb_datas.txt_col_names.levels_header[LEVELS_MONUMINH]     = "MonUMin(H)";
   glb_datas.txt_col_names.levels_header[LEVELS_MONUMAXH]     = "MonUMax(H)";
   glb_datas.txt_col_names.levels_header[LEVELS_MONWNDR]      = "MonWndr";
   glb_datas.txt_col_names.levels_header[LEVELS_MONSPCWALK]   = "MonSpcWalk";
   glb_datas.txt_col_names.levels_header[LEVELS_NUMMON]       = "NumMon";
   glb_datas.txt_col_names.levels_header[LEVELS_MON1]         = "mon1";
   glb_datas.txt_col_names.levels_header[LEVELS_MON2]         = "mon2";
   glb_datas.txt_col_names.levels_header[LEVELS_MON3]         = "mon3";
   glb_datas.txt_col_names.levels_header[LEVELS_MON4]         = "mon4";
   glb_datas.txt_col_names.levels_header[LEVELS_MON5]         = "mon5";
   glb_datas.txt_col_names.levels_header[LEVELS_MON6]         = "mon6";
   glb_datas.txt_col_names.levels_header[LEVELS_MON7]         = "mon7";
   glb_datas.txt_col_names.levels_header[LEVELS_MON8]         = "mon8";
   glb_datas.txt_col_names.levels_header[LEVELS_MON9]         = "mon9";
   glb_datas.txt_col_names.levels_header[LEVELS_MON10]        = "mon10";
   glb_datas.txt_col_names.levels_header[LEVELS_MON11]        = "mon11";
   glb_datas.txt_col_names.levels_header[LEVELS_MON12]        = "mon12";
   glb_datas.txt_col_names.levels_header[LEVELS_MON13]        = "mon13";
   glb_datas.txt_col_names.levels_header[LEVELS_MON14]        = "mon14";
   glb_datas.txt_col_names.levels_header[LEVELS_MON15]        = "mon15";
   glb_datas.txt_col_names.levels_header[LEVELS_MON16]        = "mon16";
   glb_datas.txt_col_names.levels_header[LEVELS_MON17]        = "mon17";
   glb_datas.txt_col_names.levels_header[LEVELS_MON18]        = "mon18";
   glb_datas.txt_col_names.levels_header[LEVELS_MON19]        = "mon19";
   glb_datas.txt_col_names.levels_header[LEVELS_MON20]        = "mon20";
   glb_datas.txt_col_names.levels_header[LEVELS_MON21]        = "mon21";
   glb_datas.txt_col_names.levels_header[LEVELS_MON22]        = "mon22";
   glb_datas.txt_col_names.levels_header[LEVELS_MON23]        = "mon23";
   glb_datas.txt_col_names.levels_header[LEVELS_MON24]        = "mon24";
   glb_datas.txt_col_names.levels_header[LEVELS_MON25]        = "mon25";
   glb_datas.txt_col_names.levels_header[LEVELS_RANGEDSPAWN]  = "rangedspawn";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON1]        = "nmon1";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON2]        = "nmon2";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON3]        = "nmon3";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON4]        = "nmon4";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON5]        = "nmon5";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON6]        = "nmon6";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON7]        = "nmon7";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON8]        = "nmon8";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON9]        = "nmon9";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON10]       = "nmon10";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON11]       = "nmon11";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON12]       = "nmon12";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON13]       = "nmon13";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON14]       = "nmon14";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON15]       = "nmon15";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON16]       = "nmon16";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON17]       = "nmon17";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON18]       = "nmon18";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON19]       = "nmon19";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON20]       = "nmon20";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON21]       = "nmon21";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON22]       = "nmon22";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON23]       = "nmon23";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON24]       = "nmon24";
   glb_datas.txt_col_names.levels_header[LEVELS_NMON25]       = "nmon25";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON1]        = "umon1";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON2]        = "umon2";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON3]        = "umon3";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON4]        = "umon4";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON5]        = "umon5";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON6]        = "umon6";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON7]        = "umon7";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON8]        = "umon8";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON9]        = "umon9";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON10]       = "umon10";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON11]       = "umon11";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON12]       = "umon12";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON13]       = "umon13";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON14]       = "umon14";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON15]       = "umon15";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON16]       = "umon16";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON17]       = "umon17";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON18]       = "umon18";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON19]       = "umon19";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON20]       = "umon20";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON21]       = "umon21";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON22]       = "umon22";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON23]       = "umon23";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON24]       = "umon24";
   glb_datas.txt_col_names.levels_header[LEVELS_UMON25]       = "umon25";
   glb_datas.txt_col_names.levels_header[LEVELS_CMON1]        = "cmon1";
   glb_datas.txt_col_names.levels_header[LEVELS_CMON2]        = "cmon2";
   glb_datas.txt_col_names.levels_header[LEVELS_CMON3]        = "cmon3";
   glb_datas.txt_col_names.levels_header[LEVELS_CMON4]        = "cmon4";
   glb_datas.txt_col_names.levels_header[LEVELS_CPCT1]        = "cpct1";
   glb_datas.txt_col_names.levels_header[LEVELS_CPCT2]        = "cpct2";
   glb_datas.txt_col_names.levels_header[LEVELS_CPCT3]        = "cpct3";
   glb_datas.txt_col_names.levels_header[LEVELS_CPCT4]        = "cpct4";
   glb_datas.txt_col_names.levels_header[LEVELS_CAMT1]        = "camt1";
   glb_datas.txt_col_names.levels_header[LEVELS_CAMT2]        = "camt2";
   glb_datas.txt_col_names.levels_header[LEVELS_CAMT3]        = "camt3";
   glb_datas.txt_col_names.levels_header[LEVELS_CAMT4]        = "camt4";
   glb_datas.txt_col_names.levels_header[LEVELS_THEMES]       = "Themes";
   glb_datas.txt_col_names.levels_header[LEVELS_SOUNDENV]     = "SoundEnv";
   glb_datas.txt_col_names.levels_header[LEVELS_WAYPOINT]     = "Waypoint";
   glb_datas.txt_col_names.levels_header[LEVELS_LEVELNAME]    = "LevelName";
   glb_datas.txt_col_names.levels_header[LEVELS_LEVELWARP]    = "LevelWarp";
   glb_datas.txt_col_names.levels_header[LEVELS_ENTRYFILE]    = "EntryFile";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP0]      = "ObjGrp0";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP1]      = "ObjGrp1";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP2]      = "ObjGrp2";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP3]      = "ObjGrp3";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP4]      = "ObjGrp4";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP5]      = "ObjGrp5";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP6]      = "ObjGrp6";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJGRP7]      = "ObjGrp7";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB0]      = "ObjPrb0";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB1]      = "ObjPrb1";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB2]      = "ObjPrb2";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB3]      = "ObjPrb3";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB4]      = "ObjPrb4";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB5]      = "ObjPrb5";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB6]      = "ObjPrb6";
   glb_datas.txt_col_names.levels_header[LEVELS_OBJPRB7]      = "ObjPrb7";
   glb_datas.txt_col_names.levels_header[LEVELS_BETA]         = "Beta";

   // LowQualityItems.txt
   glb_datas.txt_col_names.lowqualityitems_header[LOWQUALITYITEMS_NAME] = "Name";

   // LvlMaze.txt
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_NAME]   = "Name";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_LEVEL]  = "Level";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_ROOMS]  = "Rooms";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_ROOMSN] = "Rooms(N)";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_ROOMSH] = "Rooms(H)";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_SIZEX]  = "SizeX";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_SIZEY]  = "SizeY";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_MERGE]  = "Merge";
   glb_datas.txt_col_names.lvlmaze_header[LVLMAZE_BETA]   = "Beta";

   // LvlPrest.txt
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_NAME]       = "Name";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_DEF]        = "Def";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_LEVELID]    = "LevelId";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_POPULATE]   = "Populate";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_LOGICALS]   = "Logicals";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_OUTDOORS]   = "Outdoors";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_ANIMATE]    = "Animate";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_KILLEDGE]   = "KillEdge";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILLBLANKS] = "FillBlanks";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_SIZEX]      = "SizeX";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_SIZEY]      = "SizeY";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_AUTOMAP]    = "AutoMap";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_SCAN]       = "Scan";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_POPS]       = "Pops";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_POPPAD]     = "PopPad";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILES]      = "Files";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILE1]      = "File1";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILE2]      = "File2";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILE3]      = "File3";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILE4]      = "File4";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILE5]      = "File5";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_FILE6]      = "File6";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_DT1MASK]    = "Dt1Mask";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_BETA]       = "Beta";
   glb_datas.txt_col_names.lvlprest_header[LVLPREST_EXPANSION]  = "Expansion";

   // LvlSub.txt
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_NAME]      = "Name";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_TYPE]      = "Type";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_FILE]      = "File";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_EXPANSION] = "Expansion";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_CHECKALL]  = "CheckAll";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_BORDTYPE]  = "BordType";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_GRIDSIZE]  = "GridSize";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_DT1MASK]   = "Dt1Mask";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_PROB0]     = "Prob0";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_TRIALS0]   = "Trials0";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_MAX0]      = "Max0";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_PROB1]     = "Prob1";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_TRIALS1]   = "Trials1";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_MAX1]      = "Max1";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_PROB2]     = "Prob2";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_TRIALS2]   = "Trials2";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_MAX2]      = "Max2";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_PROB3]     = "Prob3";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_TRIALS3]   = "Trials3";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_MAX3]      = "Max3";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_PROB4]     = "Prob4";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_TRIALS4]   = "Trials4";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_MAX4]      = "Max4";
   glb_datas.txt_col_names.lvlsub_header[LVLSUB_BETA]      = "Beta";
   
   // LvlTypes.txt
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_NAME]      = "Name";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_ID]        = "Id";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE1]     = "File 1";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE2]     = "File 2";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE3]     = "File 3";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE4]     = "File 4";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE5]     = "File 5";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE6]     = "File 6";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE7]     = "File 7";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE8]     = "File 8";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE9]     = "File 9";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE10]    = "File 10";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE11]    = "File 11";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE12]    = "File 12";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE13]    = "File 13";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE14]    = "File 14";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE15]    = "File 15";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE16]    = "File 16";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE17]    = "File 17";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE18]    = "File 18";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE19]    = "File 19";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE20]    = "File 20";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE21]    = "File 21";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE22]    = "File 22";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE23]    = "File 23";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE24]    = "File 24";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE25]    = "File 25";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE26]    = "File 26";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE27]    = "File 27";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE28]    = "File 28";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE29]    = "File 29";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE30]    = "File 30";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE31]    = "File 31";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_FILE32]    = "File 32";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_BETA]      = "Beta";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_ACT]       = "Act";
   glb_datas.txt_col_names.lvltypes_header[LVLTYPES_EXPANSION] = "Expansion";

   // LvlWarp.txt
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_NAME]       = "Name";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_ID]         = "Id";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_SELECTX]    = "SelectX";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_SELECTY]    = "SelectY";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_SELECTDX]   = "SelectDX";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_SELECTDY]   = "SelectDY";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_EXITWALKX]  = "ExitWalkX";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_EXITWALKY]  = "ExitWalkY";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_OFFSETX]    = "OffsetX";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_OFFSETY]    = "OffsetY";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_LITVERSION] = "LitVersion";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_TILES]      = "Tiles";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_DIRECTION]  = "Direction";
   glb_datas.txt_col_names.lvlwarp_header[LVLWARP_BETA]       = "Beta";

   // MagicPrefix.txt + MagicSuffix
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_NAME]           = "Name";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_VERSION]        = "version";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_SPAWNABLE]      = "spawnable";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_RARE]           = "rare";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_LEVEL]          = "level";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MAXLEVEL]       = "maxlevel";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_LEVELREQ]       = "levelreq";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_CLASSSPECIFIC]  = "classspecific";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_CLASS]          = "class";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_CLASSLEVELREQ]  = "classlevelreq";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_FREQUENCY]      = "frequency";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_GROUP]          = "group";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD1CODE]       = "mod1code";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD1PARAM]      = "mod1param";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD1MIN]        = "mod1min";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD1MAX]        = "mod1max";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD2CODE]       = "mod2code";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD2PARAM]      = "mod2param";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD2MIN]        = "mod2min";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD2MAX]        = "mod2max";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD3CODE]       = "mod3code";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD3PARAM]      = "mod3param";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD3MIN]        = "mod3min";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MOD3MAX]        = "mod3max";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_TRANSFORM]      = "transform";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_TRANSFORMCOLOR] = "transformcolor";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE1]         = "itype1";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE2]         = "itype2";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE3]         = "itype3";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE4]         = "itype4";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE5]         = "itype5";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE6]         = "itype6";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ITYPE7]         = "itype7";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ETYPE1]         = "etype1";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ETYPE2]         = "etype2";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ETYPE3]         = "etype3";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ETYPE4]         = "etype4";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ETYPE5]         = "etype5";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_DIVIDE]         = "divide";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_MULTIPLY]       = "multiply";
   glb_datas.txt_col_names.magicaffix_header[MAGICAFFIX_ADD]            = "add";

   // MissCalc.txt
   glb_datas.txt_col_names.misscalc_header[MISSCALC_CODE] = "code";

   // Missiles.txt
   glb_datas.txt_col_names.missiles_header[MISSILES_MISSILE]           = "Missile";
   glb_datas.txt_col_names.missiles_header[MISSILES_ID]                = "Id";
   glb_datas.txt_col_names.missiles_header[MISSILES_PCLTDOFUNC]        = "pCltDoFunc";
   glb_datas.txt_col_names.missiles_header[MISSILES_PCLTHITFUNC]       = "pCltHitFunc";
   glb_datas.txt_col_names.missiles_header[MISSILES_PSRVDOFUNC]        = "pSrvDoFunc";
   glb_datas.txt_col_names.missiles_header[MISSILES_PSRVHITFUNC]       = "pSrvHitFunc";
   glb_datas.txt_col_names.missiles_header[MISSILES_PSRVDMGFUNC]       = "pSrvDmgFunc";
   glb_datas.txt_col_names.missiles_header[MISSILES_SRVCALC1]          = "SrvCalc1";
   glb_datas.txt_col_names.missiles_header[MISSILES_PARAM1]            = "Param1";
   glb_datas.txt_col_names.missiles_header[MISSILES_PARAM2]            = "Param2";
   glb_datas.txt_col_names.missiles_header[MISSILES_PARAM3]            = "Param3";
   glb_datas.txt_col_names.missiles_header[MISSILES_PARAM4]            = "Param4";
   glb_datas.txt_col_names.missiles_header[MISSILES_PARAM5]            = "Param5";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTCALC1]          = "CltCalc1";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTPARAM1]         = "CltParam1";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTPARAM2]         = "CltParam2";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTPARAM3]         = "CltParam3";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTPARAM4]         = "CltParam4";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTPARAM5]         = "CltParam5";
   glb_datas.txt_col_names.missiles_header[MISSILES_SHITCALC1]         = "SHitCalc1";
   glb_datas.txt_col_names.missiles_header[MISSILES_SHITPAR1]          = "sHitPar1";
   glb_datas.txt_col_names.missiles_header[MISSILES_SHITPAR2]          = "sHitPar2";
   glb_datas.txt_col_names.missiles_header[MISSILES_SHITPAR3]          = "sHitPar3";
   glb_datas.txt_col_names.missiles_header[MISSILES_CHITCALC1]         = "CHitCalc1";
   glb_datas.txt_col_names.missiles_header[MISSILES_CHITPAR1]          = "cHitPar1";
   glb_datas.txt_col_names.missiles_header[MISSILES_CHITPAR2]          = "cHitPar2";
   glb_datas.txt_col_names.missiles_header[MISSILES_CHITPAR3]          = "cHitPar3";
   glb_datas.txt_col_names.missiles_header[MISSILES_DMGCALC1]          = "DmgCalc1";
   glb_datas.txt_col_names.missiles_header[MISSILES_DPARAM1]           = "dParam1";
   glb_datas.txt_col_names.missiles_header[MISSILES_DPARAM2]           = "dParam2";
   glb_datas.txt_col_names.missiles_header[MISSILES_VEL]               = "Vel";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXVEL]            = "MaxVel";
   glb_datas.txt_col_names.missiles_header[MISSILES_VELLEV]            = "VelLev";
   glb_datas.txt_col_names.missiles_header[MISSILES_ACCEL]             = "Accel";
   glb_datas.txt_col_names.missiles_header[MISSILES_RANGE]             = "Range";
   glb_datas.txt_col_names.missiles_header[MISSILES_LEVRANGE]          = "LevRange";
   glb_datas.txt_col_names.missiles_header[MISSILES_LIGHT]             = "Light";
   glb_datas.txt_col_names.missiles_header[MISSILES_FLICKER]           = "Flicker";
   glb_datas.txt_col_names.missiles_header[MISSILES_RED]               = "Red";
   glb_datas.txt_col_names.missiles_header[MISSILES_GREEN]             = "Green";
   glb_datas.txt_col_names.missiles_header[MISSILES_BLUE]              = "Blue";
   glb_datas.txt_col_names.missiles_header[MISSILES_INITSTEPS]         = "InitSteps";
   glb_datas.txt_col_names.missiles_header[MISSILES_ACTIVATE]          = "Activate";
   glb_datas.txt_col_names.missiles_header[MISSILES_LOOPANIM]          = "LoopAnim";
   glb_datas.txt_col_names.missiles_header[MISSILES_CELFILE]           = "CelFile";
   glb_datas.txt_col_names.missiles_header[MISSILES_ANIMRATE]          = "animrate";
   glb_datas.txt_col_names.missiles_header[MISSILES_ANIMLEN]           = "AnimLen";
   glb_datas.txt_col_names.missiles_header[MISSILES_ANIMSPEED]         = "AnimSpeed";
   glb_datas.txt_col_names.missiles_header[MISSILES_RANDSTART]         = "RandStart";
   glb_datas.txt_col_names.missiles_header[MISSILES_SUBLOOP]           = "SubLoop";
   glb_datas.txt_col_names.missiles_header[MISSILES_SUBSTART]          = "SubStart";
   glb_datas.txt_col_names.missiles_header[MISSILES_SUBSTOP]           = "SubStop";
   glb_datas.txt_col_names.missiles_header[MISSILES_COLLIDETYPE]       = "CollideType";
   glb_datas.txt_col_names.missiles_header[MISSILES_COLLIDEKILL]       = "CollideKill";
   glb_datas.txt_col_names.missiles_header[MISSILES_COLLIDEFRIEND]     = "CollideFriend";
   glb_datas.txt_col_names.missiles_header[MISSILES_LASTCOLLIDE]       = "LastCollide";
   glb_datas.txt_col_names.missiles_header[MISSILES_COLLISION]         = "Collision";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLIENTCOL]         = "ClientCol";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLIENTSEND]        = "ClientSend";
   glb_datas.txt_col_names.missiles_header[MISSILES_NEXTHIT]           = "NextHit";
   glb_datas.txt_col_names.missiles_header[MISSILES_NEXTDELAY]         = "NextDelay";
   glb_datas.txt_col_names.missiles_header[MISSILES_XOFFSET]           = "xoffset";
   glb_datas.txt_col_names.missiles_header[MISSILES_YOFFSET]           = "yoffset";
   glb_datas.txt_col_names.missiles_header[MISSILES_ZOFFSET]           = "zoffset";
   glb_datas.txt_col_names.missiles_header[MISSILES_SIZE]              = "Size";
   glb_datas.txt_col_names.missiles_header[MISSILES_SRCTOWN]           = "SrcTown";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTSRCTOWN]        = "CltSrcTown";
   glb_datas.txt_col_names.missiles_header[MISSILES_CANDESTROY]        = "CanDestroy";
   glb_datas.txt_col_names.missiles_header[MISSILES_TOHIT]             = "ToHit";
   glb_datas.txt_col_names.missiles_header[MISSILES_ALWAYSEXPLODE]     = "AlwaysExplode";
   glb_datas.txt_col_names.missiles_header[MISSILES_EXPLOSION]         = "Explosion";
   glb_datas.txt_col_names.missiles_header[MISSILES_TOWN]              = "Town";
   glb_datas.txt_col_names.missiles_header[MISSILES_NOUNIQUEMOD]       = "NoUniqueMod";
   glb_datas.txt_col_names.missiles_header[MISSILES_NOMULTISHOT]       = "NoMultiShot";
   glb_datas.txt_col_names.missiles_header[MISSILES_HOLY]              = "Holy";
   glb_datas.txt_col_names.missiles_header[MISSILES_CANSLOW]           = "CanSlow";
   glb_datas.txt_col_names.missiles_header[MISSILES_RETURNFIRE]        = "ReturnFire";
   glb_datas.txt_col_names.missiles_header[MISSILES_GETHIT]            = "GetHit";
   glb_datas.txt_col_names.missiles_header[MISSILES_SOFTHIT]           = "SoftHit";
   glb_datas.txt_col_names.missiles_header[MISSILES_KNOCKBACK]         = "KnockBack";
   glb_datas.txt_col_names.missiles_header[MISSILES_TRANS]             = "Trans";
   glb_datas.txt_col_names.missiles_header[MISSILES_QTY]               = "Qty";
   glb_datas.txt_col_names.missiles_header[MISSILES_PIERCE]            = "Pierce";
   glb_datas.txt_col_names.missiles_header[MISSILES_SPECIALSETUP]      = "SpecialSetup";
   glb_datas.txt_col_names.missiles_header[MISSILES_MISSILESKILL]      = "MissileSkill";
   glb_datas.txt_col_names.missiles_header[MISSILES_SKILL]             = "Skill";
   glb_datas.txt_col_names.missiles_header[MISSILES_RESULTFLAGS]       = "ResultFlags";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITFLAGS]          = "HitFlags";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITSHIFT]          = "HitShift";
   glb_datas.txt_col_names.missiles_header[MISSILES_APPLYMASTERY]      = "ApplyMastery";
   glb_datas.txt_col_names.missiles_header[MISSILES_SRCDAMAGE]         = "SrcDamage";
   glb_datas.txt_col_names.missiles_header[MISSILES_HALF2HSRC]         = "Half2HSrc";
   glb_datas.txt_col_names.missiles_header[MISSILES_SRCMISSDMG]        = "SrcMissDmg";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINDAMAGE]         = "MinDamage";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINLEVDAM1]        = "MinLevDam1";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINLEVDAM2]        = "MinLevDam2";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINLEVDAM3]        = "MinLevDam3";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINLEVDAM4]        = "MinLevDam4";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINLEVDAM5]        = "MinLevDam5";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXDAMAGE]         = "MaxDamage";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXLEVDAM1]        = "MaxLevDam1";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXLEVDAM2]        = "MaxLevDam2";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXLEVDAM3]        = "MaxLevDam3";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXLEVDAM4]        = "MaxLevDam4";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXLEVDAM5]        = "MaxLevDam5";
   glb_datas.txt_col_names.missiles_header[MISSILES_DMGSYMPERCALC]     = "DmgSymPerCalc";
   glb_datas.txt_col_names.missiles_header[MISSILES_ETYPE]             = "EType";
   glb_datas.txt_col_names.missiles_header[MISSILES_EMIN]              = "EMin";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINELEV1]          = "MinELev1";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINELEV2]          = "MinELev2";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINELEV3]          = "MinELev3";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINELEV4]          = "MinELev4";
   glb_datas.txt_col_names.missiles_header[MISSILES_MINELEV5]          = "MinELev5";
   glb_datas.txt_col_names.missiles_header[MISSILES_EMAX]              = "Emax";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXELEV1]          = "MaxELev1";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXELEV2]          = "MaxELev2";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXELEV3]          = "MaxELev3";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXELEV4]          = "MaxELev4";
   glb_datas.txt_col_names.missiles_header[MISSILES_MAXELEV5]          = "MaxELev5";
   glb_datas.txt_col_names.missiles_header[MISSILES_EDMGSYMPERCALC]    = "EDmgSymPerCalc";
   glb_datas.txt_col_names.missiles_header[MISSILES_ELEN]              = "ELen";
   glb_datas.txt_col_names.missiles_header[MISSILES_ELEVLEN1]          = "ELevLen1";
   glb_datas.txt_col_names.missiles_header[MISSILES_ELEVLEN2]          = "ELevLen2";
   glb_datas.txt_col_names.missiles_header[MISSILES_ELEVLEN3]          = "ELevLen3";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITCLASS]          = "HitClass";
   glb_datas.txt_col_names.missiles_header[MISSILES_NUMDIRECTIONS]     = "NumDirections";
   glb_datas.txt_col_names.missiles_header[MISSILES_LOCALBLOOD]        = "LocalBlood";
   glb_datas.txt_col_names.missiles_header[MISSILES_DAMAGERATE]        = "DamageRate";
   glb_datas.txt_col_names.missiles_header[MISSILES_TRAVELSOUND]       = "TravelSound";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITSOUND]          = "HitSound";
   glb_datas.txt_col_names.missiles_header[MISSILES_PROGSOUND]         = "ProgSound";
   glb_datas.txt_col_names.missiles_header[MISSILES_PROGOVERLAY]       = "ProgOverlay";
   glb_datas.txt_col_names.missiles_header[MISSILES_EXPLOSIONMISSILE]  = "ExplosionMissile";
   glb_datas.txt_col_names.missiles_header[MISSILES_SUBMISSILE1]       = "SubMissile1";
   glb_datas.txt_col_names.missiles_header[MISSILES_SUBMISSILE2]       = "SubMissile2";
   glb_datas.txt_col_names.missiles_header[MISSILES_SUBMISSILE3]       = "SubMissile3";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITSUBMISSILE1]    = "HitSubMissile1";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITSUBMISSILE2]    = "HitSubMissile2";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITSUBMISSILE3]    = "HitSubMissile3";
   glb_datas.txt_col_names.missiles_header[MISSILES_HITSUBMISSILE4]    = "HitSubMissile4";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTSUBMISSILE1]    = "CltSubMissile1";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTSUBMISSILE2]    = "CltSubMissile2";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTSUBMISSILE3]    = "CltSubMissile3";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTHITSUBMISSILE1] = "CltHitSubMissile1";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTHITSUBMISSILE2] = "CltHitSubMissile2";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTHITSUBMISSILE3] = "CltHitSubMissile3";
   glb_datas.txt_col_names.missiles_header[MISSILES_CLTHITSUBMISSILE4] = "CltHitSubMissile4";

   // MonAI.txt
   glb_datas.txt_col_names.monai_header[MONAI_AI] = "AI";

   // MonEquip.txt
   glb_datas.txt_col_names.monequip_header[MONEQUIP_MONSTER] = "monster";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_ONINIT]  = "oninit";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_LEVEL]   = "level";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_ITEM1]   = "item1";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_LOC1]    = "loc1";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_MOD1]    = "mod1";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_ITEM2]   = "item2";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_LOC2]    = "loc2";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_MOD2]    = "mod2";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_ITEM3]   = "item3";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_LOC3]    = "loc3";
   glb_datas.txt_col_names.monequip_header[MONEQUIP_MOD3]    = "mod3";

   // MonLvl.txt
   glb_datas.txt_col_names.monlvl_header[MONLVL_LEVEL] = "Level";
   glb_datas.txt_col_names.monlvl_header[MONLVL_AC]    = "AC";
   glb_datas.txt_col_names.monlvl_header[MONLVL_ACN]   = "AC(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_ACH]   = "AC(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LAC]   = "L-AC";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LACN]  = "L-AC(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LACH]  = "L-AC(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_TH]    = "TH";
   glb_datas.txt_col_names.monlvl_header[MONLVL_THN]   = "TH(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_THH]   = "TH(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LTH]   = "L-TH";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LTHN]  = "L-TH(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LTHH]  = "L-TH(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_HP]    = "HP";
   glb_datas.txt_col_names.monlvl_header[MONLVL_HPN]   = "HP(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_HPH]   = "HP(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LHP]   = "L-HP";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LHPN]  = "L-HP(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LHPH]  = "L-HP(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_DM]    = "DM";
   glb_datas.txt_col_names.monlvl_header[MONLVL_DMN]   = "DM(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_DMH]   = "DM(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LDM]   = "L-DM";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LDMN]  = "L-DM(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LDMH]  = "L-DM(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_XP]    = "XP";
   glb_datas.txt_col_names.monlvl_header[MONLVL_XPN]   = "XP(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_XPH]   = "XP(H)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LXP]   = "L-XP";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LXPN]  = "L-XP(N)";
   glb_datas.txt_col_names.monlvl_header[MONLVL_LXPH]  = "L-XP(H)";

   // MonMode.txt
   glb_datas.txt_col_names.monmode_header[MONMODE_NAME]  = "name";
   glb_datas.txt_col_names.monmode_header[MONMODE_TOKEN] = "token";
   glb_datas.txt_col_names.monmode_header[MONMODE_CODE]  = "code";

   // MonName.txt
   glb_datas.txt_col_names.monname_header[MONNAME_MONNAME] = "MonName";

   // MonPlace.txt
   glb_datas.txt_col_names.monplace_header[MONPLACE_CODE] = "code";

   // MonPreset.txt
   glb_datas.txt_col_names.monpreset_header[MONPRESET_ACT]   = "Act";
   glb_datas.txt_col_names.monpreset_header[MONPRESET_PLACE] = "Place";

   // MonSeq.txt
   glb_datas.txt_col_names.monseq_header[MONSEQ_SEQUENCE] = "sequence";
   glb_datas.txt_col_names.monseq_header[MONSEQ_MODE]     = "mode";
   glb_datas.txt_col_names.monseq_header[MONSEQ_FRAME]    = "frame";
   glb_datas.txt_col_names.monseq_header[MONSEQ_DIR]      = "dir";
   glb_datas.txt_col_names.monseq_header[MONSEQ_EVENT]    = "event";

   // MonProp.txt
   glb_datas.txt_col_names.monprop_header[MONPROP_ID]       = "Id";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP1]    = "prop1";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE1]  = "chance1";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR1]     = "par1";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN1]     = "min1";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX1]     = "max1";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP2]    = "prop2";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE2]  = "chance2";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR2]     = "par2";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN2]     = "min2";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX2]     = "max2";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP3]    = "prop3";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE3]  = "chance3";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR3]     = "par3";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN3]     = "min3";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX3]     = "max3";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP4]    = "prop4";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE4]  = "chance4";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR4]     = "par4";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN4]     = "min4";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX4]     = "max4";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP5]    = "prop5";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE5]  = "chance5";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR5]     = "par5";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN5]     = "min5";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX5]     = "max5";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP6]    = "prop6";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE6]  = "chance6";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR6]     = "par6";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN6]     = "min6";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX6]     = "max6";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP1N]   = "prop1 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE1N] = "chance1 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR1N]    = "par1 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN1N]    = "min1 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX1N]    = "max1 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP2N]   = "prop2 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE2N] = "chance2 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR2N]    = "par2 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN2N]    = "min2 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX2N]    = "max2 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP3N]   = "prop3 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE3N] = "chance3 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR3N]    = "par3 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN3N]    = "min3 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX3N]    = "max3 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP4N]   = "prop4 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE4N] = "chance4 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR4N]    = "par4 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN4N]    = "min4 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX4N]    = "max4 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP5N]   = "prop5 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE5N] = "chance5 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR5N]    = "par5 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN5N]    = "min5 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX5N]    = "max5 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP6N]   = "prop6 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE6N] = "chance6 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR6N]    = "par6 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN6N]    = "min6 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX6N]    = "max6 (N)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP1H]   = "prop1 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE1H] = "chance1 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR1H]    = "par1 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN1H]    = "min1 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX1H]    = "max1 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP2H]   = "prop2 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE2H] = "chance2 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR2H]    = "par2 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN2H]    = "min2 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX2H]    = "max2 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP3H]   = "prop3 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE3H] = "chance3 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR3H]    = "par3 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN3H]    = "min3 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX3H]    = "max3 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP4H]   = "prop4 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE4H] = "chance4 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR4H]    = "par4 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN4H]    = "min4 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX4H]    = "max4 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP5H]   = "prop5 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE5H] = "chance5 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR5H]    = "par5 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN5H]    = "min5 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX5H]    = "max5 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PROP6H]   = "prop6 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_CHANCE6H] = "chance6 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_PAR6H]    = "par6 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MIN6H]    = "min6 (H)";
   glb_datas.txt_col_names.monprop_header[MONPROP_MAX6H]    = "max6 (H)";

   // MonSounds.txt
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ID]            = "Id";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ATTACK1]       = "Attack1";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_WEAPON1]       = "Weapon1";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ATT1DEL]       = "Att1Del";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_WEA1DEL]       = "Wea1Del";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ATT1PRB]       = "Att1Prb";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_WEA1VOL]       = "Wea1Vol";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ATTACK2]       = "Attack2";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_WEAPON2]       = "Weapon2";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ATT2DEL]       = "Att2Del";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_WEA2DEL]       = "Wea2Del";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_ATT2PRB]       = "Att2Prb";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_WEA2VOL]       = "Wea2Vol";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_HITSOUND]      = "HitSound";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_DEATHSOUND]    = "DeathSound";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_HITDELAY]      = "HitDelay";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_DEADELAY]      = "DeaDelay";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_SKILL1]        = "Skill1";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_SKILL2]        = "Skill2";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_SKILL3]        = "Skill3";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_SKILL4]        = "Skill4";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_FOOTSTEP]      = "Footstep";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_FOOTSTEPLAYER] = "FootstepLayer";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_FSCNT]         = "FsCnt";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_FSOFF]         = "FsOff";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_FSPRB]         = "FsPrb";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_NEUTRAL]       = "Neutral";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_NEUTIME]       = "NeuTime";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_INIT]          = "Init";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_TAUNT]         = "Taunt";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_FLEE]          = "Flee";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTMO1]        = "CvtMo1";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTSK1]        = "CvtSk1";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTTGT1]       = "CvtTgt1";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTMO2]        = "CvtMo2";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTSK2]        = "CvtSk2";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTTGT2]       = "CvtTgt2";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTMO3]        = "CvtMo3";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTSK3]        = "CvtSk3";
   glb_datas.txt_col_names.monsounds_header[MONSOUNDS_CVTTGT3]       = "CvtTgt3";

   // MonStats.txt
   glb_datas.txt_col_names.monstats_header[MONSTATS_ID]              = "Id";
   glb_datas.txt_col_names.monstats_header[MONSTATS_HCIDX]           = "hcIdx";
   glb_datas.txt_col_names.monstats_header[MONSTATS_BASEID]          = "BaseId";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NEXTINCLASS]     = "NextInClass";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TRANSLVL]        = "TransLvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NAMESTR]         = "NameStr";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MONSTATSEX]      = "MonStatsEx";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MONPROP]         = "MonProp";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MONTYPE]         = "MonType";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AI]              = "AI";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DESCSTR]         = "DescStr";
   glb_datas.txt_col_names.monstats_header[MONSTATS_CODE]            = "Code";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ENABLED]         = "enabled";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RANGEDTYPE]      = "rangedtype";
   glb_datas.txt_col_names.monstats_header[MONSTATS_PLACESPAWN]      = "placespawn";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPAWN]           = "spawn";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPAWNX]          = "spawnx";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPAWNY]          = "spawny";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPAWNMODE]       = "spawnmode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MINION1]         = "minion1";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MINION2]         = "minion2";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SETBOSS]         = "SetBoss";
   glb_datas.txt_col_names.monstats_header[MONSTATS_BOSSXFER]        = "BossXfer";
   glb_datas.txt_col_names.monstats_header[MONSTATS_PARTYMIN]        = "PartyMin";
   glb_datas.txt_col_names.monstats_header[MONSTATS_PARTYMAX]        = "PartyMax";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MINGRP]          = "MinGrp";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MAXGRP]          = "MaxGrp";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPARSEPOPULATE]  = "sparsePopulate";
   glb_datas.txt_col_names.monstats_header[MONSTATS_VELOCITY]        = "Velocity";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RUN]             = "Run";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RARITY]          = "Rarity";
   glb_datas.txt_col_names.monstats_header[MONSTATS_LEVEL]           = "Level";
   glb_datas.txt_col_names.monstats_header[MONSTATS_LEVELN]          = "Level(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_LEVELH]          = "Level(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MONSOUND]        = "MonSound";
   glb_datas.txt_col_names.monstats_header[MONSTATS_UMONSOUND]       = "UMonSound";
   glb_datas.txt_col_names.monstats_header[MONSTATS_THREAT]          = "threat";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIDEL]           = "aidel";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIDELN]          = "aidel(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIDELH]          = "aidel(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIDIST]          = "aidist";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIDISTN]         = "aidist(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIDISTH]         = "aidist(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP1]            = "aip1";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP1N]           = "aip1(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP1H]           = "aip1(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP2]            = "aip2";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP2N]           = "aip2(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP2H]           = "aip2(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP3]            = "aip3";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP3N]           = "aip3(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP3H]           = "aip3(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP4]            = "aip4";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP4N]           = "aip4(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP4H]           = "aip4(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP5]            = "aip5";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP5N]           = "aip5(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP5H]           = "aip5(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP6]            = "aip6";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP6N]           = "aip6(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP6H]           = "aip6(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP7]            = "aip7";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP7N]           = "aip7(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP7H]           = "aip7(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP8]            = "aip8";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP8N]           = "aip8(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AIP8H]           = "aip8(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSA1]          = "MissA1";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSA2]          = "MissA2";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSS1]          = "MissS1";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSS2]          = "MissS2";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSS3]          = "MissS3";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSS4]          = "MissS4";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSC]           = "MissC";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MISSSQ]          = "MissSQ";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ALIGN]           = "Align";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ISSPAWN]         = "isSpawn";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ISMELEE]         = "isMelee";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NPC]             = "npc";
   glb_datas.txt_col_names.monstats_header[MONSTATS_INTERACT]        = "interact";
   glb_datas.txt_col_names.monstats_header[MONSTATS_INVENTORY]       = "inventory";
   glb_datas.txt_col_names.monstats_header[MONSTATS_INTOWN]          = "inTown";
   glb_datas.txt_col_names.monstats_header[MONSTATS_LUNDEAD]         = "lUndead";
   glb_datas.txt_col_names.monstats_header[MONSTATS_HUNDEAD]         = "hUndead";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DEMON]           = "demon";
   glb_datas.txt_col_names.monstats_header[MONSTATS_FLYING]          = "flying";
   glb_datas.txt_col_names.monstats_header[MONSTATS_OPENDOORS]       = "opendoors";
   glb_datas.txt_col_names.monstats_header[MONSTATS_BOSS]            = "boss";
   glb_datas.txt_col_names.monstats_header[MONSTATS_PRIMEEVIL]       = "primeevil";
   glb_datas.txt_col_names.monstats_header[MONSTATS_KILLABLE]        = "killable";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SWITCHAI]        = "switchai";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NOAURA]          = "noAura";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NOMULTISHOT]     = "nomultishot";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NEVERCOUNT]      = "neverCount";
   glb_datas.txt_col_names.monstats_header[MONSTATS_PETIGNORE]       = "petIgnore";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DEATHDMG]        = "deathDmg";
   glb_datas.txt_col_names.monstats_header[MONSTATS_GENERICSPAWN]    = "genericSpawn";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ZOO]             = "zoo";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SENDSKILLS]      = "SendSkills";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL1]          = "Skill1";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK1MODE]         = "Sk1mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK1LVL]          = "Sk1lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL2]          = "Skill2";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK2MODE]         = "Sk2mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK2LVL]          = "Sk2lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL3]          = "Skill3";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK3MODE]         = "Sk3mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK3LVL]          = "Sk3lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL4]          = "Skill4";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK4MODE]         = "Sk4mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK4LVL]          = "Sk4lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL5]          = "Skill5";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK5MODE]         = "Sk5mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK5LVL]          = "Sk5lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL6]          = "Skill6";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK6MODE]         = "Sk6mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK6LVL]          = "Sk6lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL7]          = "Skill7";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK7MODE]         = "Sk7mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK7LVL]          = "Sk7lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILL8]          = "Skill8";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK8MODE]         = "Sk8mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SK8LVL]          = "Sk8lvl";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DRAIN]           = "Drain";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DRAINN]          = "Drain(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DRAINH]          = "Drain(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_COLDEFFECT]      = "coldeffect";
   glb_datas.txt_col_names.monstats_header[MONSTATS_COLDEFFECTN]     = "coldeffect(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_COLDEFFECTH]     = "coldeffect(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESDM]           = "ResDm";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESMA]           = "ResMa";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESFI]           = "ResFi";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESLI]           = "ResLi";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESCO]           = "ResCo";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESPO]           = "ResPo";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESDMN]          = "ResDm(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESMAN]          = "ResMa(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESFIN]          = "ResFi(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESLIN]          = "ResLi(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESCON]          = "ResCo(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESPON]          = "ResPo(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESDMH]          = "ResDm(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESMAH]          = "ResMa(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESFIH]          = "ResFi(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESLIH]          = "ResLi(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESCOH]          = "ResCo(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_RESPOH]          = "ResPo(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_DAMAGEREGEN]     = "DamageRegen";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SKILLDAMAGE]     = "SkillDamage";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NORATIO]         = "noRatio";
   glb_datas.txt_col_names.monstats_header[MONSTATS_NOSHLDBLOCK]     = "NoShldBlock";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TOBLOCK]         = "ToBlock";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TOBLOCKN]        = "ToBlock(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TOBLOCKH]        = "ToBlock(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_CRIT]            = "Crit";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MINHP]           = "minHP";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MAXHP]           = "maxHP";
   glb_datas.txt_col_names.monstats_header[MONSTATS_AC]              = "AC";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EXP]             = "Exp";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1MIND]          = "A1MinD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1MAXD]          = "A1MaxD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1TH]            = "A1TH";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2MIND]          = "A2MinD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2MAXD]          = "A2MaxD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2TH]            = "A2TH";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1MIND]          = "S1MinD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1MAXD]          = "S1MaxD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1TH]            = "S1TH";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MINHPN]          = "MinHP(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MAXHPN]          = "MaxHP(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ACN]             = "AC(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EXPN]            = "Exp(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1MINDN]         = "A1MinD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1MAXDN]         = "A1MaxD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1THN]           = "A1TH(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2MINDN]         = "A2MinD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2MAXDN]         = "A2MaxD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2THN]           = "A2TH(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1MINDN]         = "S1MinD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1MAXDN]         = "S1MaxD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1THN]           = "S1TH(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MINHPH]          = "MinHP(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_MAXHPH]          = "MaxHP(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_ACH]             = "AC(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EXPH]            = "Exp(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1MINDH]         = "A1MinD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1MAXDH]         = "A1MaxD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A1THH]           = "A1TH(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2MINDH]         = "A2MinD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2MAXDH]         = "A2MaxD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_A2THH]           = "A2TH(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1MINDH]         = "S1MinD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1MAXDH]         = "S1MaxD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_S1THH]           = "S1TH(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MODE]         = "El1Mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1TYPE]         = "El1Type";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1PCT]          = "El1Pct";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MIND]         = "El1MinD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MAXD]         = "El1MaxD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1DUR]          = "El1Dur";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1PCTN]         = "El1Pct(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MINDN]        = "El1MinD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MAXDN]        = "El1MaxD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1DURN]         = "El1Dur(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1PCTH]         = "El1Pct(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MINDH]        = "El1MinD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1MAXDH]        = "El1MaxD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL1DURH]         = "El1Dur(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MODE]         = "El2Mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2TYPE]         = "El2Type";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2PCT]          = "El2Pct";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MIND]         = "El2MinD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MAXD]         = "El2MaxD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2DUR]          = "El2Dur";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2PCTN]         = "El2Pct(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MINDN]        = "El2MinD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MAXDN]        = "El2MaxD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2DURN]         = "El2Dur(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2PCTH]         = "El2Pct(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MINDH]        = "El2MinD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2MAXDH]        = "El2MaxD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL2DURH]         = "El2Dur(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MODE]         = "El3Mode";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3TYPE]         = "El3Type";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3PCT]          = "El3Pct";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MIND]         = "El3MinD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MAXD]         = "El3MaxD";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3DUR]          = "El3Dur";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3PCTN]         = "El3Pct(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MINDN]        = "El3MinD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MAXDN]        = "El3MaxD(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3DURN]         = "El3Dur(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3PCTH]         = "El3Pct(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MINDH]        = "El3MinD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3MAXDH]        = "El3MaxD(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_EL3DURH]         = "El3Dur(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS1]  = "TreasureClass1";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS2]  = "TreasureClass2";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS3]  = "TreasureClass3";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS4]  = "TreasureClass4";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS1N] = "TreasureClass1(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS2N] = "TreasureClass2(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS3N] = "TreasureClass3(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS4N] = "TreasureClass4(N)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS1H] = "TreasureClass1(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS2H] = "TreasureClass2(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS3H] = "TreasureClass3(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TREASURECLASS4H] = "TreasureClass4(H)";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TCQUESTID]       = "TCQuestId";
   glb_datas.txt_col_names.monstats_header[MONSTATS_TCQUESTCP]       = "TCQuestCP";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPLENDDEATH]     = "SplEndDeath";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPLGETMODECHART] = "SplGetModeChart";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPLENDGENERIC]   = "SplEndGeneric";
   glb_datas.txt_col_names.monstats_header[MONSTATS_SPLCLIENTEND]    = "SplClientEnd";

   // MonStats2.txt
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_ID]              = "Id";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HEIGHT]          = "Height";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_OVERLAYHEIGHT]   = "OverlayHeight";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_PIXHEIGHT]       = "pixHeight";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SIZEX]           = "SizeX";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SIZEY]           = "SizeY";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SPAWNCOL]        = "spawnCol";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MELEERNG]        = "MeleeRng";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_BASEW]           = "BaseW";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HITCLASS]        = "HitClass";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HDV]             = "HDv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_TRV]             = "TRv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LGV]             = "LGv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RAV]             = "Rav";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LAV]             = "Lav";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RHV]             = "RHv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LHV]             = "LHv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SHV]             = "SHv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S1V]             = "S1v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S2V]             = "S2v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S3V]             = "S3v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S4V]             = "S4v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S5V]             = "S5v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S6V]             = "S6v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S7V]             = "S7v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S8V]             = "S8v";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HD]              = "HD";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_TR]              = "TR";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LG]              = "LG";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RA]              = "RA";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LA]              = "LA";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RH]              = "RH";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LH]              = "LH";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SH]              = "SH";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S1]              = "S1";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S2]              = "S2";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S3]              = "S3";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S4]              = "S4";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S5]              = "S5";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S6]              = "S6";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S7]              = "S7";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S8]              = "S8";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_TOTALPIECES]     = "TotalPieces";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MDT]             = "mDT";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MNU]             = "mNU";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MWL]             = "mWL";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MGH]             = "mGH";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MA1]             = "mA1";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MA2]             = "mA2";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MBL]             = "mBL";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MSC]             = "mSC";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MS1]             = "mS1";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MS2]             = "mS2";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MS3]             = "mS3";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MS4]             = "mS4";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MDD]             = "mDD";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MKB]             = "mKB";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MSQ]             = "mSQ";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_MRN]             = "mRN";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DDT]             = "dDT";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DNU]             = "dNU";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DWL]             = "dWL";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DGH]             = "dGH";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DA1]             = "dA1";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DA2]             = "dA2";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DBL]             = "dBL";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DSC]             = "dSC";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DS1]             = "dS1";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DS2]             = "dS2";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DS3]             = "dS3";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DS4]             = "dS4";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DDD]             = "dDD";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DKB]             = "dKB";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DSQ]             = "dSQ";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DRN]             = "dRN";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_A1MV]            = "A1mv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_A2MV]            = "A2mv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SCMV]            = "SCmv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S1MV]            = "S1mv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S2MV]            = "S2mv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S3MV]            = "S3mv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_S4MV]            = "S4mv";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_NOGFXHITTEST]    = "noGfxHitTest";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HTTOP]           = "htTop";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HTLEFT]          = "htLeft";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HTWIDTH]         = "htWidth";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HTHEIGHT]        = "htHeight";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RESTORE]         = "restore";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_AUTOMAPCEL]      = "automapCel";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_NOMAP]           = "noMap";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_NOOVLY]          = "noOvly";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_ISSEL]           = "isSel";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_ALSEL]           = "alSel";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_NOSEL]           = "noSel";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SHIFTSEL]        = "shiftSel";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_CORPSESEL]       = "corpseSel";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_ISATT]           = "isAtt";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_REVIVE]          = "revive";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_CRITTER]         = "critter";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SMALL]           = "small";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LARGE]           = "large";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SOFT]            = "soft";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_INERT]           = "inert";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_OBJCOL]          = "objCol";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_DEADCOL]         = "deadCol";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_UNFLATDEAD]      = "unflatDead";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_SHADOW]          = "Shadow";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_NOUNIQUESHIFT]   = "noUniqueShift";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_COMPOSITEDEATH]  = "compositeDeath";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LOCALBLOOD]      = "localBlood";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_BLEED]           = "Bleed";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LIGHT]           = "Light";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LIGHTR]          = "light-r";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LIGHTG]          = "light-g";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_LIGHTB]          = "light-b";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_UTRANS]          = "Utrans";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_UTRANSN]         = "Utrans(N)";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_UTRANSH]         = "Utrans(H)";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_HEART]           = "Heart";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_BODYPART]        = "BodyPart";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_INFERNOLEN]      = "InfernoLen";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_INFERNOANIM]     = "InfernoAnim";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_INFERNOROLLBACK] = "InfernoRollback";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RESURRECTMODE]   = "ResurrectMode";
   glb_datas.txt_col_names.monstats2_header[MONSTATS2_RESURRECTSKILL]  = "ResurrectSkill";

   // MonType.txt
   glb_datas.txt_col_names.montype_header[MONTYPE_TYPE]    = "type";
   glb_datas.txt_col_names.montype_header[MONTYPE_EQUIV1]  = "equiv1";
   glb_datas.txt_col_names.montype_header[MONTYPE_EQUIV2]  = "equiv2";
   glb_datas.txt_col_names.montype_header[MONTYPE_EQUIV3]  = "equiv3";
   glb_datas.txt_col_names.montype_header[MONTYPE_STRSING] = "strsing";
   glb_datas.txt_col_names.montype_header[MONTYPE_STRPLUR] = "strplur";

   // MonUMod.txt
   glb_datas.txt_col_names.monumod_header[MONUMOD_UNIQUEMOD] = "uniquemod";
   glb_datas.txt_col_names.monumod_header[MONUMOD_ID]        = "id";
   glb_datas.txt_col_names.monumod_header[MONUMOD_ENABLED]   = "enabled";
   glb_datas.txt_col_names.monumod_header[MONUMOD_VERSION]   = "version";
   glb_datas.txt_col_names.monumod_header[MONUMOD_XFER]      = "xfer";
   glb_datas.txt_col_names.monumod_header[MONUMOD_CHAMPION]  = "champion";
   glb_datas.txt_col_names.monumod_header[MONUMOD_FPICK]     = "fPick";
   glb_datas.txt_col_names.monumod_header[MONUMOD_EXCLUDE1]  = "exclude1";
   glb_datas.txt_col_names.monumod_header[MONUMOD_EXCLUDE2]  = "exclude2";
   glb_datas.txt_col_names.monumod_header[MONUMOD_CPICK]     = "cpick";
   glb_datas.txt_col_names.monumod_header[MONUMOD_CPICKN]    = "cpick (N)";
   glb_datas.txt_col_names.monumod_header[MONUMOD_CPICKH]    = "cpick (H)";
   glb_datas.txt_col_names.monumod_header[MONUMOD_UPICK]     = "upick";
   glb_datas.txt_col_names.monumod_header[MONUMOD_UPICKN]    = "upick (N)";
   glb_datas.txt_col_names.monumod_header[MONUMOD_UPICKH]    = "upick (H)";
   glb_datas.txt_col_names.monumod_header[MONUMOD_FINIT]     = "fInit";
   glb_datas.txt_col_names.monumod_header[MONUMOD_CONSTANTS] = "constants";

   // Npc.txt
   glb_datas.txt_col_names.npc_header[NPC_NPC]            = "npc";
   glb_datas.txt_col_names.npc_header[NPC_BUYMULT]        = "buy mult";
   glb_datas.txt_col_names.npc_header[NPC_SELLMULT]       = "sell mult";
   glb_datas.txt_col_names.npc_header[NPC_REPMULT]        = "rep mult";
   glb_datas.txt_col_names.npc_header[NPC_QUESTFLAGA]     = "questflag A";
   glb_datas.txt_col_names.npc_header[NPC_QUESTBUYMULTA]  = "questbuymult A";
   glb_datas.txt_col_names.npc_header[NPC_QUESTSELLMULTA] = "questsellmult A";
   glb_datas.txt_col_names.npc_header[NPC_QUESTREPMULTA]  = "questrepmult A";
   glb_datas.txt_col_names.npc_header[NPC_QUESTFLAGB]     = "questflag B";
   glb_datas.txt_col_names.npc_header[NPC_QUESTBUYMULTB]  = "questbuymult B";
   glb_datas.txt_col_names.npc_header[NPC_QUESTSELLMULTB] = "questsellmult B";
   glb_datas.txt_col_names.npc_header[NPC_QUESTREPMULTB]  = "questrepmult B";
   glb_datas.txt_col_names.npc_header[NPC_QUESTFLAGC]     = "questflag C";
   glb_datas.txt_col_names.npc_header[NPC_QUESTBUYMULTC]  = "questbuymult C";
   glb_datas.txt_col_names.npc_header[NPC_QUESTSELLMULTC] = "questsellmult C";
   glb_datas.txt_col_names.npc_header[NPC_QUESTREPMULTC]  = "questrepmult C";
   glb_datas.txt_col_names.npc_header[NPC_MAXBUY]         = "max buy";
   glb_datas.txt_col_names.npc_header[NPC_MAXBUYN]        = "max buy (N)";
   glb_datas.txt_col_names.npc_header[NPC_MAXBUYH]        = "max buy (H)";

   // Objects.txt
   glb_datas.txt_col_names.objects_header[OBJECTS_NAME]           = "Name";
   glb_datas.txt_col_names.objects_header[OBJECTS_ID]             = "Id";
   glb_datas.txt_col_names.objects_header[OBJECTS_TOKEN]          = "Token";
   glb_datas.txt_col_names.objects_header[OBJECTS_SPAWNMAX]       = "SpawnMax";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE0]    = "Selectable0";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE1]    = "Selectable1";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE2]    = "Selectable2";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE3]    = "Selectable3";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE4]    = "Selectable4";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE5]    = "Selectable5";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE6]    = "Selectable6";
   glb_datas.txt_col_names.objects_header[OBJECTS_SELECTABLE7]    = "Selectable7";
   glb_datas.txt_col_names.objects_header[OBJECTS_TRAPPROB]       = "TrapProb";
   glb_datas.txt_col_names.objects_header[OBJECTS_SIZEX]          = "SizeX";
   glb_datas.txt_col_names.objects_header[OBJECTS_SIZEY]          = "SizeY";
   glb_datas.txt_col_names.objects_header[OBJECTS_NTGTFX]         = "nTgtFX";
   glb_datas.txt_col_names.objects_header[OBJECTS_NTGTFY]         = "nTgtFY";
   glb_datas.txt_col_names.objects_header[OBJECTS_NTGTBX]         = "nTgtBX";
   glb_datas.txt_col_names.objects_header[OBJECTS_NTGTBY]         = "nTgtBY";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT0]      = "FrameCnt0";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT1]      = "FrameCnt1";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT2]      = "FrameCnt2";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT3]      = "FrameCnt3";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT4]      = "FrameCnt4";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT5]      = "FrameCnt5";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT6]      = "FrameCnt6";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMECNT7]      = "FrameCnt7";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA0]    = "FrameDelta0";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA1]    = "FrameDelta1";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA2]    = "FrameDelta2";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA3]    = "FrameDelta3";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA4]    = "FrameDelta4";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA5]    = "FrameDelta5";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA6]    = "FrameDelta6";
   glb_datas.txt_col_names.objects_header[OBJECTS_FRAMEDELTA7]    = "FrameDelta7";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM0]     = "CycleAnim0";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM1]     = "CycleAnim1";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM2]     = "CycleAnim2";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM3]     = "CycleAnim3";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM4]     = "CycleAnim4";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM5]     = "CycleAnim5";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM6]     = "CycleAnim6";
   glb_datas.txt_col_names.objects_header[OBJECTS_CYCLEANIM7]     = "CycleAnim7";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT0]           = "Lit0";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT1]           = "Lit1";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT2]           = "Lit2";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT3]           = "Lit3";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT4]           = "Lit4";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT5]           = "Lit5";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT6]           = "Lit6";
   glb_datas.txt_col_names.objects_header[OBJECTS_LIT7]           = "Lit7";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT0]   = "BlocksLight0";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT1]   = "BlocksLight1";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT2]   = "BlocksLight2";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT3]   = "BlocksLight3";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT4]   = "BlocksLight4";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT5]   = "BlocksLight5";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT6]   = "BlocksLight6";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSLIGHT7]   = "BlocksLight7";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION0]  = "HasCollision0";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION1]  = "HasCollision1";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION2]  = "HasCollision2";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION3]  = "HasCollision3";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION4]  = "HasCollision4";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION5]  = "HasCollision5";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION6]  = "HasCollision6";
   glb_datas.txt_col_names.objects_header[OBJECTS_HASCOLLISION7]  = "HasCollision7";
   glb_datas.txt_col_names.objects_header[OBJECTS_ISATTACKABLE0]  = "IsAttackable0";
   glb_datas.txt_col_names.objects_header[OBJECTS_START0]         = "Start0";
   glb_datas.txt_col_names.objects_header[OBJECTS_START1]         = "Start1";
   glb_datas.txt_col_names.objects_header[OBJECTS_START2]         = "Start2";
   glb_datas.txt_col_names.objects_header[OBJECTS_START3]         = "Start3";
   glb_datas.txt_col_names.objects_header[OBJECTS_START4]         = "Start4";
   glb_datas.txt_col_names.objects_header[OBJECTS_START5]         = "Start5";
   glb_datas.txt_col_names.objects_header[OBJECTS_START6]         = "Start6";
   glb_datas.txt_col_names.objects_header[OBJECTS_START7]         = "Start7";
   glb_datas.txt_col_names.objects_header[OBJECTS_ENVEFFECT]      = "EnvEffect";
   glb_datas.txt_col_names.objects_header[OBJECTS_ISDOOR]         = "IsDoor";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKSVIS]      = "BlocksVis";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORIENTATION]    = "Orientation";
   glb_datas.txt_col_names.objects_header[OBJECTS_TRANS]          = "Trans";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG0]     = "OrderFlag0";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG1]     = "OrderFlag1";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG2]     = "OrderFlag2";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG3]     = "OrderFlag3";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG4]     = "OrderFlag4";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG5]     = "OrderFlag5";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG6]     = "OrderFlag6";
   glb_datas.txt_col_names.objects_header[OBJECTS_ORDERFLAG7]     = "OrderFlag7";
   glb_datas.txt_col_names.objects_header[OBJECTS_PREOPERATE]     = "PreOperate";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE0]          = "Mode0";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE1]          = "Mode1";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE2]          = "Mode2";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE3]          = "Mode3";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE4]          = "Mode4";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE5]          = "Mode5";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE6]          = "Mode6";
   glb_datas.txt_col_names.objects_header[OBJECTS_MODE7]          = "Mode7";
   glb_datas.txt_col_names.objects_header[OBJECTS_YOFFSET]        = "Yoffset";
   glb_datas.txt_col_names.objects_header[OBJECTS_XOFFSET]        = "Xoffset";
   glb_datas.txt_col_names.objects_header[OBJECTS_DRAW]           = "Draw";
   glb_datas.txt_col_names.objects_header[OBJECTS_RED]            = "Red";
   glb_datas.txt_col_names.objects_header[OBJECTS_GREEN]          = "Green";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLUE]           = "Blue";
   glb_datas.txt_col_names.objects_header[OBJECTS_HD]             = "HD";
   glb_datas.txt_col_names.objects_header[OBJECTS_TR]             = "TR";
   glb_datas.txt_col_names.objects_header[OBJECTS_LG]             = "LG";
   glb_datas.txt_col_names.objects_header[OBJECTS_RA]             = "RA";
   glb_datas.txt_col_names.objects_header[OBJECTS_LA]             = "LA";
   glb_datas.txt_col_names.objects_header[OBJECTS_RH]             = "RH";
   glb_datas.txt_col_names.objects_header[OBJECTS_LH]             = "LH";
   glb_datas.txt_col_names.objects_header[OBJECTS_SH]             = "SH";
   glb_datas.txt_col_names.objects_header[OBJECTS_S1]             = "S1";
   glb_datas.txt_col_names.objects_header[OBJECTS_S2]             = "S2";
   glb_datas.txt_col_names.objects_header[OBJECTS_S3]             = "S3";
   glb_datas.txt_col_names.objects_header[OBJECTS_S4]             = "S4";
   glb_datas.txt_col_names.objects_header[OBJECTS_S5]             = "S5";
   glb_datas.txt_col_names.objects_header[OBJECTS_S6]             = "S6";
   glb_datas.txt_col_names.objects_header[OBJECTS_S7]             = "S7";
   glb_datas.txt_col_names.objects_header[OBJECTS_S8]             = "S8";
   glb_datas.txt_col_names.objects_header[OBJECTS_TOTALPIECES]    = "TotalPieces";
   glb_datas.txt_col_names.objects_header[OBJECTS_SUBCLASS]       = "SubClass";
   glb_datas.txt_col_names.objects_header[OBJECTS_XSPACE]         = "Xspace";
   glb_datas.txt_col_names.objects_header[OBJECTS_YSPACE]         = "Yspace";
   glb_datas.txt_col_names.objects_header[OBJECTS_NAMEOFFSET]     = "NameOffset";
   glb_datas.txt_col_names.objects_header[OBJECTS_MONSTEROK]      = "MonsterOK";
   glb_datas.txt_col_names.objects_header[OBJECTS_OPERATERANGE]   = "OperateRange";
   glb_datas.txt_col_names.objects_header[OBJECTS_SHRINEFUNCTION] = "ShrineFunction";
   glb_datas.txt_col_names.objects_header[OBJECTS_RESTORE]        = "Restore";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM0]          = "Parm0";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM1]          = "Parm1";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM2]          = "Parm2";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM3]          = "Parm3";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM4]          = "Parm4";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM5]          = "Parm5";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM6]          = "Parm6";
   glb_datas.txt_col_names.objects_header[OBJECTS_PARM7]          = "Parm7";
   glb_datas.txt_col_names.objects_header[OBJECTS_ACT]            = "Act";
   glb_datas.txt_col_names.objects_header[OBJECTS_LOCKABLE]       = "Lockable";
   glb_datas.txt_col_names.objects_header[OBJECTS_GORE]           = "Gore";
   glb_datas.txt_col_names.objects_header[OBJECTS_SYNC]           = "Sync";
   glb_datas.txt_col_names.objects_header[OBJECTS_FLICKER]        = "Flicker";
   glb_datas.txt_col_names.objects_header[OBJECTS_DAMAGE]         = "Damage";
   glb_datas.txt_col_names.objects_header[OBJECTS_BETA]           = "Beta";
   glb_datas.txt_col_names.objects_header[OBJECTS_OVERLAY]        = "Overlay";
   glb_datas.txt_col_names.objects_header[OBJECTS_COLLISIONSUBST] = "CollisionSubst";
   glb_datas.txt_col_names.objects_header[OBJECTS_LEFT]           = "Left";
   glb_datas.txt_col_names.objects_header[OBJECTS_TOP]            = "Top";
   glb_datas.txt_col_names.objects_header[OBJECTS_WIDTH]          = "Width";
   glb_datas.txt_col_names.objects_header[OBJECTS_HEIGHT]         = "Height";
   glb_datas.txt_col_names.objects_header[OBJECTS_OPERATEFN]      = "OperateFn";
   glb_datas.txt_col_names.objects_header[OBJECTS_POPULATEFN]     = "PopulateFn";
   glb_datas.txt_col_names.objects_header[OBJECTS_INITFN]         = "InitFn";
   glb_datas.txt_col_names.objects_header[OBJECTS_CLIENTFN]       = "ClientFn";
   glb_datas.txt_col_names.objects_header[OBJECTS_RESTOREVIRGINS] = "RestoreVirgins";
   glb_datas.txt_col_names.objects_header[OBJECTS_BLOCKMISSILE]   = "BlockMissile";
   glb_datas.txt_col_names.objects_header[OBJECTS_DRAWUNDER]      = "DrawUnder";
   glb_datas.txt_col_names.objects_header[OBJECTS_OPENWARP]       = "OpenWarp";
   glb_datas.txt_col_names.objects_header[OBJECTS_AUTOMAP]        = "AutoMap";

   // ObjGroup.txt
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_GROUPNAME] = "GroupName";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_OFFSET]    = "Offset";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID0]       = "ID0";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY0]  = "DENSITY0";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB0]     = "PROB0";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID1]       = "ID1";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY1]  = "DENSITY1";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB1]     = "PROB1";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID2]       = "ID2";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY2]  = "DENSITY2";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB2]     = "PROB2";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID3]       = "ID3";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY3]  = "DENSITY3";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB3]     = "PROB3";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID4]       = "ID4";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY4]  = "DENSITY4";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB4]     = "PROB4";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID5]       = "ID5";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY5]  = "DENSITY5";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB5]     = "PROB5";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID6]       = "ID6";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY6]  = "DENSITY6";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB6]     = "PROB6";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_ID7]       = "ID7";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_DENSITY7]  = "DENSITY7";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_PROB7]     = "PROB7";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_SHRINES]   = "SHRINES";
   glb_datas.txt_col_names.objgroup_header[OBJGROUP_WELLS]     = "WELLS";

   // ObjMode.txt
   glb_datas.txt_col_names.objmode_header[OBJMODE_NAME]  = "Name";
   glb_datas.txt_col_names.objmode_header[OBJMODE_TOKEN] = "Token";

   // ObjType.txt
   glb_datas.txt_col_names.objtype_header[OBJTYPE_NAME]  = "Name";
   glb_datas.txt_col_names.objtype_header[OBJTYPE_TOKEN] = "Token";
   glb_datas.txt_col_names.objtype_header[OBJTYPE_BETA]  = "Beta";

   // Overlay.txt
   glb_datas.txt_col_names.overlay_header[OVERLAY_OVERLAY]       = "overlay";
   glb_datas.txt_col_names.overlay_header[OVERLAY_FILENAME]      = "Filename";
   glb_datas.txt_col_names.overlay_header[OVERLAY_VERSION]       = "version";
   glb_datas.txt_col_names.overlay_header[OVERLAY_FRAMES]        = "Frames";
   glb_datas.txt_col_names.overlay_header[OVERLAY_CHARACTER]     = "Character";
   glb_datas.txt_col_names.overlay_header[OVERLAY_PREDRAW]       = "PreDraw";
   glb_datas.txt_col_names.overlay_header[OVERLAY_1OFN]          = "1ofN";
   glb_datas.txt_col_names.overlay_header[OVERLAY_DIR]           = "Dir";
   glb_datas.txt_col_names.overlay_header[OVERLAY_OPEN]          = "Open";
   glb_datas.txt_col_names.overlay_header[OVERLAY_BETA]          = "Beta";
   glb_datas.txt_col_names.overlay_header[OVERLAY_XOFFSET]       = "Xoffset";
   glb_datas.txt_col_names.overlay_header[OVERLAY_YOFFSET]       = "Yoffset";
   glb_datas.txt_col_names.overlay_header[OVERLAY_HEIGHT1]       = "Height1";
   glb_datas.txt_col_names.overlay_header[OVERLAY_HEIGHT2]       = "Height2";
   glb_datas.txt_col_names.overlay_header[OVERLAY_HEIGHT3]       = "Height3";
   glb_datas.txt_col_names.overlay_header[OVERLAY_HEIGHT4]       = "Height4";
   glb_datas.txt_col_names.overlay_header[OVERLAY_ANIMRATE]      = "AnimRate";
   glb_datas.txt_col_names.overlay_header[OVERLAY_LOOPWAITTIME]  = "LoopWaitTime";
   glb_datas.txt_col_names.overlay_header[OVERLAY_TRANS]         = "Trans";
   glb_datas.txt_col_names.overlay_header[OVERLAY_INITRADIUS]    = "InitRadius";
   glb_datas.txt_col_names.overlay_header[OVERLAY_RADIUS]        = "Radius";
   glb_datas.txt_col_names.overlay_header[OVERLAY_RED]           = "Red";
   glb_datas.txt_col_names.overlay_header[OVERLAY_GREEN]         = "Green";
   glb_datas.txt_col_names.overlay_header[OVERLAY_BLUE]          = "Blue";
   glb_datas.txt_col_names.overlay_header[OVERLAY_NUMDIRECTIONS] = "NumDirections";
   glb_datas.txt_col_names.overlay_header[OVERLAY_LOCALBLOOD]    = "LocalBlood";

   // PetType.txt
   glb_datas.txt_col_names.pettype_header[PETTYPE_PETTYPE]   = "pet type";
   glb_datas.txt_col_names.pettype_header[PETTYPE_IDX]       = "idx";
   glb_datas.txt_col_names.pettype_header[PETTYPE_GROUP]     = "group";
   glb_datas.txt_col_names.pettype_header[PETTYPE_BASEMAX]   = "basemax";
   glb_datas.txt_col_names.pettype_header[PETTYPE_WARP]      = "warp";
   glb_datas.txt_col_names.pettype_header[PETTYPE_RANGE]     = "range";
   glb_datas.txt_col_names.pettype_header[PETTYPE_PARTYSEND] = "partysend";
   glb_datas.txt_col_names.pettype_header[PETTYPE_UNSUMMON]  = "unsummon";
   glb_datas.txt_col_names.pettype_header[PETTYPE_AUTOMAP]   = "automap";
   glb_datas.txt_col_names.pettype_header[PETTYPE_NAME]      = "name";
   glb_datas.txt_col_names.pettype_header[PETTYPE_DRAWHP]    = "drawhp";
   glb_datas.txt_col_names.pettype_header[PETTYPE_ICONTYPE]  = "icontype";
   glb_datas.txt_col_names.pettype_header[PETTYPE_BASEICON]  = "baseicon";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MCLASS1]   = "mclass1";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MICON1]    = "micon1";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MCLASS2]   = "mclass2";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MICON2]    = "micon2";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MCLASS3]   = "mclass3";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MICON3]    = "micon3";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MCLASS4]   = "mclass4";
   glb_datas.txt_col_names.pettype_header[PETTYPE_MICON4]    = "micon4";

   // PlayerClass.txt
   glb_datas.txt_col_names.playerclass_header[PLAYERCLASS_PLAYERCLASS] = "Player Class";
   glb_datas.txt_col_names.playerclass_header[PLAYERCLASS_CODE]        = "Code";

   // PlrMode.txt
   glb_datas.txt_col_names.plrmode_header[PLRMODE_NAME]  = "Name";
   glb_datas.txt_col_names.plrmode_header[PLRMODE_TOKEN] = "Token";
   glb_datas.txt_col_names.plrmode_header[PLRMODE_CODE]  = "Code";

   // PlrType.txt
   glb_datas.txt_col_names.plrtype_header[PLRTYPE_NAME]  = "Name";
   glb_datas.txt_col_names.plrtype_header[PLRTYPE_TOKEN] = "Token";

   // Properties.txt
   glb_datas.txt_col_names.properties_header[PROPERTIES_CODE]  = "code";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET1]  = "set1";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL1]  = "val1";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC1] = "func1";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT1] = "stat1";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET2]  = "set2";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL2]  = "val2";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC2] = "func2";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT2] = "stat2";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET3]  = "set3";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL3]  = "val3";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC3] = "func3";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT3] = "stat3";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET4]  = "set4";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL4]  = "val4";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC4] = "func4";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT4] = "stat4";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET5]  = "set5";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL5]  = "val5";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC5] = "func5";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT5] = "stat5";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET6]  = "set6";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL6]  = "val6";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC6] = "func6";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT6] = "stat6";
   glb_datas.txt_col_names.properties_header[PROPERTIES_SET7]  = "set7";
   glb_datas.txt_col_names.properties_header[PROPERTIES_VAL7]  = "val7";
   glb_datas.txt_col_names.properties_header[PROPERTIES_FUNC7] = "func7";
   glb_datas.txt_col_names.properties_header[PROPERTIES_STAT7] = "stat7";

   // QualityItems.txt
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_NUMMODS]   = "nummods";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD1CODE]  = "mod1code";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD1PARAM] = "mod1param";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD1MIN]   = "mod1min";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD1MAX]   = "mod1max";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD2CODE]  = "mod2code";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD2PARAM] = "mod2param";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD2MIN]   = "mod2min";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MOD2MAX]   = "mod2max";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_TOHITMIN]  = "ToHitMin";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_TOHITMAX]  = "ToHitMax";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_DAMMIN]    = "Dam%Min";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_DAMMAX]    = "Dam%Max";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_ACMIN]     = "AC%Min";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_ACMAX]     = "AC%Max";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_DURMIN]    = "Dur%Min";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_DURMAX]    = "Dur%Max";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_EFFECT1]   = "effect1";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_EFFECT2]   = "effect2";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_ARMOR]     = "armor";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_WEAPON]    = "weapon";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_SHIELD]    = "shield";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_THROWN]    = "thrown";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_SCEPTER]   = "scepter";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_WAND]      = "wand";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_STAFF]     = "staff";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_BOW]       = "bow";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_BOOTS]     = "boots";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_GLOVES]    = "gloves";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_BELT]      = "belt";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_LEVEL]     = "level";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_MULTIPLY]  = "multiply";
   glb_datas.txt_col_names.qualityitems_header[QUALITYITEMS_ADD]       = "add";

   // RarePrefix.txt + RareSuffix.txt
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_NAME]     = "name";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_VERSION]  = "version";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE1]   = "itype1";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE2]   = "itype2";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE3]   = "itype3";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE4]   = "itype4";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE5]   = "itype5";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE6]   = "itype6";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ITYPE7]   = "itype7";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ETYPE1]   = "etype1";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ETYPE2]   = "etype2";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ETYPE3]   = "etype3";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ETYPE4]   = "etype4";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_ADD]      = "add";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_MULTIPLY] = "multiply";
   glb_datas.txt_col_names.rareaffix_header[RAREAFFIX_DIVIDE]   = "divide";

   // Runes.txt
   glb_datas.txt_col_names.runes_header[RUNES_NAME]     = "Name";
   glb_datas.txt_col_names.runes_header[RUNES_RUNENAME] = "Rune Name";
   glb_datas.txt_col_names.runes_header[RUNES_COMPLETE] = "complete";
   glb_datas.txt_col_names.runes_header[RUNES_SERVER]   = "server";
   glb_datas.txt_col_names.runes_header[RUNES_ITYPE1]   = "itype1";
   glb_datas.txt_col_names.runes_header[RUNES_ITYPE2]   = "itype2";
   glb_datas.txt_col_names.runes_header[RUNES_ITYPE3]   = "itype3";
   glb_datas.txt_col_names.runes_header[RUNES_ITYPE4]   = "itype4";
   glb_datas.txt_col_names.runes_header[RUNES_ITYPE5]   = "itype5";
   glb_datas.txt_col_names.runes_header[RUNES_ITYPE6]   = "itype6";
   glb_datas.txt_col_names.runes_header[RUNES_ETYPE1]   = "etype1";
   glb_datas.txt_col_names.runes_header[RUNES_ETYPE2]   = "etype2";
   glb_datas.txt_col_names.runes_header[RUNES_ETYPE3]   = "etype3";
   glb_datas.txt_col_names.runes_header[RUNES_RUNE1]    = "Rune1";
   glb_datas.txt_col_names.runes_header[RUNES_RUNE2]    = "Rune2";
   glb_datas.txt_col_names.runes_header[RUNES_RUNE3]    = "Rune3";
   glb_datas.txt_col_names.runes_header[RUNES_RUNE4]    = "Rune4";
   glb_datas.txt_col_names.runes_header[RUNES_RUNE5]    = "Rune5";
   glb_datas.txt_col_names.runes_header[RUNES_RUNE6]    = "Rune6";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE1]  = "T1Code1";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM1] = "T1Param1";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN1]   = "T1Min1";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX1]   = "T1Max1";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE2]  = "T1Code2";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM2] = "T1Param2";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN2]   = "T1Min2";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX2]   = "T1Max2";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE3]  = "T1Code3";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM3] = "T1Param3";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN3]   = "T1Min3";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX3]   = "T1Max3";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE4]  = "T1Code4";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM4] = "T1Param4";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN4]   = "T1Min4";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX4]   = "T1Max4";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE5]  = "T1Code5";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM5] = "T1Param5";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN5]   = "T1Min5";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX5]   = "T1Max5";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE6]  = "T1Code6";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM6] = "T1Param6";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN6]   = "T1Min6";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX6]   = "T1Max6";
   glb_datas.txt_col_names.runes_header[RUNES_T1CODE7]  = "T1Code7";
   glb_datas.txt_col_names.runes_header[RUNES_T1PARAM7] = "T1Param7";
   glb_datas.txt_col_names.runes_header[RUNES_T1MIN7]   = "T1Min7";
   glb_datas.txt_col_names.runes_header[RUNES_T1MAX7]   = "T1Max7";

   // SetItems.txt
   glb_datas.txt_col_names.setitems_header[SETITEMS_INDEX]        = "index";
   glb_datas.txt_col_names.setitems_header[SETITEMS_SET]          = "set";
   glb_datas.txt_col_names.setitems_header[SETITEMS_ITEM]         = "item";
   glb_datas.txt_col_names.setitems_header[SETITEMS_RARITY]       = "rarity";
   glb_datas.txt_col_names.setitems_header[SETITEMS_LVL]          = "lvl";
   glb_datas.txt_col_names.setitems_header[SETITEMS_LVLREQ]       = "lvl req";
   glb_datas.txt_col_names.setitems_header[SETITEMS_CHRTRANSFORM] = "chrtransform";
   glb_datas.txt_col_names.setitems_header[SETITEMS_INVTRANSFORM] = "invtransform";
   glb_datas.txt_col_names.setitems_header[SETITEMS_INVFILE]      = "invfile";
   glb_datas.txt_col_names.setitems_header[SETITEMS_FLIPPYFILE]   = "flippyfile";
   glb_datas.txt_col_names.setitems_header[SETITEMS_DROPSOUND]    = "dropsound";
   glb_datas.txt_col_names.setitems_header[SETITEMS_DROPSFXFRAME] = "dropsfxframe";
   glb_datas.txt_col_names.setitems_header[SETITEMS_USESOUND]     = "usesound";
   glb_datas.txt_col_names.setitems_header[SETITEMS_COSTMULT]     = "cost mult";
   glb_datas.txt_col_names.setitems_header[SETITEMS_COSTADD]      = "cost add";
   glb_datas.txt_col_names.setitems_header[SETITEMS_ADDFUNC]      = "add func";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP1]        = "prop1";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR1]         = "par1";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN1]         = "min1";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX1]         = "max1";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP2]        = "prop2";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR2]         = "par2";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN2]         = "min2";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX2]         = "max2";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP3]        = "prop3";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR3]         = "par3";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN3]         = "min3";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX3]         = "max3";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP4]        = "prop4";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR4]         = "par4";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN4]         = "min4";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX4]         = "max4";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP5]        = "prop5";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR5]         = "par5";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN5]         = "min5";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX5]         = "max5";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP6]        = "prop6";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR6]         = "par6";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN6]         = "min6";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX6]         = "max6";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP7]        = "prop7";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR7]         = "par7";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN7]         = "min7";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX7]         = "max7";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP8]        = "prop8";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR8]         = "par8";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN8]         = "min8";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX8]         = "max8";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PROP9]        = "prop9";
   glb_datas.txt_col_names.setitems_header[SETITEMS_PAR9]         = "par9";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MIN9]         = "min9";
   glb_datas.txt_col_names.setitems_header[SETITEMS_MAX9]         = "max9";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP1A]      = "aprop1a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR1A]       = "apar1a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN1A]       = "amin1a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX1A]       = "amax1a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP1B]      = "aprop1b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR1B]       = "apar1b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN1B]       = "amin1b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX1B]       = "amax1b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP2A]      = "aprop2a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR2A]       = "apar2a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN2A]       = "amin2a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX2A]       = "amax2a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP2B]      = "aprop2b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR2B]       = "apar2b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN2B]       = "amin2b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX2B]       = "amax2b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP3A]      = "aprop3a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR3A]       = "apar3a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN3A]       = "amin3a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX3A]       = "amax3a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP3B]      = "aprop3b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR3B]       = "apar3b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN3B]       = "amin3b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX3B]       = "amax3b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP4A]      = "aprop4a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR4A]       = "apar4a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN4A]       = "amin4a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX4A]       = "amax4a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP4B]      = "aprop4b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR4B]       = "apar4b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN4B]       = "amin4b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX4B]       = "amax4b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP5A]      = "aprop5a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR5A]       = "apar5a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN5A]       = "amin5a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX5A]       = "amax5a";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APROP5B]      = "aprop5b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_APAR5B]       = "apar5b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMIN5B]       = "amin5b";
   glb_datas.txt_col_names.setitems_header[SETITEMS_AMAX5B]       = "amax5b";

   // Sets.txt
   glb_datas.txt_col_names.sets_header[SETS_INDEX]    = "index";
   glb_datas.txt_col_names.sets_header[SETS_NAME]     = "name";
   glb_datas.txt_col_names.sets_header[SETS_VERSION]  = "version";
   glb_datas.txt_col_names.sets_header[SETS_LEVEL]    = "level";
   glb_datas.txt_col_names.sets_header[SETS_PCODE2A]  = "PCode2a";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM2A] = "PParam2a";
   glb_datas.txt_col_names.sets_header[SETS_PMIN2A]   = "PMin2a";
   glb_datas.txt_col_names.sets_header[SETS_PMAX2A]   = "PMax2a";
   glb_datas.txt_col_names.sets_header[SETS_PCODE2B]  = "PCode2b";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM2B] = "PParam2b";
   glb_datas.txt_col_names.sets_header[SETS_PMIN2B]   = "PMin2b";
   glb_datas.txt_col_names.sets_header[SETS_PMAX2B]   = "PMax2b";
   glb_datas.txt_col_names.sets_header[SETS_PCODE3A]  = "PCode3a";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM3A] = "PParam3a";
   glb_datas.txt_col_names.sets_header[SETS_PMIN3A]   = "PMin3a";
   glb_datas.txt_col_names.sets_header[SETS_PMAX3A]   = "PMax3a";
   glb_datas.txt_col_names.sets_header[SETS_PCODE3B]  = "PCode3b";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM3B] = "PParam3b";
   glb_datas.txt_col_names.sets_header[SETS_PMIN3B]   = "PMin3b";
   glb_datas.txt_col_names.sets_header[SETS_PMAX3B]   = "PMax3b";
   glb_datas.txt_col_names.sets_header[SETS_PCODE4A]  = "PCode4a";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM4A] = "PParam4a";
   glb_datas.txt_col_names.sets_header[SETS_PMIN4A]   = "PMin4a";
   glb_datas.txt_col_names.sets_header[SETS_PMAX4A]   = "PMax4a";
   glb_datas.txt_col_names.sets_header[SETS_PCODE4B]  = "PCode4b";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM4B] = "PParam4b";
   glb_datas.txt_col_names.sets_header[SETS_PMIN4B]   = "PMin4b";
   glb_datas.txt_col_names.sets_header[SETS_PMAX4B]   = "PMax4b";
   glb_datas.txt_col_names.sets_header[SETS_PCODE5A]  = "PCode5a";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM5A] = "PParam5a";
   glb_datas.txt_col_names.sets_header[SETS_PMIN5A]   = "PMin5a";
   glb_datas.txt_col_names.sets_header[SETS_PMAX5A]   = "PMax5a";
   glb_datas.txt_col_names.sets_header[SETS_PCODE5B]  = "PCode5b";
   glb_datas.txt_col_names.sets_header[SETS_PPARAM5B] = "PParam5b";
   glb_datas.txt_col_names.sets_header[SETS_PMIN5B]   = "PMin5b";
   glb_datas.txt_col_names.sets_header[SETS_PMAX5B]   = "PMax5b";
   glb_datas.txt_col_names.sets_header[SETS_FCODE1]   = "FCode1";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM1]  = "FParam1";
   glb_datas.txt_col_names.sets_header[SETS_FMIN1]    = "FMin1";
   glb_datas.txt_col_names.sets_header[SETS_FMAX1]    = "FMax1";
   glb_datas.txt_col_names.sets_header[SETS_FCODE2]   = "FCode2";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM2]  = "FParam2";
   glb_datas.txt_col_names.sets_header[SETS_FMIN2]    = "FMin2";
   glb_datas.txt_col_names.sets_header[SETS_FMAX2]    = "FMax2";
   glb_datas.txt_col_names.sets_header[SETS_FCODE3]   = "FCode3";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM3]  = "FParam3";
   glb_datas.txt_col_names.sets_header[SETS_FMIN3]    = "FMin3";
   glb_datas.txt_col_names.sets_header[SETS_FMAX3]    = "FMax3";
   glb_datas.txt_col_names.sets_header[SETS_FCODE4]   = "FCode4";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM4]  = "FParam4";
   glb_datas.txt_col_names.sets_header[SETS_FMIN4]    = "FMin4";
   glb_datas.txt_col_names.sets_header[SETS_FMAX4]    = "FMax4";
   glb_datas.txt_col_names.sets_header[SETS_FCODE5]   = "FCode5";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM5]  = "FParam5";
   glb_datas.txt_col_names.sets_header[SETS_FMIN5]    = "FMin5";
   glb_datas.txt_col_names.sets_header[SETS_FMAX5]    = "FMax5";
   glb_datas.txt_col_names.sets_header[SETS_FCODE6]   = "FCode6";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM6]  = "FParam6";
   glb_datas.txt_col_names.sets_header[SETS_FMIN6]    = "FMin6";
   glb_datas.txt_col_names.sets_header[SETS_FMAX6]    = "FMax6";
   glb_datas.txt_col_names.sets_header[SETS_FCODE7]   = "FCode7";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM7]  = "FParam7";
   glb_datas.txt_col_names.sets_header[SETS_FMIN7]    = "FMin7";
   glb_datas.txt_col_names.sets_header[SETS_FMAX7]    = "FMax7";
   glb_datas.txt_col_names.sets_header[SETS_FCODE8]   = "FCode8";
   glb_datas.txt_col_names.sets_header[SETS_FPARAM8]  = "FParam8";
   glb_datas.txt_col_names.sets_header[SETS_FMIN8]    = "FMin8";
   glb_datas.txt_col_names.sets_header[SETS_FMAX8]    = "FMax8";

   // Shrines.txt
   glb_datas.txt_col_names.shrines_header[SHRINES_SHRINETYPE]         = "Shrine Type";
   glb_datas.txt_col_names.shrines_header[SHRINES_SHRINENAME]         = "Shrine name";
   glb_datas.txt_col_names.shrines_header[SHRINES_EFFECT]             = "Effect";
   glb_datas.txt_col_names.shrines_header[SHRINES_CODE]               = "Code";
   glb_datas.txt_col_names.shrines_header[SHRINES_ARG0]               = "Arg0";
   glb_datas.txt_col_names.shrines_header[SHRINES_ARG1]               = "Arg1";
   glb_datas.txt_col_names.shrines_header[SHRINES_DURATIONINFRAMES]   = "Duration in frames";
   glb_datas.txt_col_names.shrines_header[SHRINES_RESETTIMEINMINUTES] = "reset time in minutes";
   glb_datas.txt_col_names.shrines_header[SHRINES_RARITY]             = "rarity";
   glb_datas.txt_col_names.shrines_header[SHRINES_VIEWNAME]           = "view name";
   glb_datas.txt_col_names.shrines_header[SHRINES_NIFTYPHRASE]        = "niftyphrase";
   glb_datas.txt_col_names.shrines_header[SHRINES_EFFECTCLASS]        = "effectclass";
   glb_datas.txt_col_names.shrines_header[SHRINES_LEVELMIN]           = "LevelMin";

   // SkillCalc.txt
   glb_datas.txt_col_names.skillcalc_header[SKILLCALC_CODE] = "code";

   // SkillDesc.txt
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_SKILLDESC]    = "skilldesc";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_SKILLPAGE]    = "SkillPage";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_SKILLROW]     = "SkillRow";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_SKILLCOLUMN]  = "SkillColumn";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_LISTROW]      = "ListRow";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_LISTPOOL]     = "ListPool";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_ICONCEL]      = "IconCel";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_STRNAME]      = "str name";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_STRSHORT]     = "str short";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_STRLONG]      = "str long";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_STRALT]       = "str alt";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_STRMANA]      = "str mana";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCDAM]      = "descdam";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DDAMCALC1]    = "ddam calc1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DDAMCALC2]    = "ddam calc2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P1DMELEM]     = "p1dmelem";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P1DMMIN]      = "p1dmmin";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P1DMMAX]      = "p1dmmax";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P2DMELEM]     = "p2dmelem";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P2DMMIN]      = "p2dmmin";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P2DMMAX]      = "p2dmmax";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P3DMELEM]     = "p3dmelem";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P3DMMIN]      = "p3dmmin";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_P3DMMAX]      = "p3dmmax";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCATT]      = "descatt";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCMISSILE1] = "descmissile1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCMISSILE2] = "descmissile2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCMISSILE3] = "descmissile3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCLINE1]    = "descline1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTA1]   = "desctexta1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTB1]   = "desctextb1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCA1]   = "desccalca1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCB1]   = "desccalcb1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCLINE2]    = "descline2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTA2]   = "desctexta2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTB2]   = "desctextb2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCA2]   = "desccalca2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCB2]   = "desccalcb2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCLINE3]    = "descline3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTA3]   = "desctexta3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTB3]   = "desctextb3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCA3]   = "desccalca3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCB3]   = "desccalcb3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCLINE4]    = "descline4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTA4]   = "desctexta4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTB4]   = "desctextb4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCA4]   = "desccalca4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCB4]   = "desccalcb4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCLINE5]    = "descline5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTA5]   = "desctexta5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTB5]   = "desctextb5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCA5]   = "desccalca5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCB5]   = "desccalcb5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCLINE6]    = "descline6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTA6]   = "desctexta6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCTEXTB6]   = "desctextb6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCA6]   = "desccalca6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DESCCALCB6]   = "desccalcb6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2LINE1]    = "dsc2line1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTA1]   = "dsc2texta1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTB1]   = "dsc2textb1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCA1]   = "dsc2calca1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCB1]   = "dsc2calcb1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2LINE2]    = "dsc2line2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTA2]   = "dsc2texta2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTB2]   = "dsc2textb2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCA2]   = "dsc2calca2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCB2]   = "dsc2calcb2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2LINE3]    = "dsc2line3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTA3]   = "dsc2texta3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTB3]   = "dsc2textb3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCA3]   = "dsc2calca3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCB3]   = "dsc2calcb3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2LINE4]    = "dsc2line4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTA4]   = "dsc2texta4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2TEXTB4]   = "dsc2textb4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCA4]   = "dsc2calca4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC2CALCB4]   = "dsc2calcb4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE1]    = "dsc3line1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA1]   = "dsc3texta1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB1]   = "dsc3textb1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA1]   = "dsc3calca1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB1]   = "dsc3calcb1";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE2]    = "dsc3line2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA2]   = "dsc3texta2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB2]   = "dsc3textb2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA2]   = "dsc3calca2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB2]   = "dsc3calcb2";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE3]    = "dsc3line3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA3]   = "dsc3texta3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB3]   = "dsc3textb3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA3]   = "dsc3calca3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB3]   = "dsc3calcb3";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE4]    = "dsc3line4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA4]   = "dsc3texta4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB4]   = "dsc3textb4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA4]   = "dsc3calca4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB4]   = "dsc3calcb4";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE5]    = "dsc3line5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA5]   = "dsc3texta5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB5]   = "dsc3textb5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA5]   = "dsc3calca5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB5]   = "dsc3calcb5";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE6]    = "dsc3line6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA6]   = "dsc3texta6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB6]   = "dsc3textb6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA6]   = "dsc3calca6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB6]   = "dsc3calcb6";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3LINE7]    = "dsc3line7";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTA7]   = "dsc3texta7";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3TEXTB7]   = "dsc3textb7";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCA7]   = "dsc3calca7";
   glb_datas.txt_col_names.skilldesc_header[SKILLDESC_DSC3CALCB7]   = "dsc3calcb7";

   // Skills.txt
   glb_datas.txt_col_names.skills_header[SKILLS_SKILL]             = "skill";
   glb_datas.txt_col_names.skills_header[SKILLS_ID]                = "Id";
   glb_datas.txt_col_names.skills_header[SKILLS_CHARCLASS]         = "charclass";
   glb_datas.txt_col_names.skills_header[SKILLS_SKILLDESC]         = "skilldesc";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVSTFUNC]         = "srvstfunc";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVDOFUNC]         = "srvdofunc";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGSTACK]          = "prgstack";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVPRGFUNC1]       = "srvprgfunc1";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVPRGFUNC2]       = "srvprgfunc2";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVPRGFUNC3]       = "srvprgfunc3";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGCALC1]          = "prgcalc1";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGCALC2]          = "prgcalc2";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGCALC3]          = "prgcalc3";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGDAM]            = "prgdam";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVMISSILE]        = "srvmissile";
   glb_datas.txt_col_names.skills_header[SKILLS_DECQUANT]          = "decquant";
   glb_datas.txt_col_names.skills_header[SKILLS_LOB]               = "lob";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVMISSILEA]       = "srvmissilea";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVMISSILEB]       = "srvmissileb";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVMISSILEC]       = "srvmissilec";
   glb_datas.txt_col_names.skills_header[SKILLS_SRVOVERLAY]        = "srvoverlay";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAFILTER]        = "aurafilter";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATE]         = "aurastate";
   glb_datas.txt_col_names.skills_header[SKILLS_AURATARGETSTATE]   = "auratargetstate";
   glb_datas.txt_col_names.skills_header[SKILLS_AURALENCALC]       = "auralencalc";
   glb_datas.txt_col_names.skills_header[SKILLS_AURARANGECALC]     = "aurarangecalc";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTAT1]         = "aurastat1";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATCALC1]     = "aurastatcalc1";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTAT2]         = "aurastat2";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATCALC2]     = "aurastatcalc2";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTAT3]         = "aurastat3";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATCALC3]     = "aurastatcalc3";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTAT4]         = "aurastat4";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATCALC4]     = "aurastatcalc4";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTAT5]         = "aurastat5";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATCALC5]     = "aurastatcalc5";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTAT6]         = "aurastat6";
   glb_datas.txt_col_names.skills_header[SKILLS_AURASTATCALC6]     = "aurastatcalc6";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAEVENT1]        = "auraevent1";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAEVENTFUNC1]    = "auraeventfunc1";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAEVENT2]        = "auraevent2";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAEVENTFUNC2]    = "auraeventfunc2";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAEVENT3]        = "auraevent3";
   glb_datas.txt_col_names.skills_header[SKILLS_AURAEVENTFUNC3]    = "auraeventfunc3";
   glb_datas.txt_col_names.skills_header[SKILLS_AURATGTEVENT]      = "auratgtevent";
   glb_datas.txt_col_names.skills_header[SKILLS_AURATGTEVENTFUNC]  = "auratgteventfunc";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVESTATE]      = "passivestate";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVEITYPE]      = "passiveitype";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVESTAT1]      = "passivestat1";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVECALC1]      = "passivecalc1";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVESTAT2]      = "passivestat2";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVECALC2]      = "passivecalc2";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVESTAT3]      = "passivestat3";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVECALC3]      = "passivecalc3";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVESTAT4]      = "passivestat4";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVECALC4]      = "passivecalc4";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVESTAT5]      = "passivestat5";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVECALC5]      = "passivecalc5";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVEEVENT]      = "passiveevent";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVEEVENTFUNC]  = "passiveeventfunc";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMMON]            = "summon";
   glb_datas.txt_col_names.skills_header[SKILLS_PETTYPE]           = "pettype";
   glb_datas.txt_col_names.skills_header[SKILLS_PETMAX]            = "petmax";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMMODE]           = "summode";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSKILL1]         = "sumskill1";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSK1CALC]        = "sumsk1calc";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSKILL2]         = "sumskill2";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSK2CALC]        = "sumsk2calc";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSKILL3]         = "sumskill3";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSK3CALC]        = "sumsk3calc";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSKILL4]         = "sumskill4";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSK4CALC]        = "sumsk4calc";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSKILL5]         = "sumskill5";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMSK5CALC]        = "sumsk5calc";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMUMOD]           = "sumumod";
   glb_datas.txt_col_names.skills_header[SKILLS_SUMOVERLAY]        = "sumoverlay";
   glb_datas.txt_col_names.skills_header[SKILLS_STSUCCESSONLY]     = "stsuccessonly";
   glb_datas.txt_col_names.skills_header[SKILLS_STSOUND]           = "stsound";
   glb_datas.txt_col_names.skills_header[SKILLS_STSOUNDCLASS]      = "stsoundclass";
   glb_datas.txt_col_names.skills_header[SKILLS_STSOUNDDELAY]      = "stsounddelay";
   glb_datas.txt_col_names.skills_header[SKILLS_WEAPONSND]         = "weaponsnd";
   glb_datas.txt_col_names.skills_header[SKILLS_DOSOUND]           = "dosound";
   glb_datas.txt_col_names.skills_header[SKILLS_DOSOUNDA]          = "dosound a";
   glb_datas.txt_col_names.skills_header[SKILLS_DOSOUNDB]          = "dosound b";
   glb_datas.txt_col_names.skills_header[SKILLS_TGTOVERLAY]        = "tgtoverlay";
   glb_datas.txt_col_names.skills_header[SKILLS_TGTSOUND]          = "tgtsound";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGOVERLAY]        = "prgoverlay";
   glb_datas.txt_col_names.skills_header[SKILLS_PRGSOUND]          = "prgsound";
   glb_datas.txt_col_names.skills_header[SKILLS_CASTOVERLAY]       = "castoverlay";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTOVERLAYA]       = "cltoverlaya";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTOVERLAYB]       = "cltoverlayb";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTSTFUNC]         = "cltstfunc";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTDOFUNC]         = "cltdofunc";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTPRGFUNC1]       = "cltprgfunc1";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTPRGFUNC2]       = "cltprgfunc2";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTPRGFUNC3]       = "cltprgfunc3";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTMISSILE]        = "cltmissile";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTMISSILEA]       = "cltmissilea";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTMISSILEB]       = "cltmissileb";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTMISSILEC]       = "cltmissilec";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTMISSILED]       = "cltmissiled";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTCALC1]          = "cltcalc1";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTCALC2]          = "cltcalc2";
   glb_datas.txt_col_names.skills_header[SKILLS_CLTCALC3]          = "cltcalc3";
   glb_datas.txt_col_names.skills_header[SKILLS_WARP]              = "warp";
   glb_datas.txt_col_names.skills_header[SKILLS_IMMEDIATE]         = "immediate";
   glb_datas.txt_col_names.skills_header[SKILLS_ENHANCEABLE]       = "enhanceable";
   glb_datas.txt_col_names.skills_header[SKILLS_ATTACKRANK]        = "attackrank";
   glb_datas.txt_col_names.skills_header[SKILLS_NOAMMO]            = "noammo";
   glb_datas.txt_col_names.skills_header[SKILLS_RANGE]             = "range";
   glb_datas.txt_col_names.skills_header[SKILLS_WEAPSEL]           = "weapsel";
   glb_datas.txt_col_names.skills_header[SKILLS_ITYPEA1]           = "itypea1";
   glb_datas.txt_col_names.skills_header[SKILLS_ITYPEA2]           = "itypea2";
   glb_datas.txt_col_names.skills_header[SKILLS_ITYPEA3]           = "itypea3";
   glb_datas.txt_col_names.skills_header[SKILLS_ETYPEA1]           = "etypea1";
   glb_datas.txt_col_names.skills_header[SKILLS_ETYPEA2]           = "etypea2";
   glb_datas.txt_col_names.skills_header[SKILLS_ITYPEB1]           = "itypeb1";
   glb_datas.txt_col_names.skills_header[SKILLS_ITYPEB2]           = "itypeb2";
   glb_datas.txt_col_names.skills_header[SKILLS_ITYPEB3]           = "itypeb3";
   glb_datas.txt_col_names.skills_header[SKILLS_ETYPEB1]           = "etypeb1";
   glb_datas.txt_col_names.skills_header[SKILLS_ETYPEB2]           = "etypeb2";
   glb_datas.txt_col_names.skills_header[SKILLS_ANIM]              = "anim";
   glb_datas.txt_col_names.skills_header[SKILLS_SEQTRANS]          = "seqtrans";
   glb_datas.txt_col_names.skills_header[SKILLS_MONANIM]           = "monanim";
   glb_datas.txt_col_names.skills_header[SKILLS_SEQNUM]            = "seqnum";
   glb_datas.txt_col_names.skills_header[SKILLS_SEQINPUT]          = "seqinput";
   glb_datas.txt_col_names.skills_header[SKILLS_DURABILITY]        = "durability";
   glb_datas.txt_col_names.skills_header[SKILLS_USEATTACKRATE]     = "UseAttackRate";
   glb_datas.txt_col_names.skills_header[SKILLS_LINEOFSIGHT]       = "LineOfSight";
   glb_datas.txt_col_names.skills_header[SKILLS_TARGETABLEONLY]    = "TargetableOnly";
   glb_datas.txt_col_names.skills_header[SKILLS_SEARCHENEMYXY]     = "SearchEnemyXY";
   glb_datas.txt_col_names.skills_header[SKILLS_SEARCHENEMYNEAR]   = "SearchEnemyNear";
   glb_datas.txt_col_names.skills_header[SKILLS_SEARCHOPENXY]      = "SearchOpenXY";
   glb_datas.txt_col_names.skills_header[SKILLS_SELECTPROC]        = "SelectProc";
   glb_datas.txt_col_names.skills_header[SKILLS_TARGETCORPSE]      = "TargetCorpse";
   glb_datas.txt_col_names.skills_header[SKILLS_TARGETPET]         = "TargetPet";
   glb_datas.txt_col_names.skills_header[SKILLS_TARGETALLY]        = "TargetAlly";
   glb_datas.txt_col_names.skills_header[SKILLS_TARGETITEM]        = "TargetItem";
   glb_datas.txt_col_names.skills_header[SKILLS_ATTACKNOMANA]      = "AttackNoMana";
   glb_datas.txt_col_names.skills_header[SKILLS_TGTPLACECHECK]     = "TgtPlaceCheck";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMEFFECT]        = "ItemEffect";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMCLTEFFECT]     = "ItemCltEffect";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMTGTDO]         = "ItemTgtDo";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMTARGET]        = "ItemTarget";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMCHECKSTART]    = "ItemCheckStart";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMCLTCHECKSTART] = "ItemCltCheckStart";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMCASTSOUND]     = "ItemCastSound";
   glb_datas.txt_col_names.skills_header[SKILLS_ITEMCASTOVERLAY]   = "ItemCastOverlay";
   glb_datas.txt_col_names.skills_header[SKILLS_SKPOINTS]          = "skpoints";
   glb_datas.txt_col_names.skills_header[SKILLS_REQLEVEL]          = "reqlevel";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXLVL]            = "maxlvl";
   glb_datas.txt_col_names.skills_header[SKILLS_REQSTR]            = "reqstr";
   glb_datas.txt_col_names.skills_header[SKILLS_REQDEX]            = "reqdex";
   glb_datas.txt_col_names.skills_header[SKILLS_REQINT]            = "reqint";
   glb_datas.txt_col_names.skills_header[SKILLS_REQVIT]            = "reqvit";
   glb_datas.txt_col_names.skills_header[SKILLS_REQSKILL1]         = "reqskill1";
   glb_datas.txt_col_names.skills_header[SKILLS_REQSKILL2]         = "reqskill2";
   glb_datas.txt_col_names.skills_header[SKILLS_REQSKILL3]         = "reqskill3";
   glb_datas.txt_col_names.skills_header[SKILLS_RESTRICT]          = "restrict";
   glb_datas.txt_col_names.skills_header[SKILLS_STATE1]            = "State1";
   glb_datas.txt_col_names.skills_header[SKILLS_STATE2]            = "State2";
   glb_datas.txt_col_names.skills_header[SKILLS_STATE3]            = "State3";
   glb_datas.txt_col_names.skills_header[SKILLS_DELAY]             = "delay";
   glb_datas.txt_col_names.skills_header[SKILLS_LEFTSKILL]         = "leftskill";
   glb_datas.txt_col_names.skills_header[SKILLS_REPEAT]            = "repeat";
   glb_datas.txt_col_names.skills_header[SKILLS_CHECKFUNC]         = "checkfunc";
   glb_datas.txt_col_names.skills_header[SKILLS_NOCOSTINSTATE]     = "nocostinstate";
   glb_datas.txt_col_names.skills_header[SKILLS_USEMANAONDO]       = "usemanaondo";
   glb_datas.txt_col_names.skills_header[SKILLS_STARTMANA]         = "startmana";
   glb_datas.txt_col_names.skills_header[SKILLS_MINMANA]           = "minmana";
   glb_datas.txt_col_names.skills_header[SKILLS_MANASHIFT]         = "manashift";
   glb_datas.txt_col_names.skills_header[SKILLS_MANA]              = "mana";
   glb_datas.txt_col_names.skills_header[SKILLS_LVLMANA]           = "lvlmana";
   glb_datas.txt_col_names.skills_header[SKILLS_INTERRUPT]         = "interrupt";
   glb_datas.txt_col_names.skills_header[SKILLS_INTOWN]            = "InTown";
   glb_datas.txt_col_names.skills_header[SKILLS_AURA]              = "aura";
   glb_datas.txt_col_names.skills_header[SKILLS_PERIODIC]          = "periodic";
   glb_datas.txt_col_names.skills_header[SKILLS_PERDELAY]          = "perdelay";
   glb_datas.txt_col_names.skills_header[SKILLS_FINISHING]         = "finishing";
   glb_datas.txt_col_names.skills_header[SKILLS_PASSIVE]           = "passive";
   glb_datas.txt_col_names.skills_header[SKILLS_PROGRESSIVE]       = "progressive";
   glb_datas.txt_col_names.skills_header[SKILLS_GENERAL]           = "general";
   glb_datas.txt_col_names.skills_header[SKILLS_SCROLL]            = "scroll";
   glb_datas.txt_col_names.skills_header[SKILLS_CALC1]             = "calc1";
   glb_datas.txt_col_names.skills_header[SKILLS_CALC2]             = "calc2";
   glb_datas.txt_col_names.skills_header[SKILLS_CALC3]             = "calc3";
   glb_datas.txt_col_names.skills_header[SKILLS_CALC4]             = "calc4";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM1]            = "Param1";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM2]            = "Param2";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM3]            = "Param3";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM4]            = "Param4";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM5]            = "Param5";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM6]            = "Param6";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM7]            = "Param7";
   glb_datas.txt_col_names.skills_header[SKILLS_PARAM8]            = "Param8";
   glb_datas.txt_col_names.skills_header[SKILLS_INGAME]            = "InGame";
   glb_datas.txt_col_names.skills_header[SKILLS_TOHIT]             = "ToHit";
   glb_datas.txt_col_names.skills_header[SKILLS_LEVTOHIT]          = "LevToHit";
   glb_datas.txt_col_names.skills_header[SKILLS_TOHITCALC]         = "ToHitCalc";
   glb_datas.txt_col_names.skills_header[SKILLS_RESULTFLAGS]       = "ResultFlags";
   glb_datas.txt_col_names.skills_header[SKILLS_HITFLAGS]          = "HitFlags";
   glb_datas.txt_col_names.skills_header[SKILLS_HITCLASS]          = "HitClass";
   glb_datas.txt_col_names.skills_header[SKILLS_KICK]              = "Kick";
   glb_datas.txt_col_names.skills_header[SKILLS_HITSHIFT]          = "HitShift";
   glb_datas.txt_col_names.skills_header[SKILLS_SRCDAM]            = "SrcDam";
   glb_datas.txt_col_names.skills_header[SKILLS_MINDAM]            = "MinDam";
   glb_datas.txt_col_names.skills_header[SKILLS_MINLEVDAM1]        = "MinLevDam1";
   glb_datas.txt_col_names.skills_header[SKILLS_MINLEVDAM2]        = "MinLevDam2";
   glb_datas.txt_col_names.skills_header[SKILLS_MINLEVDAM3]        = "MinLevDam3";
   glb_datas.txt_col_names.skills_header[SKILLS_MINLEVDAM4]        = "MinLevDam4";
   glb_datas.txt_col_names.skills_header[SKILLS_MINLEVDAM5]        = "MinLevDam5";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXDAM]            = "MaxDam";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXLEVDAM1]        = "MaxLevDam1";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXLEVDAM2]        = "MaxLevDam2";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXLEVDAM3]        = "MaxLevDam3";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXLEVDAM4]        = "MaxLevDam4";
   glb_datas.txt_col_names.skills_header[SKILLS_MAXLEVDAM5]        = "MaxLevDam5";
   glb_datas.txt_col_names.skills_header[SKILLS_DMGSYMPERCALC]     = "DmgSymPerCalc";
   glb_datas.txt_col_names.skills_header[SKILLS_ETYPE]             = "EType";
   glb_datas.txt_col_names.skills_header[SKILLS_EMIN]              = "EMin";
   glb_datas.txt_col_names.skills_header[SKILLS_EMINLEV1]          = "EMinLev1";
   glb_datas.txt_col_names.skills_header[SKILLS_EMINLEV2]          = "EMinLev2";
   glb_datas.txt_col_names.skills_header[SKILLS_EMINLEV3]          = "EMinLev3";
   glb_datas.txt_col_names.skills_header[SKILLS_EMINLEV4]          = "EMinLev4";
   glb_datas.txt_col_names.skills_header[SKILLS_EMINLEV5]          = "EMinLev5";
   glb_datas.txt_col_names.skills_header[SKILLS_EMAX]              = "EMax";
   glb_datas.txt_col_names.skills_header[SKILLS_EMAXLEV1]          = "EMaxLev1";
   glb_datas.txt_col_names.skills_header[SKILLS_EMAXLEV2]          = "EMaxLev2";
   glb_datas.txt_col_names.skills_header[SKILLS_EMAXLEV3]          = "EMaxLev3";
   glb_datas.txt_col_names.skills_header[SKILLS_EMAXLEV4]          = "EMaxLev4";
   glb_datas.txt_col_names.skills_header[SKILLS_EMAXLEV5]          = "EMaxLev5";
   glb_datas.txt_col_names.skills_header[SKILLS_EDMGSYMPERCALC]    = "EDmgSymPerCalc";
   glb_datas.txt_col_names.skills_header[SKILLS_ELEN]              = "ELen";
   glb_datas.txt_col_names.skills_header[SKILLS_ELEVLEN1]          = "ELevLen1";
   glb_datas.txt_col_names.skills_header[SKILLS_ELEVLEN2]          = "ELevLen2";
   glb_datas.txt_col_names.skills_header[SKILLS_ELEVLEN3]          = "ELevLen3";
   glb_datas.txt_col_names.skills_header[SKILLS_ELENSYMPERCALC]    = "ELenSymPerCalc";
   glb_datas.txt_col_names.skills_header[SKILLS_AITYPE]            = "aitype";
   glb_datas.txt_col_names.skills_header[SKILLS_AIBONUS]           = "aibonus";
   glb_datas.txt_col_names.skills_header[SKILLS_COSTMULT]          = "cost mult";
   glb_datas.txt_col_names.skills_header[SKILLS_COSTADD]           = "cost add";

   // SoundEnviron.txt
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_HANDLE]          = "Handle";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_INDEX]           = "Index";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_SONG]            = "Song";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_DAYAMBIENCE]     = "Day Ambience";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_NIGHTAMBIENCE]   = "Night Ambience";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_DAYEVENT]        = "Day Event";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_NIGHTEVENT]      = "Night Event";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EVENTDELAY]      = "Event Delay";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_INDOORS]         = "Indoors";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_MATERIAL1]       = "Material 1";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_MATERIAL2]       = "Material 2";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXENVIRON]      = "EAX Environ";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXENVSIZE]      = "EAX Env Size";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXENVDIFF]      = "EAX Env Diff";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXROOMVOL]      = "EAX Room Vol";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXROOMHF]       = "EAX Room HF";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXDECAYTIME]    = "EAX Decay Time";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXDECAYHF]      = "EAX Decay HF";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXREFLECT]      = "EAX Reflect";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXREFLECTDELAY] = "EAX Reflect Delay";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXREVERB]       = "EAX Reverb";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXREVDELAY]     = "EAX Rev Delay";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXROOMROLL]     = "EAX Room Roll";
   glb_datas.txt_col_names.soundenviron_header[SOUNDENVIRON_EAXAIRABSORB]    = "EAX Air Absorb";

   // Sounds.txt
   glb_datas.txt_col_names.sounds_header[SOUNDS_SOUND]     = "Sound";
   glb_datas.txt_col_names.sounds_header[SOUNDS_INDEX]     = "Index";
   glb_datas.txt_col_names.sounds_header[SOUNDS_FILENAME]  = "FileName";
   glb_datas.txt_col_names.sounds_header[SOUNDS_VOLUME]    = "Volume";
   glb_datas.txt_col_names.sounds_header[SOUNDS_GROUPSIZE] = "Group Size";
   glb_datas.txt_col_names.sounds_header[SOUNDS_LOOP]      = "Loop";
   glb_datas.txt_col_names.sounds_header[SOUNDS_FADEIN]    = "Fade In";
   glb_datas.txt_col_names.sounds_header[SOUNDS_FADEOUT]   = "Fade Out";
   glb_datas.txt_col_names.sounds_header[SOUNDS_DEFERINST] = "Defer Inst";
   glb_datas.txt_col_names.sounds_header[SOUNDS_STOPINST]  = "Stop Inst";
   glb_datas.txt_col_names.sounds_header[SOUNDS_DURATION]  = "Duration";
   glb_datas.txt_col_names.sounds_header[SOUNDS_COMPOUND]  = "Compound";
   glb_datas.txt_col_names.sounds_header[SOUNDS_REVERB]    = "Reverb";
   glb_datas.txt_col_names.sounds_header[SOUNDS_FALLOFF]   = "Falloff";
   glb_datas.txt_col_names.sounds_header[SOUNDS_CACHE]     = "Cache";
   glb_datas.txt_col_names.sounds_header[SOUNDS_ASYNCONLY] = "Async Only";
   glb_datas.txt_col_names.sounds_header[SOUNDS_PRIORITY]  = "Priority";
   glb_datas.txt_col_names.sounds_header[SOUNDS_STREAM]    = "Stream";
   glb_datas.txt_col_names.sounds_header[SOUNDS_STEREO]    = "Stereo";
   glb_datas.txt_col_names.sounds_header[SOUNDS_TRACKING]  = "Tracking";
   glb_datas.txt_col_names.sounds_header[SOUNDS_SOLO]      = "Solo";
   glb_datas.txt_col_names.sounds_header[SOUNDS_MUSICVOL]  = "Music Vol";
   glb_datas.txt_col_names.sounds_header[SOUNDS_BLOCK1]    = "Block 1";
   glb_datas.txt_col_names.sounds_header[SOUNDS_BLOCK2]    = "Block 2";
   glb_datas.txt_col_names.sounds_header[SOUNDS_BLOCK3]    = "Block 3";

   // States.txt
   glb_datas.txt_col_names.states_header[STATES_STATE]         = "state";
   glb_datas.txt_col_names.states_header[STATES_ID]            = "id";
   glb_datas.txt_col_names.states_header[STATES_GROUP]         = "group";
   glb_datas.txt_col_names.states_header[STATES_REMHIT]        = "remhit";
   glb_datas.txt_col_names.states_header[STATES_NOSEND]        = "nosend";
   glb_datas.txt_col_names.states_header[STATES_TRANSFORM]     = "transform";
   glb_datas.txt_col_names.states_header[STATES_AURA]          = "aura";
   glb_datas.txt_col_names.states_header[STATES_CURABLE]       = "curable";
   glb_datas.txt_col_names.states_header[STATES_CURSE]         = "curse";
   glb_datas.txt_col_names.states_header[STATES_ACTIVE]        = "active";
   glb_datas.txt_col_names.states_header[STATES_IMMED]         = "immed";
   glb_datas.txt_col_names.states_header[STATES_RESTRICT]      = "restrict";
   glb_datas.txt_col_names.states_header[STATES_DISGUISE]      = "disguise";
   glb_datas.txt_col_names.states_header[STATES_BLUE]          = "blue";
   glb_datas.txt_col_names.states_header[STATES_ATTBLUE]       = "attblue";
   glb_datas.txt_col_names.states_header[STATES_DAMBLUE]       = "damblue";
   glb_datas.txt_col_names.states_header[STATES_ARMBLUE]       = "armblue";
   glb_datas.txt_col_names.states_header[STATES_RFBLUE]        = "rfblue";
   glb_datas.txt_col_names.states_header[STATES_RLBLUE]        = "rlblue";
   glb_datas.txt_col_names.states_header[STATES_RCBLUE]        = "rcblue";
   glb_datas.txt_col_names.states_header[STATES_STAMBARBLUE]   = "stambarblue";
   glb_datas.txt_col_names.states_header[STATES_RPBLUE]        = "rpblue";
   glb_datas.txt_col_names.states_header[STATES_ATTRED]        = "attred";
   glb_datas.txt_col_names.states_header[STATES_DAMRED]        = "damred";
   glb_datas.txt_col_names.states_header[STATES_ARMRED]        = "armred";
   glb_datas.txt_col_names.states_header[STATES_RFRED]         = "rfred";
   glb_datas.txt_col_names.states_header[STATES_RLRED]         = "rlred";
   glb_datas.txt_col_names.states_header[STATES_RCRED]         = "rcred";
   glb_datas.txt_col_names.states_header[STATES_RPRED]         = "rpred";
   glb_datas.txt_col_names.states_header[STATES_EXP]           = "exp";
   glb_datas.txt_col_names.states_header[STATES_PLRSTAYDEATH]  = "plrstaydeath";
   glb_datas.txt_col_names.states_header[STATES_MONSTAYDEATH]  = "monstaydeath";
   glb_datas.txt_col_names.states_header[STATES_BOSSSTAYDEATH] = "bossstaydeath";
   glb_datas.txt_col_names.states_header[STATES_HIDE]          = "hide";
   glb_datas.txt_col_names.states_header[STATES_SHATTER]       = "shatter";
   glb_datas.txt_col_names.states_header[STATES_UDEAD]         = "udead";
   glb_datas.txt_col_names.states_header[STATES_LIFE]          = "life";
   glb_datas.txt_col_names.states_header[STATES_GREEN]         = "green";
   glb_datas.txt_col_names.states_header[STATES_PGSV]          = "pgsv";
   glb_datas.txt_col_names.states_header[STATES_NOOVERLAYS]    = "nooverlays";
   glb_datas.txt_col_names.states_header[STATES_NOCLEAR]       = "noclear";
   glb_datas.txt_col_names.states_header[STATES_BOSSINV]       = "bossinv";
   glb_datas.txt_col_names.states_header[STATES_MELEEONLY]     = "meleeonly";
   glb_datas.txt_col_names.states_header[STATES_NOTONDEAD]     = "notondead";
   glb_datas.txt_col_names.states_header[STATES_OVERLAY1]      = "overlay1";
   glb_datas.txt_col_names.states_header[STATES_OVERLAY2]      = "overlay2";
   glb_datas.txt_col_names.states_header[STATES_OVERLAY3]      = "overlay3";
   glb_datas.txt_col_names.states_header[STATES_OVERLAY4]      = "overlay4";
   glb_datas.txt_col_names.states_header[STATES_PGSVOVERLAY]   = "pgsvoverlay";
   glb_datas.txt_col_names.states_header[STATES_CASTOVERLAY]   = "castoverlay";
   glb_datas.txt_col_names.states_header[STATES_REMOVERLAY]    = "removerlay";
   glb_datas.txt_col_names.states_header[STATES_STAT]          = "stat";
   glb_datas.txt_col_names.states_header[STATES_SETFUNC]       = "setfunc";
   glb_datas.txt_col_names.states_header[STATES_REMFUNC]       = "remfunc";
   glb_datas.txt_col_names.states_header[STATES_MISSILE]       = "missile";
   glb_datas.txt_col_names.states_header[STATES_SKILL]         = "skill";
   glb_datas.txt_col_names.states_header[STATES_ITEMTYPE]      = "itemtype";
   glb_datas.txt_col_names.states_header[STATES_ITEMTRANS]     = "itemtrans";
   glb_datas.txt_col_names.states_header[STATES_COLORPRI]      = "colorpri";
   glb_datas.txt_col_names.states_header[STATES_COLORSHIFT]    = "colorshift";
   glb_datas.txt_col_names.states_header[STATES_LIGHTR]        = "light-r";
   glb_datas.txt_col_names.states_header[STATES_LIGHTG]        = "light-g";
   glb_datas.txt_col_names.states_header[STATES_LIGHTB]        = "light-b";
   glb_datas.txt_col_names.states_header[STATES_ONSOUND]       = "onsound";
   glb_datas.txt_col_names.states_header[STATES_OFFSOUND]      = "offsound";
   glb_datas.txt_col_names.states_header[STATES_GFXTYPE]       = "gfxtype";
   glb_datas.txt_col_names.states_header[STATES_GFXCLASS]      = "gfxclass";
   glb_datas.txt_col_names.states_header[STATES_CLTEVENT]      = "cltevent";
   glb_datas.txt_col_names.states_header[STATES_CLTEVENTFUNC]  = "clteventfunc";
   glb_datas.txt_col_names.states_header[STATES_CLTACTIVEFUNC] = "cltactivefunc";
   glb_datas.txt_col_names.states_header[STATES_SRVACTIVEFUNC] = "srvactivefunc";

   // StorePage.txt
   glb_datas.txt_col_names.storepage_header[STOREPAGE_STOREPAGE] = "Store Page";
   glb_datas.txt_col_names.storepage_header[STOREPAGE_CODE]      = "Code";

   // SuperUniques.txt
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_SUPERUNIQUE] = "Superunique";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_NAME]        = "Name";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_CLASS]       = "Class";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_HCIDX]       = "hcIdx";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_MONSOUND]    = "MonSound";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_MOD1]        = "Mod1";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_MOD2]        = "Mod2";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_MOD3]        = "Mod3";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_MINGRP]      = "MinGrp";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_MAXGRP]      = "MaxGrp";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_ECLASS]      = "EClass";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_AUTOPOS]     = "AutoPos";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_STACKS]      = "Stacks";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_REPLACEABLE] = "Replaceable";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_UTRANS]      = "Utrans";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_UTRANSN]     = "Utrans(N)";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_UTRANSH]     = "Utrans(H)";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_TC]          = "TC";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_TCN]         = "TC(N)";
   glb_datas.txt_col_names.superuniques_header[SUPERUNIQUES_TCH]         = "TC(H)";

   // TreasureClassEx.txt
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_TREASURECLASS] = "Treasure Class";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_GROUP]         = "group";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_LEVEL]         = "level";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PICKS]         = "Picks";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_UNIQUE]        = "Unique";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_SET]           = "Set";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_RARE]          = "Rare";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_MAGIC]         = "Magic";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_NODROP]        = "NoDrop";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM1]         = "Item1";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB1]         = "Prob1";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM2]         = "Item2";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB2]         = "Prob2";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM3]         = "Item3";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB3]         = "Prob3";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM4]         = "Item4";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB4]         = "Prob4";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM5]         = "Item5";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB5]         = "Prob5";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM6]         = "Item6";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB6]         = "Prob6";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM7]         = "Item7";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB7]         = "Prob7";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM8]         = "Item8";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB8]         = "Prob8";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM9]         = "Item9";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB9]         = "Prob9";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_ITEM10]        = "Item10";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_PROB10]        = "Prob10";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_SUMITEMS]      = "SumItems";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_TOTALPROB]     = "TotalProb";
   glb_datas.txt_col_names.treasureclassex_header[TREASURECLASSEX_DROPCHANCE]    = "DropChance";

   // UniqueAppellation.txt
   glb_datas.txt_col_names.uniqueappellation_header[UNIQUEAPPELLATION_NAME] = "Name";

   // UniqueItems.txt
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_INDEX]        = "index";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_VERSION]      = "version";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_ENABLED]      = "enabled";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_LADDER]       = "ladder";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_RARITY]       = "rarity";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_NOLIMIT]      = "nolimit";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_LVL]          = "lvl";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_LVLREQ]       = "lvl req";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_CODE]         = "code";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_CARRY1]       = "carry1";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_COSTMULT]     = "cost mult";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_COSTADD]      = "cost add";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_CHRTRANSFORM] = "chrtransform";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_INVTRANSFORM] = "invtransform";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_FLIPPYFILE]   = "flippyfile";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_INVFILE]      = "invfile";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_DROPSOUND]    = "dropsound";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_DROPSFXFRAME] = "dropsfxframe";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_USESOUND]     = "usesound";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP1]        = "prop1";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR1]         = "par1";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN1]         = "min1";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX1]         = "max1";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP2]        = "prop2";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR2]         = "par2";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN2]         = "min2";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX2]         = "max2";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP3]        = "prop3";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR3]         = "par3";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN3]         = "min3";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX3]         = "max3";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP4]        = "prop4";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR4]         = "par4";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN4]         = "min4";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX4]         = "max4";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP5]        = "prop5";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR5]         = "par5";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN5]         = "min5";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX5]         = "max5";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP6]        = "prop6";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR6]         = "par6";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN6]         = "min6";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX6]         = "max6";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP7]        = "prop7";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR7]         = "par7";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN7]         = "min7";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX7]         = "max7";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP8]        = "prop8";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR8]         = "par8";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN8]         = "min8";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX8]         = "max8";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP9]        = "prop9";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR9]         = "par9";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN9]         = "min9";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX9]         = "max9";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP10]       = "prop10";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR10]        = "par10";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN10]        = "min10";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX10]        = "max10";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP11]       = "prop11";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR11]        = "par11";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN11]        = "min11";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX11]        = "max11";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PROP12]       = "prop12";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_PAR12]        = "par12";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MIN12]        = "min12";
   glb_datas.txt_col_names.uniqueitems_header[UNIQUEITEMS_MAX12]        = "max12";

   // UniquePrefix.txt
   glb_datas.txt_col_names.uniqueprefix_header[UNIQUEPREFIX_NAME] = "Name";

   // UniqueSuffix.txt
   glb_datas.txt_col_names.uniquesuffix_header[UNIQUESUFFIX_NAME] = "Name";

   // UniqueTitle.txt
   glb_datas.txt_col_names.uniquetitle_header[UNIQUETITLE_NAME]  = "Name";
   glb_datas.txt_col_names.uniquetitle_header[UNIQUETITLE_NAMCO] = "Namco";

   // WeaponClass.txt
   glb_datas.txt_col_names.weaponclass_header[WEAPONCLASS_WEAPONCLASS] = "Weapon Class";
   glb_datas.txt_col_names.weaponclass_header[WEAPONCLASS_CODE]        = "Code";

}


// ========================================================================================
// make pointers on all values of all (used) columns of 1 .txt file
// input :
//    * txt ID
// output :
//    * 0 if ok, non-zero if error
// ========================================================================================
int txt_init_values_pointers(TXT_ENUM txt_idx)
{
   TXT_S * ptr_txt;
   int   i, size, idx, x, y, length, c;
   char  * ptr;


   if ((txt_idx < 0) || (txt_idx >= TXT_MAX))
      return -1;
   ptr_txt = & glb_datas.txt[txt_idx];
   if (ptr_txt->buffer == NULL)
      return -1;

   // which txt is it ?
   switch (txt_idx)
   {
      case ARMOR:
      case WEAPONS:
      case MISC:
         ptr_txt->header    = glb_datas.txt_col_names.awm_header;
         ptr_txt->nb_header = AWM_COL_MAX;
         break;

      case ARMTYPE:
         ptr_txt->header    = glb_datas.txt_col_names.armtype_header;
         ptr_txt->nb_header = ARMTYPE_COL_MAX;
         break;

      case AUTOMAGIC:
         ptr_txt->header    = glb_datas.txt_col_names.automagic_header;
         ptr_txt->nb_header = AUTOMAGIC_COL_MAX;
         break;

      case AUTOMAP:
         ptr_txt->header    = glb_datas.txt_col_names.automap_header;
         ptr_txt->nb_header = AUTOMAP_COL_MAX;
         break;

      case BELTS:
         ptr_txt->header    = glb_datas.txt_col_names.belts_header;
         ptr_txt->nb_header = BELTS_COL_MAX;
         break;

      case BODYLOCS:
         ptr_txt->header    = glb_datas.txt_col_names.bodylocs_header;
         ptr_txt->nb_header = BODYLOCS_COL_MAX;
         break;

      case BOOKS:
         ptr_txt->header    = glb_datas.txt_col_names.books_header;
         ptr_txt->nb_header = BOOKS_COL_MAX;
         break;

      case CHARSTATS:
         ptr_txt->header    = glb_datas.txt_col_names.charstats_header;
         ptr_txt->nb_header = CHARSTATS_COL_MAX;
         break;

      case COLORS:
         ptr_txt->header    = glb_datas.txt_col_names.colors_header;
         ptr_txt->nb_header = COLORS_COL_MAX;
         break;

      case COMPCODE:
         ptr_txt->header    = glb_datas.txt_col_names.compcode_header;
         ptr_txt->nb_header = COMPCODE_COL_MAX;
         break;

      case COMPOSIT:
         ptr_txt->header    = glb_datas.txt_col_names.composit_header;
         ptr_txt->nb_header = COMPOSIT_COL_MAX;
         break;

      case CUBEMAIN:
         ptr_txt->header    = glb_datas.txt_col_names.cubemain_header;
         ptr_txt->nb_header = CUBEMAIN_COL_MAX;
         break;

      case CUBEMOD:
         ptr_txt->header    = glb_datas.txt_col_names.cubemod_header;
         ptr_txt->nb_header = CUBEMOD_COL_MAX;
         break;

      case CUBETYPE:
         ptr_txt->header    = glb_datas.txt_col_names.cubetype_header;
         ptr_txt->nb_header = CUBETYPE_COL_MAX;
         break;

      case DIFFICULTYLEVELS:
         ptr_txt->header    = glb_datas.txt_col_names.difficultylevels_header;
         ptr_txt->nb_header = DIFFICULTYLEVELS_COL_MAX;
         break;

      case ELEMTYPES:
         ptr_txt->header    = glb_datas.txt_col_names.elemtypes_header;
         ptr_txt->nb_header = ELEMTYPES_COL_MAX;
         break;

      case EVENTS:
         ptr_txt->header    = glb_datas.txt_col_names.events_header;
         ptr_txt->nb_header = EVENTS_COL_MAX;
         break;

      case EXPERIENCE:
         ptr_txt->header    = glb_datas.txt_col_names.experience_header;
         ptr_txt->nb_header = EXPERIENCE_COL_MAX;
         break;

      case GAMBLE:
         ptr_txt->header    = glb_datas.txt_col_names.gamble_header;
         ptr_txt->nb_header = GAMBLE_COL_MAX;
         break;

      case GEMS:
         ptr_txt->header    = glb_datas.txt_col_names.gems_header;
         ptr_txt->nb_header = GEMS_COL_MAX;
         break;

      case HIREDESC:
         ptr_txt->header    = glb_datas.txt_col_names.hiredesc_header;
         ptr_txt->nb_header = HIREDESC_COL_MAX;
         break;

      case HIRELING:
         ptr_txt->header    = glb_datas.txt_col_names.hireling_header;
         ptr_txt->nb_header = HIRELING_COL_MAX;
         break;

      case HITCLASS:
         ptr_txt->header    = glb_datas.txt_col_names.hitclass_header;
         ptr_txt->nb_header = HITCLASS_COL_MAX;
         break;

      case INVENTORY:
         ptr_txt->header    = glb_datas.txt_col_names.inventory_header;
         ptr_txt->nb_header = INVENTORY_COL_MAX;
         break;

      case ITEMRATIO:
         ptr_txt->header    = glb_datas.txt_col_names.itemratio_header;
         ptr_txt->nb_header = ITEMRATIO_COL_MAX;
         break;

      case ITEMSTATCOST:
         ptr_txt->header    = glb_datas.txt_col_names.itemstatcost_header;
         ptr_txt->nb_header = ITEMSTATCOST_COL_MAX;
         break;

      case ITEMTYPES:
         ptr_txt->header    = glb_datas.txt_col_names.itemtypes_header;
         ptr_txt->nb_header = ITEMTYPES_COL_MAX;
         break;

      case LEVELS:
         ptr_txt->header    = glb_datas.txt_col_names.levels_header;
         ptr_txt->nb_header = LEVELS_COL_MAX;
         break;

      case LOWQUALITYITEMS:
         ptr_txt->header    = glb_datas.txt_col_names.lowqualityitems_header;
         ptr_txt->nb_header = LOWQUALITYITEMS_COL_MAX;
         break;

      case LVLMAZE:
         ptr_txt->header    = glb_datas.txt_col_names.lvlmaze_header;
         ptr_txt->nb_header = LVLMAZE_COL_MAX;
         break;

      case LVLPREST:
         ptr_txt->header    = glb_datas.txt_col_names.lvlprest_header;
         ptr_txt->nb_header = LVLPREST_COL_MAX;
         break;

      case LVLSUB:
         ptr_txt->header    = glb_datas.txt_col_names.lvlsub_header;
         ptr_txt->nb_header = LVLSUB_COL_MAX;
         break;

      case LVLTYPES:
         ptr_txt->header    = glb_datas.txt_col_names.lvltypes_header;
         ptr_txt->nb_header = LVLTYPES_COL_MAX;
         break;

      case LVLWARP:
         ptr_txt->header    = glb_datas.txt_col_names.lvlwarp_header;
         ptr_txt->nb_header = LVLWARP_COL_MAX;
         break;

      case MAGICPREFIX:
      case MAGICSUFFIX:
         ptr_txt->header    = glb_datas.txt_col_names.magicaffix_header;
         ptr_txt->nb_header = MAGICAFFIX_COL_MAX;
         break;

      case MISSCALC:
         ptr_txt->header    = glb_datas.txt_col_names.misscalc_header;
         ptr_txt->nb_header = MISSCALC_COL_MAX;
         break;

      case MISSILES:
         ptr_txt->header    = glb_datas.txt_col_names.missiles_header;
         ptr_txt->nb_header = MISSILES_COL_MAX;
         break;

      case MONAI:
         ptr_txt->header    = glb_datas.txt_col_names.monai_header;
         ptr_txt->nb_header = MONAI_COL_MAX;
         break;

      case MONEQUIP:
         ptr_txt->header    = glb_datas.txt_col_names.monequip_header;
         ptr_txt->nb_header = MONEQUIP_COL_MAX;
         break;

      case MONLVL:
         ptr_txt->header    = glb_datas.txt_col_names.monlvl_header;
         ptr_txt->nb_header = MONLVL_COL_MAX;
         break;

      case MONMODE:
         ptr_txt->header    = glb_datas.txt_col_names.monmode_header;
         ptr_txt->nb_header = MONMODE_COL_MAX;
         break;

      case MONNAME:
         ptr_txt->header    = glb_datas.txt_col_names.monname_header;
         ptr_txt->nb_header = MONNAME_COL_MAX;
         break;

      case MONPLACE:
         ptr_txt->header    = glb_datas.txt_col_names.monplace_header;
         ptr_txt->nb_header = MONPLACE_COL_MAX;
         break;

      case MONPRESET:
         ptr_txt->header    = glb_datas.txt_col_names.monpreset_header;
         ptr_txt->nb_header = MONPRESET_COL_MAX;
         break;

      case MONSEQ:
         ptr_txt->header    = glb_datas.txt_col_names.monseq_header;
         ptr_txt->nb_header = MONSEQ_COL_MAX;
         break;

      case MONPROP:
         ptr_txt->header    = glb_datas.txt_col_names.monprop_header;
         ptr_txt->nb_header = MONPROP_COL_MAX;
         break;

      case MONSOUNDS:
         ptr_txt->header    = glb_datas.txt_col_names.monsounds_header;
         ptr_txt->nb_header = MONSOUNDS_COL_MAX;
         break;

      case MONSTATS:
         ptr_txt->header    = glb_datas.txt_col_names.monstats_header;
         ptr_txt->nb_header = MONSTATS_COL_MAX;
         break;

      case MONSTATS2:
         ptr_txt->header    = glb_datas.txt_col_names.monstats2_header;
         ptr_txt->nb_header = MONSTATS2_COL_MAX;
         break;

      case MONTYPE:
         ptr_txt->header    = glb_datas.txt_col_names.montype_header;
         ptr_txt->nb_header = MONTYPE_COL_MAX;
         break;

      case MONUMOD:
         ptr_txt->header    = glb_datas.txt_col_names.monumod_header;
         ptr_txt->nb_header = MONUMOD_COL_MAX;
         break;

      case NPC:
         ptr_txt->header    = glb_datas.txt_col_names.npc_header;
         ptr_txt->nb_header = NPC_COL_MAX;
         break;

      case OBJECTS:
         ptr_txt->header    = glb_datas.txt_col_names.objects_header;
         ptr_txt->nb_header = OBJECTS_COL_MAX;
         break;

      case OBJGROUP:
         ptr_txt->header    = glb_datas.txt_col_names.objgroup_header;
         ptr_txt->nb_header = OBJGROUP_COL_MAX;
         break;

      case OBJMODE:
         ptr_txt->header    = glb_datas.txt_col_names.objmode_header;
         ptr_txt->nb_header = OBJMODE_COL_MAX;
         break;

      case OBJTYPE:
         ptr_txt->header    = glb_datas.txt_col_names.objtype_header;
         ptr_txt->nb_header = OBJTYPE_COL_MAX;
         break;

      case OVERLAY:
         ptr_txt->header    = glb_datas.txt_col_names.overlay_header;
         ptr_txt->nb_header = OVERLAY_COL_MAX;
         break;

      case PETTYPE:
         ptr_txt->header    = glb_datas.txt_col_names.pettype_header;
         ptr_txt->nb_header = PETTYPE_COL_MAX;
         break;

      case PLAYERCLASS:
         ptr_txt->header    = glb_datas.txt_col_names.playerclass_header;
         ptr_txt->nb_header = PLAYERCLASS_COL_MAX;
         break;

      case PLRMODE:
         ptr_txt->header    = glb_datas.txt_col_names.plrmode_header;
         ptr_txt->nb_header = PLRMODE_COL_MAX;
         break;

      case PLRTYPE:
         ptr_txt->header    = glb_datas.txt_col_names.plrtype_header;
         ptr_txt->nb_header = PLRTYPE_COL_MAX;
         break;

      case PROPERTIES:
         ptr_txt->header    = glb_datas.txt_col_names.properties_header;
         ptr_txt->nb_header = PROPERTIES_COL_MAX;
         break;

      case QUALITYITEMS:
         ptr_txt->header    = glb_datas.txt_col_names.qualityitems_header;
         ptr_txt->nb_header = QUALITYITEMS_COL_MAX;
         break;

      case RAREPREFIX:
      case RARESUFFIX:
         ptr_txt->header    = glb_datas.txt_col_names.rareaffix_header;
         ptr_txt->nb_header = RAREAFFIX_COL_MAX;
         break;

      case RUNES:
         ptr_txt->header    = glb_datas.txt_col_names.runes_header;
         ptr_txt->nb_header = RUNES_COL_MAX;
         break;

      case SETITEMS:
         ptr_txt->header    = glb_datas.txt_col_names.setitems_header;
         ptr_txt->nb_header = SETITEMS_COL_MAX;
         break;

      case SETS:
         ptr_txt->header    = glb_datas.txt_col_names.sets_header;
         ptr_txt->nb_header = SETS_COL_MAX;
         break;

      case SHRINES:
         ptr_txt->header    = glb_datas.txt_col_names.shrines_header;
         ptr_txt->nb_header = SHRINES_COL_MAX;
         break;

      case SKILLCALC:
         ptr_txt->header    = glb_datas.txt_col_names.skillcalc_header;
         ptr_txt->nb_header = SKILLCALC_COL_MAX;
         break;

      case SKILLDESC:
         ptr_txt->header    = glb_datas.txt_col_names.skilldesc_header;
         ptr_txt->nb_header = SKILLDESC_COL_MAX;
         break;

      case SKILLS:
         ptr_txt->header    = glb_datas.txt_col_names.skills_header;
         ptr_txt->nb_header = SKILLS_COL_MAX;
         break;

      case SOUNDENVIRON:
         ptr_txt->header    = glb_datas.txt_col_names.soundenviron_header;
         ptr_txt->nb_header = SOUNDENVIRON_COL_MAX;
         break;

      case SOUNDS:
         ptr_txt->header    = glb_datas.txt_col_names.sounds_header;
         ptr_txt->nb_header = SOUNDS_COL_MAX;
         break;

      case STATES:
         ptr_txt->header    = glb_datas.txt_col_names.states_header;
         ptr_txt->nb_header = STATES_COL_MAX;
         break;

      case STOREPAGE:
         ptr_txt->header    = glb_datas.txt_col_names.storepage_header;
         ptr_txt->nb_header = STOREPAGE_COL_MAX;
         break;

      case SUPERUNIQUES:
         ptr_txt->header    = glb_datas.txt_col_names.superuniques_header;
         ptr_txt->nb_header = SUPERUNIQUES_COL_MAX;
         break;

      case TREASURECLASSEX:
         ptr_txt->header    = glb_datas.txt_col_names.treasureclassex_header;
         ptr_txt->nb_header = TREASURECLASSEX_COL_MAX;
         break;

      case UNIQUEAPPELLATION:
         ptr_txt->header    = glb_datas.txt_col_names.uniqueappellation_header;
         ptr_txt->nb_header = UNIQUEAPPELLATION_COL_MAX;
         break;

      case UNIQUEITEMS:
         ptr_txt->header    = glb_datas.txt_col_names.uniqueitems_header;
         ptr_txt->nb_header = UNIQUEITEMS_COL_MAX;
         break;

      case UNIQUEPREFIX:
         ptr_txt->header    = glb_datas.txt_col_names.uniqueprefix_header;
         ptr_txt->nb_header = UNIQUEPREFIX_COL_MAX;
         break;

      case UNIQUESUFFIX:
         ptr_txt->header    = glb_datas.txt_col_names.uniquesuffix_header;
         ptr_txt->nb_header = UNIQUESUFFIX_COL_MAX;
         break;

      case UNIQUETITLE:
         ptr_txt->header    = glb_datas.txt_col_names.uniquetitle_header;
         ptr_txt->nb_header = UNIQUETITLE_COL_MAX;
         break;

      case WEAPONCLASS:
         ptr_txt->header    = glb_datas.txt_col_names.weaponclass_header;
         ptr_txt->nb_header = WEAPONCLASS_COL_MAX;
         break;

      default :
         ptr_txt->header    = NULL;
         ptr_txt->nb_header = 0;
         break;
   }

   // malloc of columns for this txt
   size = sizeof(char ***) * ptr_txt->nb_header;
   ptr_txt->col = (char ***) malloc(size);
   if (ptr_txt->col == NULL)
   {
      sprintf(glb_datas.error_strtmp, "txt_init_values_pointers() : ptr_txt->col == NULL");
      d2txtanalyser_error(glb_datas.error_strtmp);
   }
   memset(ptr_txt->col, 0, size);

   // malloc of columns values
   for (i=0; i < ptr_txt->nb_header; i++)
   {
      size = sizeof(char **) * ptr_txt->nb_rows;
      ptr_txt->col[i] = (char **) malloc(size);
      if (ptr_txt->col[i] == NULL)
      {
         sprintf(glb_datas.error_strtmp, "txt_init_values_pointers() : ptr_txt->col[%i] == NULL", i);
         d2txtanalyser_error(glb_datas.error_strtmp);
      }
      memset(ptr_txt->col[i], 0, size);
   }

   // find the pointers on all columns values

   // for all columns of this txt
   for (i=0; i < ptr_txt->nb_header; i++)
   {
      // which column position is this column ?
      idx = txt_find_column_position(ptr_txt, i);
      if (idx == -1)
      {
         // error
         printf("   * %3i : %-25s not found\n", i + 1, ptr_txt->header[i]);
         fflush(stdout);
      }
      else
      {
         // found it
         printf("   * %3i : %-25s is at column %i\n", i + 1, ptr_txt->header[i], idx + 1);
         fflush(stdout);
         ptr = (char *) ptr_txt->buffer;
         for (y=0; y < ptr_txt->nb_rows; y++)
         {
            // skip the start of this line
            for (x=0; x < idx; x++)
            {
               length = strlen(ptr);
               ptr += (length + 1);
            }

            // we're on the right column
            ptr_txt->col[i][y] = ptr;

            // skip the rest of this line
            if (y < (ptr_txt->nb_rows - 1))
            {
               c = * ptr;
               while (c != 0x0A)
               {
                  ptr++;
                  c = * ptr;
               }
               ptr++;
            }
         }
      }
   }

   // end
   return 0;
}
