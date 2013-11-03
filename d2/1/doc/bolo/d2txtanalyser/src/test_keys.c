#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "error.h"
#include "test_keys.h"
#include "mpq\mpqtypes.h"
#include "globals.h"


// ========================================================================================
// test the unicity of 1 column in 1 text
// input :
//    * txt ID
//    * column ID
//    * FALSE = check for errors & warnings, TRUE = check for warnings only
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_unicity(TXT_ENUM txt_idx, int col_idx, int only_warning)
{
   TXT_S * txt;
   int   is_ok = TRUE, is_ok_warn = TRUE, y, y2, in_error;
   char  * str1, * str2;

   if ((txt_idx < 0) || (txt_idx >= TXT_MAX))
      return -1;

   txt = & glb_datas.txt[txt_idx];

   if (txt == NULL)
      return -1;

   if ((col_idx < 0) || (col_idx >= txt->nb_header))
      return -1;

   if (txt->col[col_idx] == NULL)
      return -1;


   // for all rows with a value
   for (y=1; y < txt->nb_rows - 1; y++)
   {
      str1 = txt->col[col_idx][y];
      if (str1 == NULL)
         return -1;
      if (strlen(str1) == 0) // blank are skipped
         continue;
      if (_stricmp(str1, "Expansion") == 0)  // "Expansion" are skipped
         continue;

      // is there the same value in the folowing rows ?
      in_error = FALSE;
      if (only_warning == FALSE)
      {
         for (y2 = y+1; y2 < txt->nb_rows; y2++)
         {
            str2 = txt->col[col_idx][y2];
            if (strlen(str2) == 0)
               continue;
            if (strcmp(str1, str2) == 0)
            {
               // display txt filename
               if (is_ok == TRUE)
                  printf("\nIn %s :\n", txt->filename);

               // error
               printf("   * ERROR : '%s' at row %i (%s) is same as in row %i (%s)\n",
                  txt->header[col_idx],
                  y+1,
                  str1,
                  y2+1,
                  str2
               );
               is_ok = FALSE;
               in_error = TRUE;
               break;
            }
         }
      }

      // check for warning (case insensitive)
      if (in_error == FALSE)
      {
         for (y2 = y+1; y2 < txt->nb_rows; y2++)
         {
            str2 = txt->col[col_idx][y2];
            if (strlen(str2) == 0)
               continue;
            if (_stricmp(str1, str2) == 0)
            {
               // display txt filename
               if (is_ok_warn == TRUE)
               {
                  fprintf(glb_datas.std_warning, "\nIn %s :\n", txt->filename);
                  is_ok_warn = FALSE;
               }

               // warning
               fprintf(
                  glb_datas.std_warning,
                  "   * WARNING : '%s' at row %i (%s) is same as in row %i (%s)\n",
                  txt->header[col_idx],
                  y+1,
                  str1,
                  y2+1,
                  str2
               );
               is_ok = FALSE;
               in_error = TRUE;
               break;
            }
         }
      }
   }
   
   // end 
   if ((is_ok == TRUE) && (is_ok_warn == TRUE))
      return 0;
   return -1;
}


// ========================================================================================
// test the unicity of codes in armor weapons & misc.txt
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_unicity_awm(void)
{
   TXT_S * txt[3];
   int   y, y2, is_ok = TRUE, is_ok_warn = TRUE, t, t2, y2_start, in_error,
         txt_id[3]  = {ARMOR, WEAPONS, MISC};
   char  * str1, * str2,
         * txt_name[3] = {"Armor.txt", "Weapons.txt", "Misc.txt"};


   for (t=0; t < 3; t++)
   {
      txt[t] = & glb_datas.txt[ txt_id[t] ];
      if (txt[t]->buffer == NULL)
         return -1;
      if (txt[t]->col[AWM_CODE] == NULL)
         return -1;
      if (txt[t]->col[AWM_NAME] == NULL)
         return -1;
   }

   // for all 3 txt
   for (t=0; t < 3; t++)
   {
      // for all rows with a value within this txt
      for (y=1; y < txt[t]->nb_rows; y++)
      {
         str1 = txt[t]->col[AWM_CODE][y];
         if (strlen(str1) == 0)
            continue;
         if (_stricmp("Expansion", str1) == 0)
            continue;

         // is there the same value in any rows of current and folowing txt ?
         in_error = FALSE;
         for (t2=t; t2 < 3; t2++)
         {
            if (t2 == t)
               y2_start = y + 1;
            else
               y2_start = 1;

            for (y2 = y2_start; y2 < txt[t2]->nb_rows; y2++)
            {
               str2 = txt[t2]->col[AWM_CODE][y2];
               if (strlen(str2) == 0)
                  continue;
               if (_stricmp("Expansion", str2) == 0)
                  continue;
               if (strcmp(str1, str2) == 0)
               {
                  if (is_ok == TRUE)
                     printf("\nIn Armor/Weapons/Misc.txt :\n");

                  printf(
                     "   * ERROR : 'code' (%s) at row %i (%s) in %s is same "
                     "as 'code' (%s) in row %i (%s) of %s\n",
                     str1, y+1,  txt[t ]->col[AWM_NAME][y],  txt_name[t],
                     str2, y2+1, txt[t2]->col[AWM_NAME][y2], txt_name[t2]
                  );
                  is_ok = FALSE;
                  in_error = TRUE;
                  break;
               }
            }
         }

         if (in_error == FALSE)
         {
            // search for warnings
            for (t2=t; t2 < 3; t2++)
            {
               if (t2 == t)
                  y2_start = y + 1;
               else
                  y2_start = 1;

               for (y2 = y2_start; y2 < txt[t2]->nb_rows; y2++)
               {
                  str2 = txt[t2]->col[AWM_CODE][y2];
                  if (strlen(str2) == 0)
                     continue;
                  if (_stricmp("Expansion", str2) == 0)
                     continue;
                  if (_stricmp(str1, str2) == 0)
                  {
                     if (is_ok_warn == TRUE)
                     {
                        fprintf(glb_datas.std_warning, "\nIn Armor/Weapons/Misc.txt :\n");
                        is_ok_warn = FALSE;
                     }

                     fprintf(
                        glb_datas.std_warning,
                        "   * WARNING : 'code' (%s) at row %i (%s) in %s is same (case-insensitive) "
                        "as 'code' (%s) in row %i (%s) of %s\n",
                        str1, y+1,  txt[t ]->col[AWM_NAME][y],  txt_name[t],
                        str2, y2+1, txt[t2]->col[AWM_NAME][y2], txt_name[t2]
                     );
                     is_ok = FALSE;
                     in_error = TRUE;
                     break;
                  }
               }
            }
         }
      }
   }

   // end
   if ((is_ok == TRUE) && (is_ok_warn == TRUE))
      return 0;
   return -1;
}


