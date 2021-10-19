#ifndef _PEAR_
#define _PEAR_
#include "config.h"

#define PROGRAM_NAME            "PEAR"
#define PROGRAM_VERSION         "0.9.11"
#define VERSION_DATE            "Nov 5, 2017"
#define LICENCE                 "Creative Commons Licence"
#define CONTACT                 "Tomas.Flouri@h-its.org and Jiajie.Zhang@h-its.org"

#if (defined(HAVE_BZLIB_H) && defined(HAVE_ZLIB_H))
#define COMPILE_INFO            " - [+bzlib +zlib]"
#elif (defined(HAVE_BZLIB_H) && !defined(HAVE_ZLIB_H))
#define COMPILE_INFO            " - [+bzlib]"
#elif (!defined(HAVE_BZLIB_H) && defined(HAVE_ZLIB_H))
#define COMPILE_INFO            " - [+zlib]"
#else
#define COMPILE_INFO           ""
#endif
void fatal(const char * format, ...) __attribute__ ((noreturn));
void mstrcpl (char * s);
void mstrrev (char * s);
void cmd_stitch(const char * forward, const char * reverse, const char * outfile);
extern char map_nt[256];
extern char revmap_nt[16];
extern char cpl_nt[16];
extern int translate[16];
#endif
