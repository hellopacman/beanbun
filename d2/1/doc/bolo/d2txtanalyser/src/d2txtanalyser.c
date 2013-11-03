#include <stdlib.h>
#include <string.h>
#include <direct.h>

#include "types.h"
#include "iniread.h"
#include "txtread.h"
#include "filmem.h"
#include "error.h"
#include "test_keys.h"
#include "test.h"
#include "mpq\mpqtypes.h"
#include "mpq\mpqview.h"
#include "globals.h"

// global datas
GLB_DATAS_S glb_datas;

// pointer to current mpq file structure
// (define in mpq\mpqview.c)
extern GLB_MPQ_S * glb_mpq;

// current version
#define D2TXTANALYSER_VERSION "2.3"



// ========================================================================================
// free our memory when the program exit
// ========================================================================================
void d2txtanalyser_exit(void)
{
   int i, c;


   for (i=0; i < MPQ_MAX; i++)
   {
      if (glb_datas.mpq_filename[i])
         free(glb_datas.mpq_filename[i]);
   }

   if (glb_datas.mod_dir)
      free(glb_datas.mod_dir);

   if (glb_datas.ini)
      free(glb_datas.ini);

   for (i=0; i < MPQ_MAX; i++)
   {
      glb_mpq = & glb_datas.mpq[i];
      if (glb_mpq->is_open == TRUE)
         mpq_batch_close();
   }

   for (i=0; i < TXT_MAX; i++)
   {
      if (glb_datas.txt[i].buffer)
         free(glb_datas.txt[i].buffer);
      if (glb_datas.txt[i].col)
      {
         for (c=0; c < glb_datas.txt[i].nb_header; c++)
         {
            if (glb_datas.txt[i].col[c])
               free(glb_datas.txt[i].col[c]);
         }
         free(glb_datas.txt[i].col);
      }
   }

   if (glb_datas.std_warning)
      fclose(glb_datas.std_warning);
}


// ========================================================================================
// check if a directory exists
// input :
//    * directory path & name
// output :
//    TRUE if exists, FALSE if not found
// ========================================================================================
int d2txtanalyser_directory_exists(char * filename)
{
   char * buffer;
   int  is_ok = FALSE;


   // keep track of the current drive & directory
   buffer = _getcwd(NULL, 256);
   if (buffer == NULL)
      return FALSE;

   // try to move to that drive & directory
   if (_chdir(filename) == 0)
   {
      // sucess
      is_ok = TRUE;

      // get back to the original drive & directory
      _chdir(buffer);
   }

   // end
   free(buffer);
   return is_ok;
}


