#include "dzip.h"
#include "dzipcon.h"

char flag[NUM_SWITCHES];
char AbortOp, *dzname;
uInt dzsize;
FILE *infile, *outfile, *dzfile;

void *Dzip_calloc (uInt size)
{
	return memset(Dzip_malloc(size), 0, size);
}

struct { char *name; int flag; }
optname[] =
	{ { "-l", SW_LIST },
	  { "-x", SW_EXTRACT },
	  { "-v", SW_VERIFY },
	  { "-a", SW_ADD },
	  { "-d", SW_DELETE },
	  { "-f", SW_FORCE },
	  { "-e", SW_HALT },
	  { NULL, 0 }
	};

void parse_switch (char *arg)
{
	int i;

	if (strlen(arg) == 2 && arg[1] >= '0' && arg[1] <= '9')
	{
		zlevel = arg[1] - '0';
		return;
	}

	for (i = 0; optname[i].name; i++)
		if (!strcmp(optname[i].name,arg))
		{
			flag[optname[i].flag]++;
			return;
		}
	errorFatal("unknown switch %s",arg);
}

void usage(void)
{
    fprintf(stderr,
	"Dzip v%u.%u: specializing in Quake demo compression\n\n"

	"Compression:         dzip <filenames> [-o <outputfile>]\n"
	"Add to existing dz:  dzip -a <dzfile> <filenames>\n"
	"Delete from a dz:    dzip -d <dzfile> <filenames>\n"
	"Decompression:       dzip -x <filenames>\n"
	"Verify dz file:      dzip -v <filenames>\n"
	"List dz file:        dzip -l <filenames>\n\n"

	"Options:\n"
	"  -0 to -9: set compression level\n"
	"  -e: quit program on first error\n"
	"  -f: overwrite existing files\n\n"

	"Copyright 2000-2002 Stefan Schwoon, Nolan Pflug\n",
	MAJOR_VERSION, MINOR_VERSION
    );

    exit(1);
}

void DoFiles (char *fname, void (*func)(char *));

void DoDirectory (char *dname)
{
	char fullname[260], *ptr;

	if (!strcmp(dname, ".") || !strcmp(dname, ".."))
		return;
	ptr = &fullname[sprintf(fullname, "%s\\", dname)];
	dzAddFolder(fullname);

	/* scan directory, more system specific stuff */
	{
#ifdef WIN32
	HANDLE f;
	WIN32_FIND_DATA fd;

	*(short *)ptr = '*';
	f = FindFirstFile(fullname, &fd);
	if (f == INVALID_HANDLE_VALUE)
		return;	/* happens if user doesn't have browse permission */

	do {
		if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;
		strcpy(ptr, fd.cFileName);
		DoFiles(fullname, NULL);
	} while (!AbortOp && FindNextFile(f, &fd));

	FindClose(f);
#else
	struct dirent *e;
	DIR *d = opendir(dname);
	if (!d) return;
	ptr[-1] = '/';

	while (!AbortOp && (e = readdir(d)))
	{
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		strcpy(ptr, e->d_name);
		DoFiles(fullname, NULL);
	}

	closedir(d);
#endif
	}
}

void DoFiles (char *fname, void (*func)(char *))
{
	uInt filetime;

/* handle wildcards for Win32, assume that
   other systems will handle them for me! */
#ifdef WIN32
	HANDLE f;
	WIN32_FIND_DATA fd;
	char fullname[260];
	int p = GetFileFromPath(fname) - fname;

	f = FindFirstFile(fname, &fd);
	if (f == INVALID_HANDLE_VALUE)
	{	/* file not found, try adding .dem if no wildcards */
		if (!*FileExtension(fname) && !strpbrk(fname, "*?"))
		{
			strcat(fname, ".dem");
			f = FindFirstFile(fname, &fd);
			if (f == INVALID_HANDLE_VALUE)
				*FileExtension(fname) = 0;
		}
		if (f == INVALID_HANDLE_VALUE)
		{
			error("could not open %s", fname);
			return;
		}
	}

	do
	{
		strncpy(fullname, fname, p);
		strcpy(fullname + p, fd.cFileName);

		if (func)
		{	/* either dzList or dzUncompress */
			if (fd.nFileSizeHigh)
				dzsize = 0;
			else
				dzsize = fd.nFileSizeLow;
			func(fullname);
			continue;
		}

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			DoDirectory(fullname);
			continue;
		}

		/* cant add the current file to itself */
		if (!strcasecmp(fullname, dzname))
			continue;

		if (fd.nFileSizeHigh)
		{
			error("%s is 4GB or more and can't be compressed", fullname);
			return;
		}

		infile = fopen(fullname, "rb");
		if (!infile)
		{
			error("could not open %s: %s", fullname, strerror(errno));
			continue;
		}

		FileTimeToLocalFileTime(&fd.ftLastWriteTime, &fd.ftLastAccessTime);
		FileTimeToDosDateTime(&fd.ftLastAccessTime, (short *)&filetime + 1, (short *)&filetime);

		dzCompressFile(fullname, fd.nFileSizeLow, filetime - (1 << 21));
		fclose(infile);
		if (AbortOp == 3)	/* had a problem reading from infile */
			AbortOp = 0;	/* but still try the rest of the files */

	} while (!AbortOp && FindNextFile(f, &fd));
	FindClose(f);

