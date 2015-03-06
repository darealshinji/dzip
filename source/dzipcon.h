#ifdef WIN32

#include <windows.h>
#define CreateDir(x) CreateDirectory(x->name, NULL)

#ifdef _MSC_VER
void * _get_osfhandle(int);
#endif

#else

#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

/* adjust if target platform supports large files */
#ifndef sparc
#define stat64 stat
#endif

#define CreateDir(x) mkdir(x->name, 0x1ff)

/* it may be necessary to use this macro instead (mac os X)
#define CreateDir(x) { \
x->name[x->len - 2] = 0; \
mkdir(x->name, 0x1ff); \
x->name[x->len - 2] = DIRCHAR; \
}
*/

#endif

enum  { SW_LIST, SW_EXTRACT, SW_VERIFY,
	SW_FORCE, SW_HALT, SW_ADD, SW_DELETE, NUM_SWITCHES };

void dzAddFile(char *);

/* free directory and close current dz */
void dzClose (void);

/* create list[] array from command prompt files list */
void dzDeleteFiles_MakeList (char **, int);

void dzList (char *);

/* open a dz file, return 0 on failure */
int dzOpen (char *, int);

void dzUncompress (char *);
void dzUncompressV1 (int);

/* does not return ever */
void errorFatal (const char *, ...);

FILE *open_create (char *);

extern FILE *infile, *outfile;
extern char *dzname, flag[];