// ========================================================================================
// load & process D2TxtAnalyser ini
// ========================================================================================
void d2txtanalyser_handle_ini(void)
{
   char * ini_name = "d2txtanalyser.ini";
   int  is_ok = TRUE;

   
   if (ini_read(ini_name))
   {
      sprintf(glb_datas.error_strtmp, "d2txtanalyser_handle_ini() : can't read %s\n", ini_name);
      d2txtanalyser_error(glb_datas.error_strtmp);
      exit(-1);
   }

   // if no paths at all, error
   if (( ! glb_datas.mod_dir) && ( ! glb_datas.mpq_filename[D2DATA])
                              && ( ! glb_datas.mpq_filename[D2EXP])
                              && ( ! glb_datas.mpq_filename[PATCH_D2]))
   {
      d2txtanalyser_error("d2txtanalyser_handle_ini() : ERROR\n"
         "   At least 1 mpq or mod directory must be define in d2txtanalyser.ini\n"
      );
      exit(-1);
   }

   // show paths found (mpq & mod_dir)
   if (glb_datas.mpq_filename[D2DATA])
   {
      fprintf(stderr, "   * d2data   ");
      if ( ! filmem_file_exists(glb_datas.mpq_filename[D2DATA]))
      {
         fprintf(stderr, "(ERROR)");
         is_ok = FALSE;
      }
      else
         fprintf(stderr, "       ");
      fprintf(stderr, " = %s\n", glb_datas.mpq_filename[D2DATA]);
      fflush(stdout);
   }

   if (glb_datas.mpq_filename[D2EXP])
   {
      fprintf(stderr, "   * d2exp    ");
      if ( ! filmem_file_exists(glb_datas.mpq_filename[D2EXP]))
      {
         fprintf(stderr, "(ERROR)");
         is_ok = FALSE;
      }
      else
         fprintf(stderr, "       ");
      fprintf(stderr, " = %s\n", glb_datas.mpq_filename[D2EXP]);
      fflush(stdout);
   }

   if (glb_datas.mpq_filename[PATCH_D2])
   {
      fprintf(stderr, "   * patch_d2 ");
      if ( ! filmem_file_exists(glb_datas.mpq_filename[PATCH_D2]))
      {
         fprintf(stderr, "(ERROR)");
         is_ok = FALSE;
      }
      else
         fprintf(stderr, "       ");
      fprintf(stderr, " = %s\n", glb_datas.mpq_filename[PATCH_D2]);
      fflush(stdout);
   }

   if (glb_datas.mod_dir)
   {
      fprintf(stderr, "   * mod_dir  ");
      if ( ! d2txtanalyser_directory_exists(glb_datas.mod_dir))
      {
         fprintf(stderr, "(ERROR)");
         is_ok = FALSE;
      }
      else
         fprintf(stderr, "       ");
      fprintf(stderr, " = %s\n", glb_datas.mod_dir);
      fflush(stdout);
   }

   // user options
   if (glb_datas.be_strict)
   {
      fprintf(stderr, "   * be_strict        = ");
      if ( _stricmp(glb_datas.be_strict, "YES") == 0 )
      {
         fprintf(stderr, "YES\n");
         glb_datas.str_cmp_func = strcmp;
      }
      else
      {
         fprintf(stderr, "NO\n");
         glb_datas.str_cmp_func = _stricmp;
      }
      fflush(stdout);
   }

   if (is_ok == FALSE)
   {
      sprintf(glb_datas.error_strtmp, "d2txtanalyser_handle_ini() : ERROR\n"
         "At least 1 path in %s is wrong, correct it/them and try again\n", ini_name);
      d2txtanalyser_error(glb_datas.error_strtmp);
      exit(-1);
   }
}


// ========================================================================================
// open all mpq, for future extractions
// ========================================================================================
void d2txtanalyser_open_all_mpq(void)
{
   int i;


   fprintf(stderr, "\nOpening necessary MPQ...\n");
   for (i=0; i < MPQ_MAX; i++)
   {
      if (glb_datas.mpq_filename[i] != NULL)
      {
         fprintf(stderr, "   * %s\n", glb_datas.mpq_filename[i]);
         glb_mpq = & glb_datas.mpq[i];
         mpq_batch_open(glb_datas.mpq_filename[i]);
      }
   }
}


// ========================================================================================
// load 1 txt and prepare it for future use
// ========================================================================================
int d2txtanalyser_load_one_txt(char * filename, TXT_ENUM txt_idx)
{
   printf("\n");
   if (txt_read(filename, txt_idx) == 0)
   {
      if (txt_display_infos(txt_idx) == 0)
      {
         if (txt_replace_tab_by_zero(txt_idx) == 0)
         {
            txt_init_values_pointers(txt_idx);
            fflush(stdout);
            return 0;
         }
      }
   }
   fflush(stdout);

   // end
   return 1;
}

// ========================================================================================
// build Armor.txt, Weapons.txt and Misc.txt into 1 file
// this is for future checks, this way we will work with only 1 txt, not 3
// ========================================================================================
void d2txtanalyser_make_awm(void)
{
   TXT_S * awm =   & glb_datas.txt[AWM],
         * armor = & glb_datas.txt[ARMOR],
         * weap  = & glb_datas.txt[WEAPONS],
         * misc  = & glb_datas.txt[MISC];
   int   i, size, r, curr_row;


   if ((armor == NULL) || (weap == NULL) || (misc == NULL))
      return;

   awm->txt_idx = AWM;
   strcpy(awm->filename, "Armor / Weapons / Misc.txt");
   awm->header = glb_datas.txt_col_names.awm_header;
   awm->nb_rows = armor->nb_rows + weap->nb_rows + misc->nb_rows;
   size = sizeof(char **) * AWM_COL_MAX;
   awm->col = (char ***) malloc(size);
   if (awm->col == NULL)
      return;
   memset(awm->col, 0, size);

   size = sizeof(char *) * awm->nb_rows;
   for (i=0; i < AWM_COL_MAX; i++)
   {
      if ((armor->col[i] == NULL) && (weap->col[i] == NULL) && (misc->col[i] == NULL))
         continue;
      awm->col[i] = (char **) malloc(size);
      if (awm->col == NULL)
         return;
      memset(awm->col[i], 0, size);
      curr_row = 0;

      if (armor->col[i] != NULL)
      {
         for (r=0; r < armor->nb_rows; r++)
         {
            awm->col[i][curr_row] = armor->col[i][r];
            curr_row++;
         }
      }

      if (weap->col[i] != NULL)
      {
         for (r=0; r < weap->nb_rows; r++)
         {
            awm->col[i][curr_row] = weap->col[i][r];
            curr_row++;
         }
      }

      if (misc->col[i] != NULL)
      {
         for (r=0; r < misc->nb_rows; r++)
         {
            awm->col[i][curr_row] = misc->col[i][r];
            curr_row++;
         }
      }
   }
}