#else

	struct stat64 filestats;
	struct tm *trec;

	if (stat64(fname, &filestats))
	{	/* file probably doesn't exist if this failed */
		error("could not open %s: %s", fname, strerror(errno));
		return;
	}

	if (func)
	{	/* either dzList or dzUncompress */
		if (filestats.st_size > 0xFFFFFFFF)
			dzsize = 0;
		else
			dzsize = filestats.st_size;
		func(fname);
		return;
	}

	if (filestats.st_mode & S_IFDIR)
	{
		DoDirectory(fname);
		return;
	}

	/* cant add the current file to itself */
	if (!strcasecmp(fname, dzname))
		return;

	if (filestats.st_size > 0xFFFFFFFF)
	{
		error("%s is 4GB or more and can't be compressed", fname);
		return;
	}

	infile = fopen(fname, "rb");
	if (!infile)
	{
		error("could not open %s: %s", fname, strerror(errno));
		return;
	}

	trec = localtime(&filestats.st_mtime);
	filetime = (trec->tm_sec >> 1) + (trec->tm_min << 5)
		+ (trec->tm_hour << 11) + (trec->tm_mday << 16)
		+ (trec->tm_mon << 21) + ((trec->tm_year - 80) << 25);

	dzCompressFile(fname, filestats.st_size, filetime);
	fclose(infile);
	if (AbortOp == 3)	/* had a problem reading from infile */
		AbortOp = 0;	/* but still try the rest of the files */

#endif
}

int main(int argc, char **argv)
{
	int i, j, oflag = 0;
	int fileargs = 0;
	char *optr = NULL;
	char **files;
	char *fname;

	if (argc < 2) usage();

	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i],"-o"))
			oflag = 1;
		else if (*argv[i] == '-')
			parse_switch(argv[i]);
		else if (oflag)
			optr = argv[i];
		else
			fileargs++;

	if (!fileargs) errorFatal("no input files");
	j = !!flag[SW_LIST] + !!flag[SW_EXTRACT]
		+ !!flag[SW_VERIFY] + !!flag[SW_ADD] + !!flag[SW_DELETE];
	if (j > 1) errorFatal("conflicting switches");
	if (oflag && j == 1) errorFatal("-o invalid with other switches");
	if (oflag && !optr) errorFatal("-o without argument");

	crc_init();
	inblk = Dzip_malloc(p_blocksize * 3);
	outblk = inblk + p_blocksize;
	tmpblk = outblk + p_blocksize / 2;
	zbuf = tmpblk + p_blocksize / 2;

	files = Dzip_malloc(fileargs * 4);
	for (i = 1, j = 0; i < argc; i++)
	{
		if (*argv[i] != '-') files[j++] = argv[i];
		if (!strcmp(argv[i],"-o")) i++;
	}

	zs.zalloc = Dzip_calloc;
	zs.zfree = free;

	if (flag[SW_LIST] || flag[SW_EXTRACT] || flag[SW_VERIFY])
	{
		void (*func)(char *) = flag[SW_LIST] ? dzList : dzUncompress;
		for (i = 0; i < fileargs && !AbortOp; i++)
		{
			fname = Dzip_malloc(strlen(files[i])+4);
			strcpy(fname,files[i]);
#ifdef WIN32 /* only add extension automatically on windows */
			if (!*FileExtension(fname))
				strcat(fname, ".dz");
#endif
			DoFiles(fname, func);
			free(fname);
		}
		exit(0);
	}

	if (!flag[SW_ADD] && !flag[SW_DELETE])
	{	/* create a new dz and compress files */
		if (!optr)
		{
			optr = Dzip_malloc(strlen(files[0]) + 4);
			strcpy(optr, files[0]);
			strcpy(FileExtension(optr), ".dz");
		}
	#ifdef WIN32
		else if (!*FileExtension(optr))
		{
			char *t = Dzip_malloc(strlen(optr) + 4);
			sprintf(t, "%s.dz", optr);
			optr = t;
		}
	#endif
		dzname = optr;
		dzfile = open_create(optr);
		if (!dzfile) exit(1);
		i = 'D' + ('Z' << 8) + (MAJOR_VERSION<<16) + (MINOR_VERSION<<24);
		i = cnvlong(i);
		dzFile_Write(&i, 12);
		totalsize = 12;

		directory = Dzip_malloc(sizeof(direntry_t));

		for (i = 0; i < fileargs; i++) 
		{
			fname = Dzip_malloc(strlen(files[i])+5);
			strcpy(fname,files[i]);
			DoFiles(fname, NULL);
			free(fname);
		}

		free(files);
		dzWriteDirectory();
		dzClose();
		if (!numfiles)
			remove(dzname);
		exit(0);
	}

	/* add or delete */
	if (fileargs < 2)
		errorFatal("no input files");
	fname = Dzip_malloc(strlen(files[0])+4);
	strcpy(fname,files[0]);
#ifdef WIN32
	if (!*FileExtension(fname))
		strcat(fname, ".dz");
#endif
	dzsize = 0;
	/* figure out dzsize */
	{
#ifdef WIN32
	WIN32_FIND_DATA fd;

	FindClose(FindFirstFile(fname, &fd));
	if (!fd.nFileSizeHigh)
		dzsize = fd.nFileSizeLow;
#else
	struct stat64 filestats;

	if (!stat64(fname, &filestats))
		if (!(filestats.st_size > 0xFFFFFFFF))
			dzsize = filestats.st_size;
#endif
	}

	if (!dzOpen(fname, 1))	/* add & delete need write access */
		exit(0);

	if (flag[SW_ADD])
	{
		dzFile_Seek(4);
		dzFile_Read(&totalsize, 4);
		dzFile_Seek(totalsize);
		for (i = 1; i < fileargs; i++) 
		{
			fname = Dzip_malloc(strlen(files[i])+5);
			strcpy(fname,files[i]);
			DoFiles(fname, NULL);
			free(fname);
		}
		dzWriteDirectory();
		dzClose();
	}
	else
		dzDeleteFiles_MakeList(files + 1, fileargs - 1);
	free(files);
	exit(0);
}