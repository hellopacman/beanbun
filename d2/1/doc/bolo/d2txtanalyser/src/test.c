#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "error.h"
#include "test.h"
#include "mpq\mpqtypes.h"
#include "globals.h"


// ========================================================================================
// players must have same # of skills
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_skills_nb_skills(void)
{
   TXT_S * txt;
   int   nb_skill[7], is_ok = TRUE, p, y, good_nb;
   char  * str,
         class_code[7][4] = {
            {"ama"}, {"sor"}, {"nec"}, {"pal"}, {"bar"}, {"dru"}, {"ass"}
         };


   txt = & glb_datas.txt[SKILLS];
   if (txt->buffer == NULL)
      return -1;

   // for all player class
   for (p=0; p < 7; p++)
   {
      nb_skill[p] = 0;

      // count how many skills for that player class
      for (y=1; y < txt->nb_rows; y++)
      {
         str = txt->col[SKILLS_CHARCLASS][y];
         if (_stricmp(str, class_code[p]) == 0)
         {
            // this player class use this skill
            nb_skill[p]++;
         }
      }
   }

   good_nb = 0;
   for (p=0; p < 7; p++)
   {
      if (good_nb == 0)
         good_nb = nb_skill[p];
      else if (good_nb != nb_skill[p])
         is_ok = FALSE;
   }
   if (is_ok != TRUE)
   {
      printf("\nIn Skills.txt :\n");
      printf("   * ERROR : some player class have different number of skills\n");
      for (p=0; p < 7; p++)
         printf("   * charclass '%s' has %3i skills\n", class_code[p], nb_skill[p]);
      return -1;
   }

   // end
   return 0;
}


// ========================================================================================
// BaseID & NextInClass must make a chain of correclty linked monsters
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_monstats_nextinclass(void)
{
   TXT_S  * txt;
   int    nb_mon, size, y, is_ok = TRUE, nb_empty_nextinclass, chain_last_row,
          found, nb_chain_elem, r, n, in_error;
   char   * id, * baseid, * baseid2, * nextinclass;
   struct MONCHAIN_S
   {
      int chain_done;
      int prec;
      int next;
   } * monchain;


   txt = & glb_datas.txt[MONSTATS];

   if (txt->col[MONSTATS_ID] == NULL)
      return -1;

   if (txt->col[MONSTATS_BASEID] == NULL)
      return -1;

   if (txt->col[MONSTATS_NEXTINCLASS] == NULL)
      return -1;

   nb_mon = txt->nb_rows;
   if (nb_mon <= 0)
      return -1;

   size = sizeof(struct MONCHAIN_S) * nb_mon;
   monchain = (struct MONCHAIN_S *) malloc(size);
   if (monchain == NULL)
      return -1;
   memset(monchain, 0, size);

   // for all monsters
   in_error = FALSE;
   for (y=1; y < nb_mon; y++)
   {
      baseid = txt->col[MONSTATS_BASEID][y];
      if (monchain[y].chain_done == 1)
         continue;

      // analyse this chain
      nb_empty_nextinclass = 0;
      chain_last_row       = 0;
      nb_chain_elem        = 0;
      for (n=1; n < nb_mon; n++)
      {
         baseid2 = txt->col[MONSTATS_BASEID][n];
         if (glb_datas.str_cmp_func(baseid, baseid2) == 0)
         {
            in_error = FALSE;
            nb_chain_elem++;
            nextinclass = txt->col[MONSTATS_NEXTINCLASS][n];
            if (strlen(nextinclass) == 0)
            {
               nb_empty_nextinclass++;
               chain_last_row = n;
            }
            else
            {
               // search this ID
               found = FALSE;
               for (r = 1; r < nb_mon; r++)
               {
                  id = txt->col[MONSTATS_ID][r];
                  if (glb_datas.str_cmp_func(nextinclass, id) == 0)
                  {
                     monchain[n].next = r;
                     monchain[r].prec = n;
                     found = TRUE;
                     break;
                  }
               }
               if (found == FALSE)
               {
                  // error
                  if (is_ok == TRUE)
                  {
                     printf("\nIn %s :\n", txt->filename);
                     is_ok = FALSE;
                  }
                  printf("   * ERROR : '%s' (%s) at row %i wasn't found in '%s'\n",
                     txt->header[MONSTATS_NEXTINCLASS],
                     nextinclass,
                     n + 1,
                     txt->header[MONSTATS_ID]
                  );
                  in_error = TRUE;
               }
            }
         }
      }

      if (in_error == FALSE)
      {
         // only 1 empty NextInClass ?
         if (nb_empty_nextinclass != 1)
         {
            // error
            if (is_ok == TRUE)
            {
               printf("\nIn %s :\n", txt->filename);
               is_ok = FALSE;
            }
            printf("   * ERROR : chain of '%s' with \"%s\" have more than 1 empty '%s' (%i)\n",
               txt->header[MONSTATS_BASEID],
               baseid,
               txt->header[MONSTATS_NEXTINCLASS],
               nb_empty_nextinclass
            );
         }
         else
         {
            // follow the chain in reversed order and count how many elements in it
            n = 0;
            r = chain_last_row;
            while (r != 0)
            {
               n++;
               r = monchain[r].prec;
            }
            if (n != nb_chain_elem)
            {
               // error
               if (is_ok == TRUE)
               {
                  printf("\nIn %s :\n", txt->filename);
                  is_ok = FALSE;
               }
               printf(
                  "   * ERROR : chain of '%s' with \"%s\" is broken somehow (found %i elements, expected %i)\n",
                  txt->header[MONSTATS_BASEID],
                  baseid,
                  n,
                  nb_chain_elem
               );
            }
         }
      }

      // this chain is done
      for (n=1; n < nb_mon; n++)
      {
         baseid2 = txt->col[MONSTATS_BASEID][n];
         if (glb_datas.str_cmp_func(baseid, baseid2) == 0)
            monchain[n].chain_done = 1;
      }
   }

   // end
   free(monchain);
   return is_ok == TRUE ? 0 : -1;
}