// ========================================================================================
// test the unicity of some columns in all texts
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_unicity_in_all_txt(void)
{
   int is_ok = TRUE;


   if (test_unicity_awm()) is_ok = FALSE;

   if (test_unicity(AUTOMAGIC,         AUTOMAGIC_NAME,                FALSE)) is_ok = FALSE;
   if (test_unicity(BODYLOCS,          BODYLOCS_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(CHARSTATS,         CHARSTATS_CLASS,               FALSE)) is_ok = FALSE;
   if (test_unicity(COLORS,            COLORS_CODE,                   FALSE)) is_ok = FALSE;
   if (test_unicity(COMPCODE,          COMPCODE_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(COMPOSIT,          COMPOSIT_TOKEN,                FALSE)) is_ok = FALSE;
   if (test_unicity(CUBEMOD,           CUBEMOD_CODE,                  FALSE)) is_ok = FALSE;
   if (test_unicity(CUBETYPE,          CUBETYPE_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(DIFFICULTYLEVELS,  DIFFICULTYLEVELS_NAME,         FALSE)) is_ok = FALSE;
   if (test_unicity(ELEMTYPES,         ELEMTYPES_CODE,                FALSE)) is_ok = FALSE;
   if (test_unicity(EVENTS,            EVENTS_EVENT,                  FALSE)) is_ok = FALSE;
   if (test_unicity(EXPERIENCE,        EXPERIENCE_LEVEL,              FALSE)) is_ok = FALSE;
   if (test_unicity(GAMBLE,            GAMBLE_CODE,                   FALSE)) is_ok = FALSE;
   if (test_unicity(GEMS,              GEMS_CODE,                     FALSE)) is_ok = FALSE;
   if (test_unicity(GEMS,              GEMS_LETTER,                   FALSE)) is_ok = FALSE;
   if (test_unicity(HIREDESC,          HIREDESC_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(HITCLASS,          HITCLASS_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(ITEMTYPES,         ITEMTYPES_CODE,                FALSE)) is_ok = FALSE;
   if (test_unicity(INVENTORY,         INVENTORY_CLASS,               FALSE)) is_ok = FALSE;
   if (test_unicity(ITEMSTATCOST,      ITEMSTATCOST_STAT,             FALSE)) is_ok = FALSE;
   if (test_unicity(ITEMSTATCOST,      ITEMSTATCOST_ID,               FALSE)) is_ok = FALSE;
   if (test_unicity(LEVELS,            LEVELS_ID,                     FALSE)) is_ok = FALSE;
   if (test_unicity(LOWQUALITYITEMS,   LOWQUALITYITEMS_NAME,          FALSE)) is_ok = FALSE;
   if (test_unicity(LVLMAZE,           LVLMAZE_LEVEL,                 FALSE)) is_ok = FALSE;
   if (test_unicity(LVLPREST,          LVLPREST_DEF,                  FALSE)) is_ok = FALSE;
   if (test_unicity(LVLTYPES,          LVLTYPES_ID,                   FALSE)) is_ok = FALSE;
   if (test_unicity(MISSCALC,          MISSCALC_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(MISSILES,          MISSILES_MISSILE,              FALSE)) is_ok = FALSE;
   if (test_unicity(MISSILES,          MISSILES_ID,                   FALSE)) is_ok = FALSE;
   if (test_unicity(MONAI,             MONAI_AI,                      FALSE)) is_ok = FALSE;
   if (test_unicity(MONLVL,            MONLVL_LEVEL,                  FALSE)) is_ok = FALSE;
   if (test_unicity(MONMODE,           MONMODE_CODE,                  FALSE)) is_ok = FALSE;
   if (test_unicity(MONNAME,           MONNAME_MONNAME,               FALSE)) is_ok = FALSE;
   if (test_unicity(MONPLACE,          MONPLACE_CODE,                 FALSE)) is_ok = FALSE;
   if (test_unicity(MONPROP,           MONPROP_ID,                    FALSE)) is_ok = FALSE;
   if (test_unicity(MONSOUNDS,         MONSOUNDS_ID,                  FALSE)) is_ok = FALSE;
   if (test_unicity(MONSTATS,          MONSTATS_ID,                   FALSE)) is_ok = FALSE;
   if (test_unicity(MONSTATS,          MONSTATS_HCIDX,                FALSE)) is_ok = FALSE;
   if (test_unicity(MONSTATS2,         MONSTATS2_ID,                  FALSE)) is_ok = FALSE;
   if (test_unicity(MONTYPE,           MONTYPE_TYPE,                  FALSE)) is_ok = FALSE;
   if (test_unicity(MONUMOD,           MONUMOD_UNIQUEMOD,             FALSE)) is_ok = FALSE;
   if (test_unicity(MONUMOD,           MONUMOD_ID,                    FALSE)) is_ok = FALSE;
   if (test_unicity(NPC,               NPC_NPC,                       FALSE)) is_ok = FALSE;
   if (test_unicity(OBJECTS,           OBJECTS_ID,                    FALSE)) is_ok = FALSE;
   if (test_unicity(OBJGROUP,          OBJGROUP_OFFSET,               FALSE)) is_ok = FALSE;
   if (test_unicity(OBJMODE,           OBJMODE_TOKEN,                 FALSE)) is_ok = FALSE;
   if (test_unicity(OVERLAY,           OVERLAY_OVERLAY,               FALSE)) is_ok = FALSE;
   if (test_unicity(PETTYPE,           PETTYPE_IDX,                   FALSE)) is_ok = FALSE;
   if (test_unicity(PLAYERCLASS,       PLAYERCLASS_CODE,              FALSE)) is_ok = FALSE;
   if (test_unicity(PLRMODE,           PLRMODE_CODE,                  FALSE)) is_ok = FALSE;
   if (test_unicity(PLRTYPE,           PLRTYPE_TOKEN,                 FALSE)) is_ok = FALSE;
   if (test_unicity(PROPERTIES,        PROPERTIES_CODE,               FALSE)) is_ok = FALSE;
   if (test_unicity(RUNES,             RUNES_RUNENAME,                FALSE)) is_ok = FALSE;
   if (test_unicity(SETITEMS,          SETITEMS_INDEX,                FALSE)) is_ok = FALSE;
   if (test_unicity(SETS,              SETS_INDEX,                    FALSE)) is_ok = FALSE;
   if (test_unicity(SETS,              SETS_NAME,                     TRUE))  is_ok = FALSE;
   if (test_unicity(SHRINES,           SHRINES_CODE,                  FALSE)) is_ok = FALSE;
   if (test_unicity(SHRINES,           SHRINES_SHRINENAME,            TRUE))  is_ok = FALSE;
   if (test_unicity(SKILLS,            SKILLS_SKILL,                  FALSE)) is_ok = FALSE;
   if (test_unicity(SKILLS,            SKILLS_ID,                     FALSE)) is_ok = FALSE;
   if (test_unicity(SKILLCALC,         SKILLCALC_CODE,                FALSE)) is_ok = FALSE;
   if (test_unicity(SKILLDESC,         SKILLDESC_SKILLDESC,           FALSE)) is_ok = FALSE;
   if (test_unicity(SOUNDENVIRON,      SOUNDENVIRON_HANDLE,           FALSE)) is_ok = FALSE;
   if (test_unicity(SOUNDENVIRON,      SOUNDENVIRON_INDEX,            FALSE)) is_ok = FALSE;
   if (test_unicity(SOUNDS,            SOUNDS_SOUND,                  FALSE)) is_ok = FALSE;
   if (test_unicity(SOUNDS,            SOUNDS_INDEX,                  FALSE)) is_ok = FALSE;
   if (test_unicity(STATES,            STATES_STATE,                  FALSE)) is_ok = FALSE;
   if (test_unicity(STATES,            STATES_ID ,                    FALSE)) is_ok = FALSE;
   if (test_unicity(STOREPAGE,         STOREPAGE_CODE,                FALSE)) is_ok = FALSE;
   if (test_unicity(TREASURECLASSEX,   TREASURECLASSEX_TREASURECLASS, FALSE)) is_ok = FALSE;
   if (test_unicity(UNIQUEAPPELLATION, UNIQUEAPPELLATION_NAME,        FALSE)) is_ok = FALSE;
   if (test_unicity(UNIQUEITEMS,       UNIQUEITEMS_INDEX,             TRUE))  is_ok = FALSE;
   if (test_unicity(UNIQUEPREFIX,      UNIQUEPREFIX_NAME,             FALSE)) is_ok = FALSE;
   if (test_unicity(UNIQUESUFFIX,      UNIQUESUFFIX_NAME,             FALSE)) is_ok = FALSE;
   if (test_unicity(WEAPONCLASS,       WEAPONCLASS_CODE,              FALSE)) is_ok = FALSE;

   // end 
   return is_ok == TRUE ? 0 : -1;
}


// ========================================================================================
// test if a value exists in a txt
// input :
//    * txt ID    of values to check
//    * column ID of values to check
//    * txt ID    of available codes
//    * column ID of available codes
//    * column ID (from txt with values to check) which is the row 'name'
// ouput :
//    FALSE if not found, TRUE if ok
// ========================================================================================
int test_value_1_txt(TXT_ENUM txt_idx, int col_idx, TXT_ENUM txt_idx_ref, int col_idx_ref,
                     int col_info_idx)
{
   TXT_S * txt, * txt_ref;
   int   y, y2, is_ok = TRUE, found;
   char  * str, * str2;


   // test src
   if ((txt_idx < 0) || (txt_idx >= TXT_MAX))
      return FALSE;

   txt = & glb_datas.txt[txt_idx];

   if (txt == NULL)
      return FALSE;

   if ((col_idx < 0) || (col_idx >= txt->nb_header))
      return FALSE;

   if (txt->col[col_idx] == NULL)
      return FALSE;

   if ((col_info_idx < 0) || (col_info_idx >= txt->nb_header))
      return FALSE;

   if (txt->col[col_info_idx] == NULL)
      return FALSE;

   // test ref
   if ((txt_idx_ref < 0) || (txt_idx_ref >= TXT_MAX))
      return FALSE;

   txt_ref = & glb_datas.txt[txt_idx_ref];

   if (txt_ref == NULL)
      return FALSE;

   if ((col_idx_ref < 0) || (col_idx_ref >= txt_ref->nb_header))
      return FALSE;

   if (txt_ref->col[col_idx_ref] == NULL)
      return FALSE;

   // for all rows with a value
   for (y=1; y < txt->nb_rows; y++)
   {
      str = txt->col[col_idx][y];
      if (str == NULL)
         continue;
      if (strlen(str) == 0) // blank are skipped
         continue;
      if (_stricmp(str, "Expansion") == 0)  // "Expansion" are skipped
         continue;
      if ((txt_idx == MONEQUIP) && (_stricmp(str, "*end*  do not remove") == 0)) // don't use
         continue;

      // search if the value exists
      found = FALSE;
      for (y2=1; y2 < txt_ref->nb_rows; y2++)
      {
         str2 = txt_ref->col[col_idx_ref][y2];
         if (str2 == NULL)
            continue;
         if (strlen(str2) == 0) // blank are skipped
            continue;
         if (_stricmp(str2, "Expansion") == 0)  // "Expansion" are skipped
            continue;

         if (glb_datas.str_cmp_func(str, str2) == 0)
         {
            // found
            found = TRUE;
            break;
         }
      }

      // if not found, display error
      if (found == FALSE)
      {
         if (is_ok == TRUE)
            printf("\nIn %s :\n", txt->filename);

         printf(
            "   * ERROR : '%s' (%s) at row %i (%s) not found\n",
            txt->col[col_idx][0],
            str,
            y+1,
            txt->col[col_info_idx][y]
         );
         is_ok = FALSE;
      }
   }

   // end
   if (is_ok)
      return 0;
   else
      return -1;
}


// ========================================================================================
// test all the external keys (aka secondary keys) of a txt
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_external_keys(void)
{
   TXT_ENUM t;
   int      i, done;
   struct
   {
      TXT_ENUM txt;
      int      col;
      TXT_ENUM ref_txt;
      int      ref_col;
   } data[] = {
      // txt         col                        ref_txt          ref_col
      {SKILLS,       SKILLS_SKILLDESC,          SKILLDESC,       SKILLDESC_SKILLDESC},

      {SKILLS,       SKILLS_SUMMON,             MONSTATS,        MONSTATS_ID},

      {AUTOMAGIC,    AUTOMAGIC_CLASS,           PLAYERCLASS,     PLAYERCLASS_CODE},
      {AUTOMAGIC,    AUTOMAGIC_CLASSSPECIFIC,   PLAYERCLASS,     PLAYERCLASS_CODE},
      {CUBEMAIN,     CUBEMAIN_CLASS,            PLAYERCLASS,     PLAYERCLASS_CODE},
      {ITEMTYPES,    ITEMTYPES_CLASS,           PLAYERCLASS,     PLAYERCLASS_CODE},
      {ITEMTYPES,    ITEMTYPES_STAFFMODS,       PLAYERCLASS,     PLAYERCLASS_CODE},
      {MAGICPREFIX,  MAGICAFFIX_CLASS,          PLAYERCLASS,     PLAYERCLASS_CODE},
      {MAGICPREFIX,  MAGICAFFIX_CLASSSPECIFIC,  PLAYERCLASS,     PLAYERCLASS_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_CLASS,          PLAYERCLASS,     PLAYERCLASS_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_CLASSSPECIFIC,  PLAYERCLASS,     PLAYERCLASS_CODE},
      {SKILLS,       SKILLS_CHARCLASS,          PLAYERCLASS,     PLAYERCLASS_CODE},

      {CHARSTATS,    CHARSTATS_ITEM1LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM2LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM3LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM4LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM5LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM6LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM7LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM8LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM9LOC,        BODYLOCS,        BODYLOCS_CODE},
      {CHARSTATS,    CHARSTATS_ITEM10LOC,       BODYLOCS,        BODYLOCS_CODE},
      {ITEMTYPES,    ITEMTYPES_BODYLOC1,        BODYLOCS,        BODYLOCS_CODE},
      {ITEMTYPES,    ITEMTYPES_BODYLOC2,        BODYLOCS,        BODYLOCS_CODE},
      {MONEQUIP,     MONEQUIP_LOC1,             BODYLOCS,        BODYLOCS_CODE},
      {MONEQUIP,     MONEQUIP_LOC2,             BODYLOCS,        BODYLOCS_CODE},
      {MONEQUIP,     MONEQUIP_LOC3,             BODYLOCS,        BODYLOCS_CODE},

      {ITEMTYPES,    ITEMTYPES_STOREPAGE,       STOREPAGE,       STOREPAGE_CODE},

      {MISSILES,     MISSILES_ETYPE,            ELEMTYPES,       ELEMTYPES_CODE},
      {MONSTATS,     MONSTATS_EL1TYPE,          ELEMTYPES,       ELEMTYPES_CODE},
      {MONSTATS,     MONSTATS_EL2TYPE,          ELEMTYPES,       ELEMTYPES_CODE},
      {MONSTATS,     MONSTATS_EL3TYPE,          ELEMTYPES,       ELEMTYPES_CODE},
      {SKILLDESC,    SKILLDESC_P1DMELEM,        ELEMTYPES,       ELEMTYPES_CODE},
      {SKILLDESC,    SKILLDESC_P2DMELEM,        ELEMTYPES,       ELEMTYPES_CODE},
      {SKILLDESC,    SKILLDESC_P3DMELEM,        ELEMTYPES,       ELEMTYPES_CODE},
      {SKILLS,       SKILLS_ETYPE,              ELEMTYPES,       ELEMTYPES_CODE},

      {ARMOR,        AWM_HITCLASS,              HITCLASS,        HITCLASS_CODE},
      {MISC,         AWM_HITCLASS,              HITCLASS,        HITCLASS_CODE},
      {WEAPONS,      AWM_HITCLASS,              HITCLASS,        HITCLASS_CODE},

      {MONSEQ,       MONSEQ_MODE,               MONMODE,         MONMODE_CODE},
      {MONSOUNDS,    MONSOUNDS_CVTMO1,          MONMODE,         MONMODE_CODE},
      {MONSOUNDS,    MONSOUNDS_CVTMO2,          MONMODE,         MONMODE_CODE},
      {MONSOUNDS,    MONSOUNDS_CVTMO3,          MONMODE,         MONMODE_CODE},
      {MONSOUNDS,    MONSOUNDS_CVTTGT1,         MONMODE,         MONMODE_CODE},
      {MONSOUNDS,    MONSOUNDS_CVTTGT2,         MONMODE,         MONMODE_CODE},
      {MONSOUNDS,    MONSOUNDS_CVTTGT3,         MONMODE,         MONMODE_CODE},
      {MONSTATS,     MONSTATS_EL1MODE,          MONMODE,         MONMODE_CODE},
      {MONSTATS,     MONSTATS_EL2MODE,          MONMODE,         MONMODE_CODE},
      {MONSTATS,     MONSTATS_EL3MODE,          MONMODE,         MONMODE_CODE},
      {MONSTATS,     MONSTATS_SPAWNMODE,        MONMODE,         MONMODE_CODE},
      {MONSTATS2,    MONSTATS2_RESURRECTMODE,   MONMODE,         MONMODE_CODE},
      {SKILLS,       SKILLS_MONANIM,            MONMODE,         MONMODE_CODE},
      {SKILLS,       SKILLS_SUMMODE,            MONMODE,         MONMODE_CODE},

      {SKILLS,       SKILLS_ANIM,               PLRMODE,         PLRMODE_CODE},
      {SKILLS,       SKILLS_SEQTRANS,           PLRMODE,         PLRMODE_CODE},

      {MISSILES,     MISSILES_SKILL,            SKILLS,          SKILLS_SKILL},
      {STATES,       STATES_SKILL,              SKILLS,          SKILLS_SKILL},

      {ITEMSTATCOST, ITEMSTATCOST_ITEMEVENT1,   EVENTS,          EVENTS_EVENT},
      {ITEMSTATCOST, ITEMSTATCOST_ITEMEVENT2,   EVENTS,          EVENTS_EVENT},
      {SKILLS,       SKILLS_AURAEVENT1,         EVENTS,          EVENTS_EVENT},
      {SKILLS,       SKILLS_AURAEVENT2,         EVENTS,          EVENTS_EVENT},
      {SKILLS,       SKILLS_AURAEVENT3,         EVENTS,          EVENTS_EVENT},
      {SKILLS,       SKILLS_AURATGTEVENT,       EVENTS,          EVENTS_EVENT},
      {SKILLS,       SKILLS_PASSIVEEVENT,       EVENTS,          EVENTS_EVENT},
      {STATES,       STATES_CLTEVENT,           EVENTS,          EVENTS_EVENT},

      {MONSTATS,     MONSTATS_AI,               MONAI,           MONAI_AI},

      {MONEQUIP,     MONEQUIP_ITEM1,            AWM,             AWM_CODE},
      {MONEQUIP,     MONEQUIP_ITEM2,            AWM,             AWM_CODE},
      {MONEQUIP,     MONEQUIP_ITEM3,            AWM,             AWM_CODE},
      {GEMS,         GEMS_CODE,                 AWM,             AWM_CODE},
      {RUNES,        RUNES_RUNE1,               AWM,             AWM_CODE},
      {RUNES,        RUNES_RUNE2,               AWM,             AWM_CODE},
      {RUNES,        RUNES_RUNE3,               AWM,             AWM_CODE},
      {RUNES,        RUNES_RUNE4,               AWM,             AWM_CODE},
      {RUNES,        RUNES_RUNE5,               AWM,             AWM_CODE},
      {RUNES,        RUNES_RUNE6,               AWM,             AWM_CODE},

      {AUTOMAGIC,    AUTOMAGIC_MOD1CODE,        PROPERTIES,      PROPERTIES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_MOD2CODE,        PROPERTIES,      PROPERTIES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_MOD3CODE,        PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_BMOD1,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_BMOD2,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_BMOD3,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_BMOD4,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_BMOD5,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_CMOD1,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_CMOD2,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_CMOD3,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_CMOD4,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_CMOD5,            PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_MOD1,             PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_MOD2,             PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_MOD3,             PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_MOD4,             PROPERTIES,      PROPERTIES_CODE},
      {CUBEMAIN,     CUBEMAIN_MOD5,             PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_HELMMOD1CODE,         PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_HELMMOD2CODE,         PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_HELMMOD3CODE,         PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_SHIELDMOD1CODE,       PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_SHIELDMOD2CODE,       PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_SHIELDMOD3CODE,       PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_WEAPONMOD1CODE,       PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_WEAPONMOD2CODE,       PROPERTIES,      PROPERTIES_CODE},
      {GEMS,         GEMS_WEAPONMOD3CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_MOD1CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_MOD2CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_MOD3CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_MOD1CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_MOD2CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_MOD3CODE,       PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP1,             PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP1H,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP1N,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP2,             PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP2H,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP2N,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP3,             PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP3H,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP3N,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP4,             PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP4H,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP4N,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP5,             PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP5H,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP5N,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP6,             PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP6H,            PROPERTIES,      PROPERTIES_CODE},
      {MONPROP,      MONPROP_PROP6N,            PROPERTIES,      PROPERTIES_CODE},
      {QUALITYITEMS, QUALITYITEMS_MOD1CODE,     PROPERTIES,      PROPERTIES_CODE},
      {QUALITYITEMS, QUALITYITEMS_MOD2CODE,     PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE1,             PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE2,             PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE3,             PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE4,             PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE5,             PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE6,             PROPERTIES,      PROPERTIES_CODE},
      {RUNES,        RUNES_T1CODE7,             PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP1A,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP1B,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP2A,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP2B,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP3A,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP3B,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP4A,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP4B,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP5A,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_APROP5B,          PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP1,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP2,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP3,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP4,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP5,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP6,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP7,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP8,            PROPERTIES,      PROPERTIES_CODE},
      {SETITEMS,     SETITEMS_PROP9,            PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE1,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE2,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE3,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE4,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE5,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE6,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE7,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_FCODE8,               PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE2A,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE2B,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE3A,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE3B,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE4A,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE4B,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE5A,              PROPERTIES,      PROPERTIES_CODE},
      {SETS,         SETS_PCODE5B,              PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP1,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP2,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP3,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP4,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP5,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP6,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP7,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP8,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP9,         PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP10,        PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP11,        PROPERTIES,      PROPERTIES_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_PROP12,        PROPERTIES,      PROPERTIES_CODE},

      {HIRELING,     HIRELING_HIREDESC,         HIREDESC,        HIREDESC_CODE},

      {ARMOR,        AWM_CSTATE1,               STATES,          STATES_STATE},
      {ARMOR,        AWM_CSTATE2,               STATES,          STATES_STATE},
      {ARMOR,        AWM_STATE,                 STATES,          STATES_STATE},
      {WEAPONS,      AWM_CSTATE1,               STATES,          STATES_STATE},
      {WEAPONS,      AWM_CSTATE2,               STATES,          STATES_STATE},
      {WEAPONS,      AWM_STATE,                 STATES,          STATES_STATE},
      {MISC,         AWM_CSTATE1,               STATES,          STATES_STATE},
      {MISC,         AWM_CSTATE2,               STATES,          STATES_STATE},
      {MISC,         AWM_STATE,                 STATES,          STATES_STATE},
      {SKILLS,       SKILLS_AURASTATE,          STATES,          STATES_STATE},
      {SKILLS,       SKILLS_AURATARGETSTATE,    STATES,          STATES_STATE},
      {SKILLS,       SKILLS_PASSIVESTATE,       STATES,          STATES_STATE},
      {SKILLS,       SKILLS_STATE1,             STATES,          STATES_STATE},
      {SKILLS,       SKILLS_STATE2,             STATES,          STATES_STATE},
      {SKILLS,       SKILLS_STATE3,             STATES,          STATES_STATE},

      {MONSOUNDS,    MONSOUNDS_ATTACK1,         SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_ATTACK2,         SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_DEATHSOUND,      SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_FLEE,            SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_FOOTSTEP,        SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_FOOTSTEPLAYER,   SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_HITSOUND,        SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_INIT,            SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_NEUTRAL,         SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_SKILL1,          SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_SKILL2,          SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_SKILL3,          SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_SKILL4,          SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_TAUNT,           SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_WEAPON1,         SOUNDS,          SOUNDS_SOUND},
      {MONSOUNDS,    MONSOUNDS_WEAPON2,         SOUNDS,          SOUNDS_SOUND},
      {ARMOR,        AWM_DROPSOUND,             SOUNDS,          SOUNDS_SOUND},
      {ARMOR,        AWM_USESOUND,              SOUNDS,          SOUNDS_SOUND},
      {WEAPONS,      AWM_DROPSOUND,             SOUNDS,          SOUNDS_SOUND},
      {WEAPONS,      AWM_USESOUND,              SOUNDS,          SOUNDS_SOUND},
      {MISC,         AWM_DROPSOUND,             SOUNDS,          SOUNDS_SOUND},
      {MISC,         AWM_USESOUND,              SOUNDS,          SOUNDS_SOUND},
      {MISSILES,     MISSILES_HITSOUND,         SOUNDS,          SOUNDS_SOUND},
      {MISSILES,     MISSILES_PROGSOUND,        SOUNDS,          SOUNDS_SOUND},
      {MISSILES,     MISSILES_TRAVELSOUND,      SOUNDS,          SOUNDS_SOUND},
      {SETITEMS,     SETITEMS_DROPSOUND,        SOUNDS,          SOUNDS_SOUND},
      {SETITEMS,     SETITEMS_USESOUND,         SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_DOSOUND,            SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_DOSOUNDA,           SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_DOSOUNDB,           SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_ITEMCASTSOUND,      SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_PRGSOUND,           SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_STSOUND,            SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_STSOUNDCLASS,       SOUNDS,          SOUNDS_SOUND},
      {SKILLS,       SKILLS_TGTSOUND,           SOUNDS,          SOUNDS_SOUND},
      {STATES,       STATES_OFFSOUND,           SOUNDS,          SOUNDS_SOUND},
      {STATES,       STATES_ONSOUND,            SOUNDS,          SOUNDS_SOUND},
      {UNIQUEITEMS,  UNIQUEITEMS_DROPSOUND,     SOUNDS,          SOUNDS_SOUND},
      {UNIQUEITEMS,  UNIQUEITEMS_USESOUND,      SOUNDS,          SOUNDS_SOUND},

      {AUTOMAGIC,    AUTOMAGIC_TRANSFORMCOLOR,  COLORS,          COLORS_CODE},
      {MAGICPREFIX,  MAGICAFFIX_TRANSFORMCOLOR, COLORS,          COLORS_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_TRANSFORMCOLOR, COLORS,          COLORS_CODE},
      {SETITEMS,     SETITEMS_CHRTRANSFORM,     COLORS,          COLORS_CODE},
      {SETITEMS,     SETITEMS_INVTRANSFORM,     COLORS,          COLORS_CODE},
      {STATES,       STATES_ITEMTRANS,          COLORS,          COLORS_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_CHRTRANSFORM,  COLORS,          COLORS_CODE},
      {UNIQUEITEMS,  UNIQUEITEMS_INVTRANSFORM,  COLORS,          COLORS_CODE},

      {MONSTATS,     MONSTATS_TREASURECLASS1,   TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS1H,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS1N,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS2,   TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS2H,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS2N,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS3,   TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS3H,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS3N,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS4,   TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS4H,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {MONSTATS,     MONSTATS_TREASURECLASS4N,  TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {SUPERUNIQUES, SUPERUNIQUES_TC,           TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {SUPERUNIQUES, SUPERUNIQUES_TCH,          TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},
      {SUPERUNIQUES, SUPERUNIQUES_TCN,          TREASURECLASSEX, TREASURECLASSEX_TREASURECLASS},

      {NPC,          NPC_NPC,                   MONSTATS,        MONSTATS_ID},
      {SUPERUNIQUES, SUPERUNIQUES_CLASS,        MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_CMON1,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_CMON2,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_CMON3,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_CMON4,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON1,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON2,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON3,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON4,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON5,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON6,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON7,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON8,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON9,               MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON10,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON11,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON12,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON13,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON14,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON15,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON16,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON17,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON18,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON19,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON20,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON21,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON22,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON23,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON24,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_MON25,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON1,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON2,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON3,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON4,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON5,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON6,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON7,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON8,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON9,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON10,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON11,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON12,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON13,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON14,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON15,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON16,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON17,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON18,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON19,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON20,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON21,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON22,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON23,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON24,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_NMON25,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON1,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON2,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON3,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON4,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON5,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON6,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON7,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON8,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON9,              MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON10,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON11,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON12,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON13,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON14,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON15,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON16,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON17,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON18,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON19,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON20,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON21,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON22,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON23,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON24,             MONSTATS,        MONSTATS_ID},
      {LEVELS,       LEVELS_UMON25,             MONSTATS,        MONSTATS_ID},
      {MONEQUIP,     MONEQUIP_MONSTER,          MONSTATS,        MONSTATS_ID},
//      {MONSTATS,     MONSTATS_BASEID,           MONSTATS,        MONSTATS_ID},
      {MONSTATS,     MONSTATS_MINION1,          MONSTATS,        MONSTATS_ID},
      {MONSTATS,     MONSTATS_MINION2,          MONSTATS,        MONSTATS_ID},
      {MONSTATS,     MONSTATS_NEXTINCLASS,      MONSTATS,        MONSTATS_ID},
      {MONSTATS,     MONSTATS_SPAWN,            MONSTATS,        MONSTATS_ID},

      {SUPERUNIQUES, SUPERUNIQUES_MONSOUND,     MONSOUNDS,       MONSOUNDS_ID},
      {MONSTATS,     MONSTATS_MONSOUND,         MONSOUNDS,       MONSOUNDS_ID},
      {MONSTATS,     MONSTATS_UMONSOUND,        MONSOUNDS,       MONSOUNDS_ID},

      {MONSTATS,     MONSTATS_MONSTATSEX,       MONSTATS2,       MONSTATS2_ID},

      {MISSILES,     MISSILES_CLTHITSUBMISSILE1, MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_CLTHITSUBMISSILE2, MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_CLTHITSUBMISSILE3, MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_CLTHITSUBMISSILE4, MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_CLTSUBMISSILE1,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_CLTSUBMISSILE2,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_CLTSUBMISSILE3,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_EXPLOSIONMISSILE,  MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_HITSUBMISSILE1,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_HITSUBMISSILE2,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_HITSUBMISSILE3,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_HITSUBMISSILE4,    MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_SUBMISSILE1,       MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_SUBMISSILE2,       MISSILES,       MISSILES_MISSILE},
      {MISSILES,     MISSILES_SUBMISSILE3,       MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSA1,            MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSA2,            MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSC,             MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSS1,            MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSS2,            MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSS3,            MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSS4,            MISSILES,       MISSILES_MISSILE},
      {MONSTATS,     MONSTATS_MISSSQ,            MISSILES,       MISSILES_MISSILE},
      {SKILLDESC,    SKILLDESC_DESCMISSILE1,     MISSILES,       MISSILES_MISSILE},
      {SKILLDESC,    SKILLDESC_DESCMISSILE2,     MISSILES,       MISSILES_MISSILE},
      {SKILLDESC,    SKILLDESC_DESCMISSILE3,     MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_CLTMISSILE,          MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_CLTMISSILEA,         MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_CLTMISSILEB,         MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_CLTMISSILEC,         MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_CLTMISSILED,         MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_SRVMISSILE,          MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_SRVMISSILEA,         MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_SRVMISSILEB,         MISSILES,       MISSILES_MISSILE},
      {SKILLS,       SKILLS_SRVMISSILEC,         MISSILES,       MISSILES_MISSILE},
      {STATES,       STATES_MISSILE,             MISSILES,       MISSILES_MISSILE},

      {BOOKS,        BOOKS_BOOKSKILL,            SKILLS,         SKILLS_SKILL},
      {BOOKS,        BOOKS_SCROLLSKILL,          SKILLS,         SKILLS_SKILL},
      {HIRELING,     HIRELING_SKILL1,            SKILLS,         SKILLS_SKILL},
      {HIRELING,     HIRELING_SKILL2,            SKILLS,         SKILLS_SKILL},
      {HIRELING,     HIRELING_SKILL3,            SKILLS,         SKILLS_SKILL},
      {HIRELING,     HIRELING_SKILL4,            SKILLS,         SKILLS_SKILL},
      {HIRELING,     HIRELING_SKILL5,            SKILLS,         SKILLS_SKILL},
      {HIRELING,     HIRELING_SKILL6,            SKILLS,         SKILLS_SKILL},
      {MONSOUNDS,    MONSOUNDS_CVTSK1,           SKILLS,         SKILLS_SKILL},
      {MONSOUNDS,    MONSOUNDS_CVTSK2,           SKILLS,         SKILLS_SKILL},
      {MONSOUNDS,    MONSOUNDS_CVTSK3,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL1,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL2,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL3,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL4,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL5,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL6,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL7,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL8,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL9,           SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_SKILL10,          SKILLS,         SKILLS_SKILL},
      {CHARSTATS,    CHARSTATS_STARTSKILL,       SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL1,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL2,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL3,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL4,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL5,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL6,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL7,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILL8,            SKILLS,         SKILLS_SKILL},
      {MONSTATS,     MONSTATS_SKILLDAMAGE,       SKILLS,         SKILLS_SKILL},
      {MONSTATS2,    MONSTATS2_RESURRECTSKILL,   SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_REQSKILL1,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_REQSKILL2,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_REQSKILL3,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_SUMSKILL1,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_SUMSKILL2,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_SUMSKILL3,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_SUMSKILL4,           SKILLS,         SKILLS_SKILL},
      {SKILLS,       SKILLS_SUMSKILL5,           SKILLS,         SKILLS_SKILL},

      {MISSILES,     MISSILES_PROGOVERLAY,       OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_CASTOVERLAY,         OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_CLTOVERLAYA,         OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_CLTOVERLAYB,         OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_ITEMCASTOVERLAY,     OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_PRGOVERLAY,          OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_SRVOVERLAY,          OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_SUMOVERLAY,          OVERLAY,        OVERLAY_OVERLAY},
      {SKILLS,       SKILLS_TGTOVERLAY,          OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_CASTOVERLAY,         OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_OVERLAY1,            OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_OVERLAY2,            OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_OVERLAY3,            OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_OVERLAY4,            OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_PGSVOVERLAY,         OVERLAY,        OVERLAY_OVERLAY},
      {STATES,       STATES_REMOVERLAY,          OVERLAY,        OVERLAY_OVERLAY},

      {ARMOR,        AWM_STAT1,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ARMOR,        AWM_STAT2,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ARMOR,        AWM_STAT3,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {WEAPONS,      AWM_STAT1,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {WEAPONS,      AWM_STAT2,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {WEAPONS,      AWM_STAT3,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {MISC,         AWM_STAT1,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {MISC,         AWM_STAT2,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {MISC,         AWM_STAT3,                  ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ITEMSTATCOST, ITEMSTATCOST_MAXSTAT,       ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ITEMSTATCOST, ITEMSTATCOST_OPBASE,        ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ITEMSTATCOST, ITEMSTATCOST_OPSTAT1,       ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ITEMSTATCOST, ITEMSTATCOST_OPSTAT2,       ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {ITEMSTATCOST, ITEMSTATCOST_OPSTAT3,       ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT1,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT2,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT3,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT4,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT5,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT6,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {PROPERTIES,   PROPERTIES_STAT7,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_AURASTAT1,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_AURASTAT2,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_AURASTAT3,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_AURASTAT4,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_AURASTAT5,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_AURASTAT6,           ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_PASSIVESTAT1,        ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_PASSIVESTAT2,        ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_PASSIVESTAT3,        ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_PASSIVESTAT4,        ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {SKILLS,       SKILLS_PASSIVESTAT5,        ITEMSTATCOST,   ITEMSTATCOST_STAT},
      {STATES,       STATES_STAT,                ITEMSTATCOST,   ITEMSTATCOST_STAT},

      {SKILLS,       SKILLS_PETTYPE,             PETTYPE,        PETTYPE_PETTYPE},

      {ARMOR,        AWM_TYPE,                   ITEMTYPES,      ITEMTYPES_CODE},
      {ARMOR,        AWM_TYPE2,                  ITEMTYPES,      ITEMTYPES_CODE},
      {WEAPONS,      AWM_TYPE,                   ITEMTYPES,      ITEMTYPES_CODE},
      {WEAPONS,      AWM_TYPE2,                  ITEMTYPES,      ITEMTYPES_CODE},
      {MISC,         AWM_TYPE,                   ITEMTYPES,      ITEMTYPES_CODE},
      {MISC,         AWM_TYPE2,                  ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ETYPE1,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ETYPE2,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ETYPE3,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ETYPE4,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ETYPE5,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE1,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE2,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE3,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE4,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE5,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE6,           ITEMTYPES,      ITEMTYPES_CODE},
      {AUTOMAGIC,    AUTOMAGIC_ITYPE7,           ITEMTYPES,      ITEMTYPES_CODE},
      {ITEMTYPES,    ITEMTYPES_EQUIV1,           ITEMTYPES,      ITEMTYPES_CODE},
      {ITEMTYPES,    ITEMTYPES_EQUIV2,           ITEMTYPES,      ITEMTYPES_CODE},
      {ITEMTYPES,    ITEMTYPES_QUIVER,           ITEMTYPES,      ITEMTYPES_CODE},
      {ITEMTYPES,    ITEMTYPES_SHOOTS,           ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ETYPE1,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ETYPE2,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ETYPE3,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ETYPE4,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ETYPE5,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE1,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE2,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE3,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE4,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE5,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE6,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICPREFIX,  MAGICAFFIX_ITYPE7,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ETYPE1,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ETYPE2,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ETYPE3,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ETYPE4,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ETYPE5,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE1,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE2,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE3,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE4,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE5,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE6,          ITEMTYPES,      ITEMTYPES_CODE},
      {MAGICSUFFIX,  MAGICAFFIX_ITYPE7,          ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ETYPE1,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ETYPE2,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ETYPE3,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ETYPE4,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE1,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE2,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE3,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE4,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE5,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE6,           ITEMTYPES,      ITEMTYPES_CODE},
      {RAREPREFIX,   RAREAFFIX_ITYPE7,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ETYPE1,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ETYPE2,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ETYPE3,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ETYPE4,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE1,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE2,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE3,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE4,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE5,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE6,           ITEMTYPES,      ITEMTYPES_CODE},
      {RARESUFFIX,   RAREAFFIX_ITYPE7,           ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ETYPE1,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ETYPE2,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ETYPE3,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ITYPE1,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ITYPE2,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ITYPE3,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ITYPE4,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ITYPE5,               ITEMTYPES,      ITEMTYPES_CODE},
      {RUNES,        RUNES_ITYPE6,               ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ETYPEA1,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ETYPEA2,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ETYPEB1,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ETYPEB2,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ITYPEA1,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ITYPEA2,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ITYPEA3,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ITYPEB1,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ITYPEB2,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_ITYPEB3,             ITEMTYPES,      ITEMTYPES_CODE},
      {SKILLS,       SKILLS_PASSIVEITYPE,        ITEMTYPES,      ITEMTYPES_CODE},
      {STATES,       STATES_ITEMTYPE,            ITEMTYPES,      ITEMTYPES_CODE},

      {SETITEMS,     SETITEMS_SET,               SETS,           SETS_INDEX},

      {MONSTATS,     MONSTATS_MONPROP,           MONPROP,        MONPROP_ID},

      {MONSTATS,     MONSTATS_MONTYPE,           MONTYPE,        MONTYPE_TYPE},
      {MONTYPE,      MONTYPE_EQUIV1,             MONTYPE,        MONTYPE_TYPE},
      {MONTYPE,      MONTYPE_EQUIV2,             MONTYPE,        MONTYPE_TYPE},
      {MONTYPE,      MONTYPE_EQUIV3,             MONTYPE,        MONTYPE_TYPE},
      {MONUMOD,      MONUMOD_EXCLUDE1,           MONTYPE,        MONTYPE_TYPE},
      {MONUMOD,      MONUMOD_EXCLUDE2,           MONTYPE,        MONTYPE_TYPE},

      {NONE, 0, NONE, 0}
   };

   for (t=0; t < TXT_MAX; t++)
   {
      if (t == NONE)
         continue;

      i=0;
      done = FALSE;
      while ( ! done)
      {
         if (data[i].txt == NONE)
            done = TRUE;
         else
         {
            if (data[i].txt == t)
            {
               test_value_1_txt(
                  data[i].txt,
                  data[i].col,
                  data[i].ref_txt,
                  data[i].ref_col,
                  0
               );
            }
         }
         i++;
      }
   }

   return 0;
}