// ========================================================================================
// load all .txt
// output :
//    * number of txt files that couldn't be opened / processed properly
// ========================================================================================
int d2txtanalyser_load_txt(void)
{
   int nb_err = 0;


   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Armor.txt",             ARMOR);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Weapons.txt",           WEAPONS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Misc.txt",              MISC);
   d2txtanalyser_make_awm();

   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ArmType.txt",           ARMTYPE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\AutoMagic.txt",         AUTOMAGIC);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\AutoMap.txt",           AUTOMAP);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Belts.txt",             BELTS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\BodyLocs.txt",          BODYLOCS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Books.txt",             BOOKS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\CharStats.txt",         CHARSTATS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Colors.txt",            COLORS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\CompCode.txt",          COMPCODE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Composit.txt",          COMPOSIT);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\CubeMain.txt",          CUBEMAIN);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\CubeMod.txt",           CUBEMOD);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\CubeType.txt",          CUBETYPE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\DifficultyLevels.txt",  DIFFICULTYLEVELS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ElemTypes.txt",         ELEMTYPES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Events.txt",            EVENTS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Experience.txt",        EXPERIENCE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Gamble.txt",            GAMBLE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Gems.txt",              GEMS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\HireDesc.txt",          HIREDESC);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Hireling.txt",          HIRELING);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\HitClass.txt",          HITCLASS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Inventory.txt",         INVENTORY);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ItemRatio.txt",         ITEMRATIO);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ItemStatCost.txt",      ITEMSTATCOST);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ItemTypes.txt",         ITEMTYPES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Levels.txt",            LEVELS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\LowQualityItems.txt",   LOWQUALITYITEMS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\LvlMaze.txt",           LVLMAZE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\LvlPrest.txt",          LVLPREST);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\LvlSub.txt",            LVLSUB);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\LvlTypes.txt",          LVLTYPES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\LvlWarp.txt",           LVLWARP);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MagicPrefix.txt",       MAGICPREFIX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MagicSuffix.txt",       MAGICSUFFIX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MissCalc.txt",          MISSCALC);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Missiles.txt",          MISSILES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonAI.txt",             MONAI);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonEquip.txt",          MONEQUIP);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonLvl.txt",            MONLVL);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonMode.txt",           MONMODE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonName.txt",           MONNAME);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonPlace.txt",          MONPLACE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonPreset.txt",         MONPRESET);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonProp.txt",           MONPROP);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonSeq.txt",            MONSEQ);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonSounds.txt",         MONSOUNDS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonStats.txt",          MONSTATS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonStats2.txt",         MONSTATS2);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonType.txt",           MONTYPE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\MonUMod.txt",           MONUMOD);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Npc.txt",               NPC);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Objects.txt",           OBJECTS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ObjGroup.txt",          OBJGROUP);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ObjMode.txt",           OBJMODE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\ObjType.txt",           OBJTYPE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Overlay.txt",           OVERLAY);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\PetType.txt",           PETTYPE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\PlayerClass.txt",       PLAYERCLASS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\PlrMode.txt",           PLRMODE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\PlrType.txt",           PLRTYPE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Properties.txt",        PROPERTIES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\QualityItems.txt",      QUALITYITEMS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\RarePrefix.txt",        RAREPREFIX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\RareSuffix.txt",        RARESUFFIX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Runes.txt",             RUNES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\SetItems.txt",          SETITEMS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Sets.txt",              SETS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Shrines.txt",           SHRINES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\SkillCalc.txt",         SKILLCALC);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\SkillDesc.txt",         SKILLDESC);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Skills.txt",            SKILLS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\SoundEnviron.txt",      SOUNDENVIRON);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\Sounds.txt",            SOUNDS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\States.txt",            STATES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\StorePage.txt",         STOREPAGE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\SuperUniques.txt",      SUPERUNIQUES);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\TreasureClassEx.txt",   TREASURECLASSEX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\UniqueAppellation.txt", UNIQUEAPPELLATION);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\UniqueItems.txt",       UNIQUEITEMS);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\UniquePrefix.txt",      UNIQUEPREFIX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\UniqueSuffix.txt",      UNIQUESUFFIX);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\UniqueTitle.txt",       UNIQUETITLE);
   nb_err += d2txtanalyser_load_one_txt("Data\\Global\\Excel\\WeaponClass.txt",       WEAPONCLASS);

   // end
   return nb_err;
}


