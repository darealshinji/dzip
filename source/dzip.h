#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "dzip-zlib.h"

typedef unsigned char uchar;

#define MAX_ENT 1024
#define MAJOR_VERSION 2
#define MINOR_VERSION 9
#define INITCRC 0xffffffff

enum { TYPE_NORMAL, TYPE_DEMV1, TYPE_TXT, TYPE_PAK, TYPE_DZ, TYPE_DEM,
	TYPE_NEHAHRA, TYPE_DIR, TYPE_STORE, TYPE_LAST };

enum {
	DEM_bad, DEM_nop, DEM_disconnect, DEM_updatestat, DEM_version,
	DEM_setview, DEM_sound, DEM_time, DEM_print, DEM_stufftext,
	DEM_setangle, DEM_serverinfo, DEM_lightstyle, DEM_updatename,
	DEM_updatefrags, DEM_clientdata, DEM_stopsound, DEM_updatecolors,
	DEM_particle, DEM_damage, DEM_spawnstatic, DEM_spawnbinary,
	DEM_spawnbaseline, DEM_temp_entity, DEM_setpause, DEM_signonnum,
	DEM_centerprint, DEM_killedmonster, DEM_foundsecret,
	DEM_spawnstaticsound, DEM_intermission, DEM_finale,
	DEM_cdtrack, DEM_sellscreen, DEM_cutscene, DZ_longtime,
/* nehahra */
	DEM_showlmp = 35, DEM_hidelmp, DEM_skybox, DZ_showlmp
};

typedef struct {
	uchar voz, pax;
	uchar ang0, ang1, ang2;
	uchar vel0, vel1, vel2;
	long items;
	uchar uk10, uk11, invbit;
	uchar wpf, av, wpm;
	int health;
	uchar am, sh, nl, rk, ce, wp;
	int force;
} cdata_t;

typedef struct { 
	uInt ptr;	/* start of file inside dz */
	uInt size;	/* v1: intermediate size. v2: compressed size */
	uInt real;	/* uncompressed size */
	unsigned short len;	/* length of name */
	unsigned short pak;
	uInt crc;
	uInt type;
	uInt date;
	uInt inter;	/* v2: intermediate size */
	char *name;
} direntry_t;
#define DE_DISK_SIZE 32

typedef struct {
	uchar modelindex, frame;
	uchar colormap, skin;
	uchar effects;
	uchar ang0, ang1, ang2;
	uchar newbit, present, active;
	uchar fullbright;	/* nehahra */
	int org0, org1, org2;
	int od0, od1, od2;
	int force;
	float alpha;		/* nehahra */
} ent_t;

typedef struct {
	char name[56];
	uInt ptr;
	uInt len;
} pakentry_t;

int bplus (int, int);
void copy_msg (uInt);
void create_clientdata_msg (void);
void crc_init (void);
void dem_compress (uInt, uInt);
void dem_copy_ue (void);
uInt dem_uncompress (uInt);
void dem_uncompress_init (int);

/* this file deals with uncompressing files made by
   version 1.x of Dzip and can be omitted without much harm! */
void demv1_clientdata (void);
void demv1_updateentity (void);
void demv1_dxentities (void);

void dzAddFolder (char *);
void dzCompressFile (char *, uInt, uInt);
void dzDeleteFiles  (uInt *, int, void (*)(uInt, uInt));
void dzExtractFile (uInt, int);

/* int is the number of bytes that are already used in
   inblk because they were moved back from the last block
   which is only going to happen for quake demos */
int dzRead (int);

/* read in all the stuff from a dz file, return 0 if not valid */
int dzReadDirectory (char *);

/* read from dz */
void dzFile_Read (void *, uInt);

/* write to dz */
void dzFile_Write (void *, uInt);

/*	get filesize of dz, returns zero if it could not
	be determined, most likely because it was >= 4GB */
uInt dzFile_Size (void);

/* change file pointer of dz */
void dzFile_Seek (uInt);

/* chop off end of dz; this is implementation dependent
   and may need adjusting for other platforms */
void dzFile_Truncate (void);

void dzWrite (void *, int);
void dzWriteDirectory (void);

/* safe set of allocation functions */
void *Dzip_malloc (uInt);
void *Dzip_realloc (void *, uInt);
char *Dzip_strdup (const char *);

void end_zlib_compression (void);

/* does not return if -e was specified on cmd line */
void error (const char *, ...);

/* returns a pointer directly to the period of the extension,
   or it there is none, to the nul character at the end */
char *FileExtension (char *);

int get_filetype (char *);

/* returns pointer to filename from a full path */
char *GetFileFromPath (char *);

/* read from file being compressed */
void Infile_Read (void *, uInt);

/* change file pointer in file being compressed (used for .pak) */
void Infile_Seek (uInt);

/* called when compression was unsuccessful (negative ratio) */
void Infile_Store (uInt);

void insert_msg (void *, uInt);
void make_crc (uchar *, int);
void normal_compress (uInt);

/* write to the file being extracted */
void Outfile_Write (void *, uInt);

#define pakid *(int *)"PACK"
#define discard_msg(x) inptr += x

#ifndef SFXVAR
#define SFXVAR extern
#endif

extern uchar dem_updateframe;
SFXVAR uchar copybaseline;
SFXVAR int maxent, lastent, sble;
extern int maj_ver, min_ver;	/* of the current dz file */
#define p_blocksize 32768
extern int numfiles;
extern uInt totalsize;
SFXVAR int entlink[MAX_ENT];
SFXVAR long dem_gametime;
SFXVAR long outlen;
SFXVAR long cam0, cam1, cam2;
SFXVAR uchar *inblk, *outblk, *inptr;
extern uchar *tmpblk;
extern char AbortOp;
extern uint32_t crcval;
SFXVAR cdata_t oldcd, newcd;
SFXVAR ent_t base[MAX_ENT], oldent[MAX_ENT], newent[MAX_ENT];
extern direntry_t *directory;

#if BYTE_ORDER == BIG_ENDIAN
/* byte swapping on big endian machines */
short getshort (uchar *);
long getlong (uchar *);
float getfloat (uchar *);
#define cnvlong(x) getlong((uchar*)(&x))
#else
#define getshort(x) (*(short*)(x))
#define getlong(x) (*(long*)(x))
#define getfloat(x) (*(float*)(x))
#define cnvlong(x) (x)
#endif

#define Z_BUFFER_SIZE 16384
extern z_stream zs;
extern uchar *zbuf;
extern uInt ztotal;
extern int zlevel;

#ifdef GUI
void GuiProgressMsg(const char *, ...);
#define printf
#define fprintf
#define fflush
#endif

#ifdef WIN32
#define DIRCHAR '\\'
#define WRONGCHAR '/'
#define strcasecmp stricmp
#else
#define DIRCHAR '/'
#define WRONGCHAR '\\'
#endif