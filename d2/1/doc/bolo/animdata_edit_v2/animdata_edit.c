#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char UBYTE;


// ----------------------------------------------------------------------
#pragma pack(1) // no padding bytes between members of a struct for now
// ----------------------------------------------------------------------
typedef struct
{
   char  cof_name[8];    // 7 chars + 1 zero termination (0x00)
   long  frames_per_dir; // frames per direction (0 ~ 256 normally)
   long  anim_speed;     // animation speed, in 256th of 25fps. 128=50% of 25fps = 12.5fps
   UBYTE frame_tag[144]; // tag value for each frame

   UBYTE hash;           // NOT present in AnimData.d2 !
} ANIMDATA_RECORD_S;
// ----------------------------------------------------------------------
#pragma pack()  // cancel the previous "no padding bytes" command
// ----------------------------------------------------------------------

typedef struct ANIMDATA_LINK_S
{
   struct ANIMDATA_LINK_S * next;
   ANIMDATA_RECORD_S      * record;
} ANIMDATA_LINK_S;

ANIMDATA_LINK_S    * animdata_hash[256];
ANIMDATA_LINK_S   ** animdata_sorted_name = NULL;
ANIMDATA_RECORD_S ** animdata_records = NULL;


// ==========================================================================
void animdata_edit_init(void)
{
   memset(animdata_hash, 0, sizeof(animdata_hash));
}


// ==========================================================================
void animdata_edit_atexit(void)
{
   ANIMDATA_LINK_S * link, * next;
   int i;


   for (i=0; i < 256; i++)
   {
      link = animdata_hash[i];
      while (link != NULL)
      {
         next = link->next;
         free(link);
         link = next;
      }
   }

   if (animdata_sorted_name != NULL)
      free(animdata_sorted_name);

   if (animdata_records != NULL)
      free(animdata_records);
}


// ==========================================================================
void * load_file_in_mem(char * name, long * ptr_size)
{
   FILE * in;
   void * buffer;


   in = fopen(name, "rb");
   if (in == NULL)
   {
      printf("can't open \"%s\"\n", name);
      return NULL;
   }
   fseek(in, 0, SEEK_END);
   * ptr_size = ftell(in);
   fseek(in, 0, SEEK_SET);

   // buffer
   buffer = (void *) malloc(* ptr_size);
   if (buffer == NULL)
   {
      fclose(in);
      printf("can't allocate %li bytes for copying \"%s\" in memory\n", 
         * ptr_size,
         name
      );
      return NULL;
   }

   // read
   if (fread(buffer, * ptr_size, 1, in) != 1)
   {
      free(buffer);
      fclose(in);
      printf("Disk-error while reading \"%s\"\n", name);
      printf("Maybe the file is still open into another application ?\n");
      return NULL;
   }

   // end
   fclose(in);
   return buffer;
}


// ==========================================================================
void insert_record(int hash, ANIMDATA_RECORD_S * record)
{
   ANIMDATA_LINK_S * new_link;


   if ((hash < 0) || (hash > 255))
      return;

   new_link = (ANIMDATA_LINK_S *) malloc(sizeof(ANIMDATA_LINK_S));
   if (new_link == NULL)
   {
      printf("can't allocate %i bytes for adding record '%.7s' to the hash list # %i\n",
         sizeof(ANIMDATA_LINK_S),
         record->cof_name,
         hash
      );
      return;
   }
   new_link->record    = record;
   new_link->next      = animdata_hash[hash];
   animdata_hash[hash] = new_link;
}


// ==========================================================================
int animdata_edit_cmp(const void * arg1, const void * arg2)
{
   ANIMDATA_LINK_S * link1 = (ANIMDATA_LINK_S *) * ((ANIMDATA_LINK_S **) arg1),
                   * link2 = (ANIMDATA_LINK_S *) * ((ANIMDATA_LINK_S **) arg2);
   char            * str1 = link1->record->cof_name,
                   * str2 = link2->record->cof_name;


   return stricmp(str1, str2);
}