// ========================================================================================
// Levels must be linked by at least 1 Vis, and each Vis should have a Warp
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_levels_vis_warps(void)
{
   TXT_S * txt, * txt_warp;
   int   vis_id[8] = {
            LEVELS_VIS0, LEVELS_VIS1, LEVELS_VIS2, LEVELS_VIS3,
            LEVELS_VIS4, LEVELS_VIS5, LEVELS_VIS6, LEVELS_VIS7
         },
         warp_id [8] = {
            LEVELS_WARP0, LEVELS_WARP1, LEVELS_WARP2, LEVELS_WARP3,
            LEVELS_WARP4, LEVELS_WARP5, LEVELS_WARP6, LEVELS_WARP7
         },
         y, r, n, id, vis, i, y2, n2, id2, vis2, warp, warp2,
         is_ok = TRUE, is_ok_warn = TRUE, found, valid_vis;
   char  * str;


   txt_warp = & glb_datas.txt[LVLWARP];
   txt      = & glb_datas.txt[LEVELS];
   for (i=0; i < 8; i++)
   {
      if (txt->col[ vis_id[i] ] == NULL)
         return -1;
      if (txt->col[ warp_id[i] ] == NULL)
         return -1;
   }
   if (txt->col[LEVELS_NAME] == NULL)
      return -1;
   if (txt_warp->col[LVLWARP_ID] == NULL)
      return -1;
   if (txt_warp->col[LVLWARP_NAME] == NULL)
      return -1;

   // for all levels
   for (y = 2; y < txt->nb_rows; y++)
   {
      // get ID
      str = txt->col[LEVELS_ID][y];
      id  = atoi(str);

      // for all Vis
      for (n=0; n < 8; n++)
      {
         str = txt->col[ vis_id[n] ][y];
         vis = atoi(str);
         if (vis >= 1)
         {
            // is it the same ID as current ?
            if (vis == id)
            {
               // error
               if (is_ok == TRUE)
               {
                  printf("\nIn %s :\n", txt->filename);
                  is_ok = FALSE;
               }
               printf(
                  "   * ERROR : '%s' (%i) at row %i (%s) use the Level ID where it is\n",
                  txt->header[ vis_id[n] ],
                  vis,
                  y + 1,
                  txt->col[LEVELS_NAME][y]
               );
            }
            else
            {
               // 1) test if it has a correct warp
               str  = txt->col[ warp_id[n] ][y];
               warp = atoi(str);
               if (warp < 0)
               {
                  // error
                  if (is_ok_warn == TRUE)
                  {
                     fprintf(glb_datas.std_warning, "\nIn %s :\n", txt->filename);
                     is_ok_warn = FALSE;
                  }
                  fprintf(
                     glb_datas.std_warning,
                     "   * WARNING : '%s' (%i) at row %i (%s) should use a Warp ID (not -1)\n",
                     txt->header[ warp_id[n] ],
                     warp,
                     y + 1,
                     txt->col[LEVELS_NAME][y]
                  );
               }
               else
               {
                  // is it a valid warp ID ?
                  found = FALSE;
                  for (r=1; r < txt_warp->nb_rows; r++)
                  {
                     str   = txt_warp->col[LVLWARP_ID][r];
                     warp2 = atoi(str);
                     if (warp == warp2)
                     {
                        found = TRUE;
                        break;
                     }
                  }
                  if (found == FALSE)
                  {
                     // error
                     if (is_ok == TRUE)
                     {
                        printf("\nIn %s :\n", txt->filename);
                        is_ok = FALSE;
                     }
                     printf(
                        "   * ERROR : '%s' (%i) at row %i (%s) use a warp ID which don't exists in LvlWarp.txt\n",
                        txt->header[ warp_id[n] ],
                        warp,
                        y + 1,
                        txt->col[LEVELS_NAME][y]
                     );
                  }
               }

               // 2) is the other level linked back to ourselves ?

               // 2A) for all levels
               valid_vis = FALSE;
               for (y2 = 2; y2 < txt->nb_rows; y2++)
               {
                  if (y == y2)
                     continue;
                  // get ID
                  str = txt->col[LEVELS_ID][y2];
                  id2 = atoi(str);
                  if (id2 == vis)
                  {
                     valid_vis = TRUE;

                     // for all Vis of this 2nd level
                     found = FALSE;
                     for (n2 = 0;  n2 < 8; n2++)
                     {
                        str  = txt->col[ vis_id[n2] ][y2];
                        vis2 = atoi(str);
                        if (vis2 == id)
                        {
                           found = TRUE;
                           break;
                        }
                     }
                     if (found == FALSE)
                     {
                        // error
                        if (is_ok == TRUE)
                        {
                           printf("\nIn %s :\n", txt->filename);
                           is_ok = FALSE;
                           break;
                        }
                        printf(
                           "   * ERROR : Level %s %i (%s) at row %i "
                           "is not linked to Level %s %i (%s) at row %i\n",
                           txt->header[LEVELS_ID],
                           id2,
                           txt->col[LEVELS_NAME][y2],
                           y2 + 1,
                           txt->header[LEVELS_ID],
                           id,
                           txt->col[LEVELS_NAME][y],
                           y + 1
                        );
                     }
                     else
                        break;
                  }
               }

               // 2B)
               if (valid_vis == FALSE)
               {
                  // error
                  if (is_ok == TRUE)
                  {
                     printf("\nIn %s :\n", txt->filename);
                     is_ok = FALSE;
                  }
                  printf(
                     "   * ERROR : '%s' (%i) at row %i is an ID that is not in the Levels ID\n",
                     txt->header[ vis_id[n] ],
                     vis,
                     y + 1
                  );
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
// Levels should not overlap each other within the same act of 1 difficulty
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_levels_worldspace(void)
{
   TXT_S * txt;
   int   sizex_id[3] = {LEVELS_SIZEX, LEVELS_SIZEXN, LEVELS_SIZEXH},
         sizey_id[3] = {LEVELS_SIZEY, LEVELS_SIZEYN, LEVELS_SIZEYH};
   int   is_ok = TRUE, is_ok_warn = TRUE, d, a, y, y2,
         act, sizex, sizey,
         offsetx,  offsety,  bottomx,  bottomy,
         offsetx2, offsety2, bottomx2, bottomy2,
         * overlapped;


   txt = & glb_datas.txt[LEVELS];
   if (txt->col[LEVELS_SIZEX]  == NULL)
      return -1;
   if (txt->col[LEVELS_SIZEXN] == NULL)
      return -1;
   if (txt->col[LEVELS_SIZEXH] == NULL)
      return -1;
   if (txt->col[LEVELS_SIZEY]  == NULL)
      return -1;
   if (txt->col[LEVELS_SIZEYN] == NULL)
      return -1;
   if (txt->col[LEVELS_SIZEYH] == NULL)
      return -1;
   if (txt->col[LEVELS_NAME]   == NULL)
      return -1;
   if (txt->col[LEVELS_ACT]    == NULL)
      return -1;
   if (txt->col[LEVELS_ID]     == NULL)
      return -1;

   y = sizeof(int) * txt->nb_rows;
   overlapped = (int *) malloc(y);
   if (overlapped == NULL)
      return -1;
   memset(overlapped, 0, y);

   // for all acts
   for (a=0; a < 5; a++)
   {
      // for all levels of this act
      for (y = 2; y < txt->nb_rows; y++)
      {
         if (overlapped[y] == 1)
            continue;
         if (atoi(txt->col[LEVELS_ID][y]) == 0)
            continue;

         act = atoi(txt->col[LEVELS_ACT][y]);
         if (act == a)
         {
            offsetx = atoi(txt->col[LEVELS_OFFSETX][y]);
            offsety = atoi(txt->col[LEVELS_OFFSETY][y]);
            if ((offsetx <= -1) || (offsety <= -1))
               continue;

            // for all difficulties
            for (d=0; d < 3; d++)
            {
               sizex   = atoi(txt->col[ sizex_id[d] ][y]);
               sizey   = atoi(txt->col[ sizey_id[d] ][y]);
               bottomx = offsetx + sizex - 1;
               bottomy = offsety + sizey - 1;

               // we have the coordinate of this level
               // check if it don't overlap with another of same act and difficulty
               for (y2 = 2; y2 < txt->nb_rows; y2++)
               {
                  if (y2 == y)
                     continue;
                  if (atoi(txt->col[LEVELS_ACT][y2]) != act)
                     continue;
                  if (atoi(txt->col[LEVELS_ID][y2]) == 0)
                     continue;

                  offsetx2 = atoi(txt->col[LEVELS_OFFSETX][y2]);
                  offsety2 = atoi(txt->col[LEVELS_OFFSETY][y2]);
                  if ((offsetx2 <= -1) || (offsety2 <= -1))
                     continue;

                  sizex    = atoi(txt->col[ sizex_id[d] ][y2]);
                  sizey    = atoi(txt->col[ sizey_id[d] ][y2]);
                  bottomx2 = offsetx2 + sizex - 1;
                  bottomy2 = offsety2 + sizey - 1;

                  // test overlaping
                  if ( ((offsety < offsety2) && (bottomy < offsety2)) ||
                       ((offsety > bottomy2) && (bottomy > bottomy2)) ||
                       ((offsetx < offsetx2) && (bottomx < offsetx2)) ||
                       ((offsetx > bottomx2) && (bottomx > bottomx2)))
                  {
                     // ok
                  }
                  else
                  {
                     // error
                     if (is_ok_warn == TRUE)
                     {
                        fprintf(glb_datas.std_warning, "\nIn %s :\n", txt->filename);
                        is_ok_warn = FALSE;
                     }
                     fprintf(
                        glb_datas.std_warning,
                        "   * WARNING : Level %s %i (%s) at row %i overlap\n"
                        "               level %s %i (%s) at row %i in %s difficulty\n\n",
                        txt->header[LEVELS_ID],
                        atoi(txt->col[LEVELS_ID][y]),
                        txt->col[LEVELS_NAME][y],
                        y + 1,
                        txt->header[LEVELS_ID],
                        atoi(txt->col[LEVELS_ID][y2]),
                        txt->col[LEVELS_NAME][y2],
                        y2 + 1,
                        d == 0 ? "Normal" : d == 1 ? "Nightmare" : "Hell"
                     );
                     overlapped[y2] = 1;
                  }
               }               
            }
         }
      }
   }

   // end
   free(overlapped);
   if ((is_ok == TRUE) && (is_ok_warn == TRUE))
      return 0;
   return -1;
}


// ========================================================================================
// Test Norm Uber and Ultra codes in Armor.txt and Weapons.txt
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_awm_norm_uber_ultra(void)
{
   TXT_S * txt;
   int   is_ok = TRUE, y, n, t;
   char  * code,  * norm,  * uber,  * ultra,
         * code2, * norm2, * uber2, * ultra2;


   // for Armor.txt, then Weapons.txt
   for (t=0; t < 2; t++)
   {
      if (t == 0)
         txt = & glb_datas.txt[ARMOR];
      else
         txt = & glb_datas.txt[WEAPONS];

      if (txt->col[AWM_NAME] == NULL)
         return -1;
      if (txt->col[AWM_CODE] == NULL)
         return -1;
      if (txt->col[AWM_NORMCODE] == NULL)
         return -1;
      if (txt->col[AWM_UBERCODE] == NULL)
         return -1;
      if (txt->col[AWM_ULTRACODE] == NULL)
         return -1;
   
      is_ok = TRUE;

      // for all items 
      for (y=1; y < txt->nb_rows; y++)
      {
         code  = txt->col[AWM_CODE][y];
         norm  = txt->col[AWM_NORMCODE][y];
         uber  = txt->col[AWM_UBERCODE][y];
         ultra = txt->col[AWM_ULTRACODE][y];

         if (strlen(code) == 0)
            continue;

         // is the code in norm/uber/ultra ?
         if (strcmp(code, norm) != 0)
         {
            if (strcmp(code, uber) != 0)
            {
               if (strcmp(code, ultra) != 0)
               {
                  // error
                  if (is_ok == TRUE)
                  {
                     printf("\nIn %s :\n", txt->filename);
                     is_ok = FALSE;
                  }
                  printf(
                     "   * ERROR : '%s' (%s) at row %i (%s) is "
                     "not used in Norm/Uber/Ultra code columns\n",
                     txt->header[AWM_CODE],
                     code,
                     y + 1,
                     txt->col[AWM_NAME][y]
                  );
                  continue;
               }
            }
         }

         // check all items which use this Norm/Uber/Ultra codes
         for (n=1; n < txt->nb_rows; n++)
         {
            if (y == n)
               continue;

            code2 = txt->col[AWM_CODE][n];
            if (strlen(code2) == 0)
               continue;

            if ( (strcmp(code2, norm)  == 0) ||
                 (strcmp(code2, uber)  == 0) ||
                 (strcmp(code2, ultra) == 0)
               )
            {
               norm2  = txt->col[AWM_NORMCODE][n];
               uber2  = txt->col[AWM_UBERCODE][n];
               ultra2 = txt->col[AWM_ULTRACODE][n];

               // check if all same NormCode
               if (strcmp(norm2, norm) != 0)
               {
                  // error
                  if (is_ok == TRUE)
                  {
                     printf("\nIn %s :\n", txt->filename);
                     is_ok = FALSE;
                  }
                  printf(
                     "   * ERROR : Item '%s' (%s) at row %i (%s) "
                     "has '%s' (%s), expected (%s)\n",
                     txt->header[AWM_CODE],
                     code2,
                     n + 1,
                     txt->col[AWM_NAME][n],
                     txt->header[AWM_NORMCODE],
                     norm2,
                     norm
                  );
               }

               // check if all same UberCode
               if (strcmp(uber2, uber) != 0)
               {
                  // error
                  if (is_ok == TRUE)
                  {
                     printf("\nIn %s :\n", txt->filename);
                     is_ok = FALSE;
                  }
                  printf(
                     "   * ERROR : Item '%s' (%s) at row %i (%s) "
                     "has '%s' (%s), expected (%s)\n",
                     txt->header[AWM_CODE],
                     code2,
                     n + 1,
                     txt->col[AWM_NAME][n],
                     txt->header[AWM_UBERCODE],
                     uber2,
                     uber
                  );
               }

               // check if all same UltraCode
               if (strcmp(ultra2, ultra) != 0)
               {
                  // error
                  if (is_ok == TRUE)
                  {
                     printf("\nIn %s :\n", txt->filename);
                     is_ok = FALSE;
                  }
                  printf(
                     "   * ERROR : Item '%s' (%s) at row %i (%s) "
                     "has '%s' (%s), expected (%s)\n",
                     txt->header[AWM_CODE],
                     code2,
                     n + 1,
                     txt->col[AWM_NAME][n],
                     txt->header[AWM_ULTRACODE],
                     ultra2,
                     ultra
                  );
               }
            }
         }
      }
   }

   // end
   if (is_ok == TRUE)
      return 0;
   return -1;
}


// ========================================================================================
// Test numbers of inputs in CubeMain.txt
// ouput :
//    0 if ok, non-zero if error
// ========================================================================================
int test_cubemain_inputs_number(void)
{
   TXT_S * txt;
   int   is_ok = TRUE, y, nb_input, i, sum, length, in_error, n,
         input_id[7] = {
            CUBEMAIN_INPUT1, CUBEMAIN_INPUT2, CUBEMAIN_INPUT3, CUBEMAIN_INPUT4,
            CUBEMAIN_INPUT5, CUBEMAIN_INPUT6, CUBEMAIN_INPUT7
         };
   char  * str, * qty_ptr, * buffer = glb_datas.strtmp, * desc;


   txt = & glb_datas.txt[CUBEMAIN];

   if (txt->col[CUBEMAIN_DESCRIPTION] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_NUMINPUTS] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT1] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT2] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT3] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT4] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT5] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT6] == NULL)
      return -1;
   if (txt->col[CUBEMAIN_INPUT7] == NULL)
      return -1;

   // for all formulas
   for (y=1; y < txt->nb_rows; y++)
   {
      desc = txt->col[CUBEMAIN_DESCRIPTION][y];

      nb_input = atoi(txt->col[CUBEMAIN_NUMINPUTS][y]);
      if (nb_input < 2)
      {
         // error
         if (is_ok == TRUE)
         {
            printf("\nIn %s :\n", txt->filename);
            is_ok = FALSE;
         }
         printf(
            "   * ERROR : Formula at row %i should have '%s' >= 2\n"
            "             (%s)\n\n",
            y + 1,
            txt->header[CUBEMAIN_NUMINPUTS],
            desc
         );
      }

      // compute sum of inputs used
      sum = 0;
      in_error = FALSE;
      for (i=0; i < 7; i++)
      {
         str  = txt->col[ input_id[i] ][y];
         if (strlen(str) == 0)
            break;

         qty_ptr = strstr(str, "qty=");
         if (qty_ptr != NULL)
         {
            // get quantity
            strcpy(buffer, qty_ptr + 4);
            length = strlen(buffer);
            if (length == 0)
            {
               // error
               if (is_ok == TRUE)
               {
                  printf("\nIn %s :\n", txt->filename);
                  is_ok = FALSE;
               }
               printf(
                  "   * ERROR : '%s' (%s) at row %i use 'qty=' but there's no value\n"
                  "             (%s)\n\n",
                  txt->header[ input_id[i] ],
                  str,
                  y + 1,
                  desc
               );
               in_error = TRUE;
               continue; // next input column
            }
            for (n=0; n < length; n++)
            {
               if ((buffer[n] < '0') || (buffer[n] > '9'))
               {
                  buffer[n] = 0x00;
                  break;
               }
            }

            length = strlen(buffer);
            if (length == 0)
            {
               // error
               if (is_ok == TRUE)
               {
                  printf("\nIn %s :\n", txt->filename);
                  is_ok = FALSE;
               }
               printf(
                  "   * ERROR : '%s' (%s) at row %i use 'qty=' but there's no value\n"
                  "             (%s)\n\n",
                  txt->header[ input_id[i] ],
                  str,
                  y + 1,
                  desc
               );
               in_error = TRUE;
               continue; // next input column
            }

            n = atoi(buffer);
            if (n < 1)
            {
               // error
               if (is_ok == TRUE)
               {
                  printf("\nIn %s :\n", txt->filename);
                  is_ok = FALSE;
               }
               printf(
                  "   * ERROR : '%s' (%s) at row %i should have 'qty=' value >= 1\n"
                  "             (%s)\n\n",
                  txt->header[ input_id[i] ],
                  str,
                  y + 1,
                  desc
               );
               in_error = TRUE;
               continue; // next input column
            }
            sum += n;
         }
         else
         {
            // count as 1
            sum++;
         }
      }
      if (in_error == TRUE)
         continue;

      if (nb_input != sum)
      {
         // error
         if (is_ok == TRUE)
         {
            printf("\nIn %s :\n", txt->filename);
            is_ok = FALSE;
         }
         printf(
            "   * ERROR : Formula at row %i has invalid number of inputs\n"
            "             (%s)\n\n",
            y + 1,
            desc
         );
      }
   }

   // end
   if (is_ok == TRUE)
      return 0;
   return -1;
}
