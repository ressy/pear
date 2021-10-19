#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "pear.h"

#define LINEALLOC 2048

static char buffer[LINEALLOC];
static char * line = NULL;
static size_t line_size = 0;
static size_t line_maxsize = 0;

const unsigned int pll_map_fasta[256] =
  {
    /*
    0=stripped, 1=legal, 2=fatal, 3=silently stripped
    @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
    */

/*  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
    2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  2,  2,  /* 0 */
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  /* 1 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  0,  /* 2 */
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  1,  /* 3 */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 4 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  /* 5 */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  0,  /* 6 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  /* 7 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 8 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 9 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* A */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* B */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* C */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* D */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* E */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   /* F */
  };

static void * xmalloc(size_t size)
{
  void * t;
  t = malloc(size);
  if (!t)
    fatal("Unable to allocate enough memory.");

  return t;
}

static FILE * xopen(const char * filename, const char * mode)
{
  FILE * out = fopen(filename, mode);
  if (!out)
    fatal("Cannot open file %s", filename);

  return out;
}

static void reallocline(size_t newmaxsize)
{
  char * temp = (char *)xmalloc((size_t)newmaxsize*sizeof(char));

  if (line)
  {
    memcpy(temp,line,line_size*sizeof(char));
    free(line);
  }
  line = temp;
  line_maxsize = newmaxsize;
}

static char * getnextline(FILE * fp)
{
  size_t len = 0;

  line_size = 0;

  /* read from file until newline or eof */
  while (fgets(buffer, LINEALLOC, fp))
  {
    len = strlen(buffer);

    if (line_size + len > line_maxsize)
      reallocline(line_maxsize + LINEALLOC);

    memcpy(line+line_size,buffer,len*sizeof(char));
    line_size += len;

    if (buffer[len-1] == '\n')
    {
      #if 0
      if (line_size+1 > line_maxsize)
        reallocline(line_maxsize+1);

      line[line_size] = 0;
      #else
        line[line_size-1] = 0;
      #endif

      return line;
    }
  }

  if (!line_size)
  {
    free(line);
    line_maxsize = 0;
    line = NULL;
    return NULL;
  }

  if (line_size == line_maxsize)
    reallocline(line_maxsize+1);

  line[line_size] = 0;
  return line;
}

static void sanity_check(const char * file1, const char * file2)
{
  FILE * fp;
  long lines1 = 0;
  long lines2 = 0;

  fp = xopen(file1,"r");
  while(getnextline(fp))
  {
    lines1++;
  }
  fclose(fp);

  fp = xopen(file2,"r");
  while(getnextline(fp))
    lines2++;
  fclose(fp);

  if (lines1 != lines2)
    fatal("The two files differ in number of lines");

  if (lines1 & 3)
    fatal("Number of lines in files is not a multiple of 4");
}

static void mstrcpl_revert (char * s)
{
  if (!s) return;

  while (*s)
   {  
      /* if the codes were 0,1,2,3,4 it would be
         that simple:
      *s = (*s >> 2) + (~*s & 3);
      */
 
      /* with codes 1,2,3,4,5 it gets nasty: */
      *s = revmap_nt[(int)cpl_nt[(int)map_nt[(int)*s]]];
      /*
      *s = (((*s & 1) & (! (*s & 2))) << 2) |  
           (((!((*s & 4) >> 2) ) & ((*s & 2) >> 1)) << 1 ) | 
           (((!((*s & 4) >> 2)) & ((*s & 2) >> 1) & !(*s & 1)) | 
                (((*s & 4) >> 2) & !((*s & 2) >> 1)));*/
      ++s;
   }
}

static void stitch(const char * filename_forward,
                   const char * filename_reverse,
                   const char * filename_output)
{
  FILE * fp_forward;
  FILE * fp_reverse;
  FILE * fp_output;
  long state = 0;
  long line_count = 0;

  fp_forward = xopen(filename_forward, "r");
  fp_reverse = xopen(filename_reverse, "r");
  fp_output  = xopen(filename_output, "w");

  while (1)
  {
    if (!getnextline(fp_forward))
      break;

    char * p = line;
    char * d = line;

    switch (state)
    {
      case 0:           /* header */
        fprintf(fp_output, "%s\n", line); 
        break;

      case 1:           /* sequence data */
        while(*p)
        {
          unsigned int c = pll_map_fasta[(int)*p];
          if (c == 2)
            fatal("Unknown character (ASCII: %d) at line %ld", *p, line_count);
          else if (c == 1)
          {
            *d = *p;
            ++d;
          }
          ++p;
        }
        *d = 0;
        fprintf(fp_output,"%s",line);
        break;

      case 2:           /* + sign */
        fprintf(fp_output,"%s\n", line);
        break;

      case 3:
        fprintf(fp_output,"%s", line);
        break;

      default:
        fatal("Internal error");
    }

    getnextline(fp_reverse);

    switch (state)
    {
      case 0:
        break;
      case 1:
        while(*p)
        {
          unsigned int c = pll_map_fasta[(int)*p];
          if (c == 2)
            fatal("Unknown character (ASCII: %d) at line %ld", *p, line_count);
          else if (c == 1)
          {
            *d = *p;
            ++d;
          }
          ++p;
        }
        *d = 0;
        mstrrev(line);
        mstrcpl_revert(line);
        fprintf(fp_output,"%s\n",line);
        break;

      case 2:           /* + sign */
        //fprintf("%s", line);
        break;

      case 3:
        mstrrev(line);
        fprintf(fp_output,"%s\n", line);
        break;

      default:
        fatal("Internal error");

    }

    
    state = (state+1) & 3;
    line_count++;
  }

  return; 
}

void cmd_stitch(const char * forward, const char * reverse, const char * output)
{
  sanity_check(forward,reverse);
  fprintf(stdout, "Concatenating reads by reverse-complementing reverse read...\n");
  stitch(forward,reverse,output);
  fprintf(stdout, "Done...\n");
}