// ==========================================================================
long animdata_sort_name(void)
{
   ANIMDATA_LINK_S * link;
   long            nb_rec, size, x;
   int             i;


   // count nb records
   nb_rec = 0;
   for (i=0; i < 256; i++)
   {
      link = animdata_hash[i];
      while (link != NULL)
      {
         nb_rec++;
         link = link->next;
      }
   }

   // malloc
   size = sizeof(ANIMDATA_LINK_S *) * nb_rec;
   animdata_sorted_name = (ANIMDATA_LINK_S **) malloc(size);
   if (animdata_sorted_name == NULL)
   {
      printf("can't allocate %li bytes for animdata_sorted_name\n", size);
      return 0;
   }
   memset(animdata_sorted_name, 0, size);

   // prepare the sort
   x = 0;
   for (i=0; i < 256; i++)
   {
      link = animdata_hash[i];
      while (link != NULL)
      {
         animdata_sorted_name[x] = link;
         x++;
         link = link->next;
      }
   }

   // sort them
   qsort(animdata_sorted_name, nb_rec, sizeof(ANIMDATA_LINK_S *), animdata_edit_cmp);

   // end
   return nb_rec;
}


// ==========================================================================
void make_txt(char * d2name)
{
   ANIMDATA_RECORD_S * record;
   ANIMDATA_LINK_S   * link;
   UBYTE             * buffer;
   long              size, * dw_ptr, nb_rec, cursor, r;
   int               i, x;
   char              * txtname = "AnimData.txt";
   FILE              * out;


   buffer = (UBYTE *) load_file_in_mem(d2name, & size);
   if (buffer == NULL)
   {
      return;
   }

   if (size == 0)
   {
      printf("\"%s\" is empty\n", d2name);
      return;
   }

   // read animdata.d2 and fill animdata_hash[] lists
   cursor = 0;
   for (i=0; i < 256; i++)
   {
      if ((cursor + 4) <= size) // nb_rec require 4 bytes
      {
         dw_ptr = (long *) (buffer + cursor);
         nb_rec = * dw_ptr;
         cursor += 4;
         for (r=0; r < nb_rec; r++)
         {
            // read a record
            if ((cursor + 160) <= size) // a record is 160 bytes
            {
               record = (ANIMDATA_RECORD_S *) (buffer + cursor);
               insert_record(i, record);
               cursor += 160;
            }
         }
      }
   }

   // write txt datas
   out = fopen(txtname, "wt");
   if (out == NULL)
   {
      freopen("error.txt", "wt", stdout);
      printf("can't open %s\n", txtname);
      exit(1);
   }
   fputs("CofName"
         "\tFramesPerDirection"
         "\tAnimationSpeed"
         "\tFrameData000",
         out
   );
   for (i=1; i <= 143; i++)
      fprintf(out, "\tFrameData%03i", i);
   fprintf(out, "\n");

   // sort cof names
   nb_rec = animdata_sort_name();

   // write them
   for (r=0; r < nb_rec; r++)
   {
      link = animdata_sorted_name[r];
      fprintf(out, "%.7s",  link->record->cof_name);
      fprintf(out, "\t%li", link->record->frames_per_dir);
      fprintf(out, "\t%li", link->record->anim_speed);
      for (x=0; x < 144; x++)
         fprintf(out, "\t%i", link->record->frame_tag[x]);
      fprintf(out, "\n");
   }

   // end
   fclose(out);
   free(buffer);
   printf("%li records writen into %s\n", nb_rec, txtname);
}