// ========================================================================================
// main function that launch tests on all the .txt
// ========================================================================================
void d2txtanalyser_do_tests(void)
{
   int is_ok = TRUE;


   // primary keys
   printf(
      "==================\n"
      "Check primary keys\n"
      "==================\n"
   );
   if (test_unicity_in_all_txt())
      is_ok = FALSE;
   fflush(stdout);

   // external keys (aka secondary keys)
   printf(
      "\n\n"
      "===================\n"
      "Check external keys\n"
      "===================\n"
   );
   if (test_external_keys())
      is_ok = FALSE;
   fflush(stdout);

   // starting from now : specifics tests

   printf(
      "\n\n"
      "==============\n"
      "Specific tests\n"
      "==============\n"
   );
   // players must have exact same # of skills
   if (test_skills_nb_skills())
      is_ok = FALSE;
   fflush(stdout);

   // test BaseID chains in MonStats.txt
   if (test_monstats_nextinclass())
      is_ok = FALSE;
   fflush(stdout);

   // test Vis & Warps of Levels.txt
   if (test_levels_vis_warps())
      is_ok = FALSE;
   fflush(stdout);

  // test worldspace overlaping
   if (test_levels_worldspace())
      is_ok = FALSE;
   fflush(stdout);

  // test normcode, ubercode and ultracode in armor/weapons
   if (test_awm_norm_uber_ultra())
      is_ok = FALSE;
   fflush(stdout);

  // test number of inputs in CubeMain.txt
   if (test_cubemain_inputs_number())
      is_ok = FALSE;
   fflush(stdout);

   // end
   if (is_ok == TRUE)
   {
      printf("no errors detected\n");
      fprintf(stderr, "no errors detected\n");
   }
   else
      fprintf(stderr, "some errors were detected\n");
   fflush(stdout);
}


// ========================================================================================
// entry point
// ========================================================================================
int main(void)
{
   int  nb_err;
   char * txtinfo_name    = "log_extracted_txt.txt",
        * errorlog_name   = "log_errors.txt",
        * warninglog_name = "log_warnings.txt";


   // init global datas (to zero)
   memset( & glb_datas, 0, sizeof(GLB_DATAS_S));
   glb_datas.str_cmp_func = strcmp;
   txt_init_header_names();

   // prepare the release of our memory
   atexit(d2txtanalyser_exit);

   // display the version
   fprintf(stderr,
          "==========================\n"
          "D2TxtAnalyser, Version %s\n"
          "==========================\n"
          , D2TXTANALYSER_VERSION
   );

   // read ini
   d2txtanalyser_handle_ini();

   // open all mpq
   d2txtanalyser_open_all_mpq();

   // read txt
   fprintf(stderr, "\ncreate file : %s\n", txtinfo_name);
   freopen(txtinfo_name, "w", stdout);
   nb_err = d2txtanalyser_load_txt();
   if (nb_err > 0)
   {
      fprintf(stderr, "\n%i text%s invalid\n",
         nb_err,
         nb_err == 1 ? " is" : "s are"
      );
      return -1;
   }

   // create new error & warning logs
   fprintf(stderr, "create file : %s\n", warninglog_name);
   glb_datas.std_warning = fopen(warninglog_name, "wt");
   if (glb_datas.std_warning == NULL)
   {
      fprintf(stderr, "\nERROR, can't create %s\n", warninglog_name);
      exit(-1);
   }
   freopen(warninglog_name, "w", stdout);

   fprintf(stderr, "create file : %s\n", errorlog_name);
   freopen(errorlog_name, "w", stdout);

   // do tests
   fprintf(stderr, "\nTests in progress...\n");
   d2txtanalyser_do_tests();

   // end
   fprintf(stderr, "\ndone\n");
   return 0;
}
