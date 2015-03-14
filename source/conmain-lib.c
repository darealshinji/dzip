#include "dzip.h"
#include "dzipcon.h"

char flag[NUM_SWITCHES];
char AbortOp, *dzname;
uInt dzsize;
FILE *infile, *outfile, *dzfile;

/* read from file being compressed */
void Infile_Read (void *buf, uInt num)
{
	if (fread(buf, 1, num, infile) != num)
	{
		error(": error reading from input: %s", strerror(errno));
		AbortOp = 3;
	}
	make_crc(buf, num);
}

/* change file pointer in file being compressed (used for .pak) */
void Infile_Seek (uInt dest)
{
	fseek(infile, dest, SEEK_SET);
}

/* called when compression was unsuccessful (negative ratio) */
void Infile_Store (uInt size)
{
	void *buf = Dzip_malloc(32768);
	int s;

	Infile_Seek(ftell(infile) - size);
	while (size && !AbortOp)
	{
		s = (size > 32768) ? 32768 : size;
		Infile_Read(buf, s);
		dzFile_Write(buf, s);
		size -= s;
	}
	free(buf);
}

/* write to the file being extracted */
void Outfile_Write (void *buf, uInt num)
{
	if (!flag[SW_VERIFY])
	if (fwrite(buf, 1, num, outfile) != num)
	{
		error(": error writing to output: %s", strerror(errno));
		AbortOp = 1;
	}
	make_crc(buf, num);
}

/* read from dz */
void dzFile_Read (void *buf, uInt num)
{
	if (fread(buf, 1, num, dzfile) != num)
	{
		error(": error reading from %s: %s", dzname, strerror(errno));
		AbortOp = 1;
	}
}

/* write to dz */
void dzFile_Write (void *buf, uInt num)
{
	if (fwrite(buf, 1, num, dzfile) != num)
	{
		error(": error writing to %s: %s", dzname, strerror(errno));
		AbortOp = 1;
	}
}

/*	get filesize of dz, returns zero if it could not
	be determined, most likely because it was >= 4GB */
uInt dzFile_Size(void)
{	/* size was determined in DoFiles */
	return dzsize;
}

/* change file pointer of dz */
void dzFile_Seek (uInt dest)
{
#ifdef _MSC_VER
	/* do this so 2-4GB files will work for windows version */
	int hi = 0;
	fseek(dzfile, 0, SEEK_CUR); /* can't just seek out from under crt's nose */
	SetFilePointer(_get_osfhandle(fileno(dzfile)), dest, &hi, FILE_BEGIN);
#else
	fseek(dzfile, dest, SEEK_SET);
#endif
}

/* chop off end of dz; this is implementation dependent
   and may need adjusting for other platforms */
void dzFile_Truncate(void)
{
#ifdef _MSC_VER
	SetEndOfFile(_get_osfhandle(fileno(dzfile)));
#else
	ftruncate(fileno(dzfile), ftell(dzfile));
#endif
}

/* free directory and close current dz */
void dzClose(void)
{
	int i;
	for (i = 0; i < numfiles; free(directory[i++].name));
	free(directory);
	fclose(dzfile);
}

/* open a dz file, return 0 on failure */
int dzOpen (char *src, int needwrite)
{
	dzname = src;
	dzfile = fopen(src, needwrite ? "rb+" : "rb");
	if (!dzfile)
	{
		error(needwrite ? "could not open %s for writing: %s"
			: "could not open %s: %s", src, strerror(errno));
		return 0;
	}

	if (dzReadDirectory(src))
		return 1;

	fclose(dzfile);
	return 0;
}

FILE *open_create (char *src)
{
	FILE *f;

	if (!flag[SW_FORCE])
		if ((f = fopen(src, "r"))) 
		{
			error("%s exists; will not overwrite", src);
			return NULL;
		}

	f = fopen(src,"wb+");
	if (!f) error("could not create %s: %s",src, strerror(errno));
	return f;
}

/* safe set of allocation functions */
void *Dzip_malloc (uInt size)
{
	void *m = malloc(size);
	if (!m)	errorFatal("Out of memory!");
	return m;		
}

void *Dzip_realloc (void *ptr, uInt size)
{
	void *m = realloc(ptr, size);
	if (!m)	errorFatal("Out of memory!");
	return m;		
}

char *Dzip_strdup (const char *str)
{
	char *m = strdup(str);
	if (!m)	errorFatal("Out of memory!");
	return m;		
}

#if BYTE_ORDER == BIG_ENDIAN
/* byte swapping on big endian machines */
short getshort (uchar *c)
{
	return (c[1] << 8) + c[0];
}

long getlong (uchar *c)
{
	return (c[3] << 24) + (c[2] << 16) + (c[1] << 8) + c[0];
}

float getfloat (uchar *c)
{
	return (float)(getlong(c));
}
#endif

/* does not return if -e was specified on cmd line */
void error (const char *msg, ...)
{
	va_list	the_args;
	va_start(the_args,msg);
	if (flag[SW_HALT]) {
		if (*msg == ':')
		{
			fprintf(stderr, "\nERROR: ");
			msg += 2;
		}
		else
			fprintf(stderr,"ERROR: ");
	} /* stupid compilers bitch about ambigious else */

	vfprintf(stderr,msg,the_args);
	fprintf(stderr,"\n");
	va_end(the_args);
	if (flag[SW_HALT])
		exit(1);
}

/* does not return ever */
void errorFatal (const char *msg, ...)
{
	va_list	the_args;
	va_start(the_args,msg);
	fprintf(stderr,"ERROR: ");
	vfprintf(stderr,msg,the_args);
	fprintf(stderr,"\n");
	va_end(the_args);
	exit(1);
}