// ==========================================================================
void make_d2(void)
{
   ANIMDATA_RECORD_S * record;
   FILE              * in, * out;
   char              txtname[] = "AnimData.txt", d2name[] = "AnimData.d2";
   int               c, i, x;
   unsigned long     udword;
   unsigned char     ubyte;
   long              pos, r, size, nb_rec, records_max = 0;


   // read txt datas
   in = fopen(txtname, "rb");
   if (in == NULL)
   {
      freopen("error.txt", "wt", stdout);
      printf("can't open %s\n", txtname);
      exit(1);
   }

   // skip 1st line
   c = fgetc(in);
   while ((c != EOF) && ((c != 0x0D) && (c != 0x0A)))
      c = fgetc(in);
   while ((c == 0x0D) || (c == 0x0A))
      c = fgetc(in);

   // count # of records
   x = c;
   pos = ftell(in);
   nb_rec = 0;
   while (c != EOF)
   {
      nb_rec++;
      c = fgetc(in);
      while ((c != EOF) && ((c != 0x0D) && (c != 0x0A)))
         c = fgetc(in);
      while ((c == 0x0D) || (c == 0x0A))
         c = fgetc(in);
   }
   fseek(in, pos, SEEK_SET);
   c = x;
   records_max = nb_rec;

   // malloc for table of pointers to records
   size = sizeof(ANIMDATA_RECORD_S *) * records_max;
   animdata_records = (ANIMDATA_RECORD_S **) malloc(size);
   if (animdata_records == NULL)
   {
      printf("can't allocate %li bytes for animdata_records\n", size);
      return;
   }
   memset(animdata_records, 0, size);

   // for all records
   r = 0;
   while (c != EOF)
   {
      // create a new record
      size = sizeof(ANIMDATA_RECORD_S);
      record = (ANIMDATA_RECORD_S *) malloc(size);
      if (record == NULL)
      {
         printf("can't allocate %li bytes for animdata_records[%i]\n", size, r);
         return;
      }
      memset(record, 0, size);
      animdata_records[r] = record;
      r++;

      // cof name
      x = 0;
      for (i=0; i<7; i++)
      {
         record->cof_name[i] = (UBYTE) c;
         x += toupper(c);
         c = fgetc(in);
      }
      record->hash = (UBYTE) (x & 0xFF);

      // frame per dir
      fscanf(in, "%li", & udword);
      record->frames_per_dir = udword;

      // animation speed
      fscanf(in, "%li", & udword);
      record->anim_speed = udword;

      // frame datas
      for (i=0; i < 144; i++)
      {
         fscanf(in, "%i", & ubyte);
         record->frame_tag[i] = ubyte;
      }
      
      // next record
      c = fgetc(in);
      while ((c == 0x0D) || (c == 0x0A))
         c = fgetc(in);
   }
   fclose(in);

   // make animdata.d2

   out = fopen(d2name, "wb");
   if (out == NULL)
   {
      freopen("error.txt", "wt", stdout);
      printf("can't open for writing %s\n", d2name);
      exit(1);
   }

   for(x=0; x < 256; x++)
   {
      // # of records in this block
      nb_rec = 0;
      for (r=0; r < records_max; r++)
      {
         record = animdata_records[r];
         if (record->hash == x)
            nb_rec++;
      }
      fwrite( & nb_rec, 4, 1, out);

      // write records
      for (r=0; r < records_max; r++)
      {
         record = animdata_records[r];
         if (record->hash == x)
         {
            fwrite(record, 160, 1, out);
         }
      }
   }

   // end
   fclose(out);
   printf("%li records writen into %s\n\n", records_max, d2name);
   fflush(stdout);

   // check for duplicated COF
   for (r=0; r < records_max; r++)
      animdata_records[r]->hash = 0;
   for (r=0; r < records_max - 1; r++)
   {
      record = animdata_records[r];
      if (stricmp(record->cof_name, animdata_records[r+1]->cof_name) == 0)
      {
         if (record->hash == 0)
         {
            printf("warning : '%.7s' is present more than once in %s\n",
               animdata_records[r]->cof_name,
               d2name
            );
            record->hash = 1;
            for (i = r + 1; i < records_max; i++)
            {
               if (stricmp(record->cof_name, animdata_records[i]->cof_name) == 0)
                  animdata_records[i]->hash = 1;
            }
         }
      }
   }
   fflush(stdout);

   for (r=0; r < records_max; r++)
      free(animdata_records[r]);
}


// ==========================================================================
int main(int argc, char ** argv)
{
   printf("animdata_edit v. 2\n");
   fflush(stdout);
   if (argc != 2)
   {
      freopen("error.txt", "wt", stdout);
      printf("syntaxe : animdata_edit <file>\n"
             "   <file> is either animdata.d2 OR animdata.txt\n");
      exit(0);
   }
   
   animdata_edit_init();
   atexit(animdata_edit_atexit);

   if (stricmp(argv[1], "animdata.d2") == 0)
      make_txt(argv[1]);
   else if (stricmp(argv[1], "animdata.txt") == 0)
      make_d2();
   else
   {
      freopen("error.txt", "wt", stdout);
      printf("can't process %s : not animdata.d2 nor animdata.txt", argv[1]);
      exit(1);
   }
   return 0;
}
