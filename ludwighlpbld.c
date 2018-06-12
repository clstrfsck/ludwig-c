/**********************************************************************/
/*                                                                    */
/*           L      U   U   DDDD   W      W  IIIII   GGGG             */
/*           L      U   U   D   D   W    W     I    G                 */
/*           L      U   U   D   D   W ww W     I    G   GG            */
/*           L      U   U   D   D    W  W      I    G    G            */
/*           LLLLL   UUU    DDDD     W  W    IIIII   GGGG             */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/*  Copyright (C) 1981, 1987                                          */
/*  Department of Computer Science, University of Adelaide, Australia */
/*  All rights reserved.                                              */
/*  Reproduction of the work or any substantial part thereof in any   */
/*  material form whatsoever is prohibited.                           */
/*                                                                    */
/**********************************************************************/

/*
 * Name:        LUDWIGHLP
 *
 * Description: This program converts a sequential Ludwig help file into
 *              an indexed file for fast access.
 *
 * Revision History:
 * 4-001 Ludwig V4.0 release.                                 7-Apr-1987
 * 4-002 Kelvin B. Nicolle                                    5-May-1987
 *       The input text has been reformatted so that column one contains
 *       only flag characters.
 * 4-003 Jeff Blows                                          23-Jun-1989
 *       Merge changes needed to compile on MS-DOS
 * 4-004 Ludwig                                              28-Feb-1990
 *       Stop complaining about line too long when it's a comment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(MACOS) || defined(linux)
#include <unistd.h>
#endif

#define BUFSIZE         1024
#define ENTRYSIZE       78      /* 77 + 1 for NUL */
#define KEYSIZE         4

void process_files(FILE *, FILE *);

int
main(int argc, char **argv)
{
    char infile[BUFSIZE],outfile[BUFSIZE];
    FILE *in,*out;

    if (--argc > 0)
	strcpy(infile,*++argv);
    else
#ifdef turboc
	strcpy(infile,"ludwighl.txt");
#else
	strcpy(infile,"ludwighlp.t");
#endif
    if ((in = fopen(infile,"r")) == NULL) {
	perror(infile);
	exit(1);
    }
    if (--argc > 0)
	strcpy(outfile,*++argv);
    else
#ifdef turboc
	strcpy(outfile,"ludwighl.idx");
#else
	strcpy(outfile,"ludwighlp.idx");
#endif
    if ((out = fopen(outfile,"w+")) == NULL) {
	perror(outfile);
	exit(1);
    }
    process_files(in,out);
    printf("Conversion complete.\n");
    exit(0);
}

void
process_files(in,out)
FILE *in,*out;

{
# ifdef MKSTEMP
    int f1, f2, f3;
# else
    char *f1, *f2, *f3;
# endif
    char flag,line[ENTRYSIZE+1],section[KEYSIZE+1];
    long i,len,index_lines,contents_lines;
    FILE *index,*body,*contents;
# ifndef ultrix
    char *mktemp();
    char temp1[] = "LWXXXXXX";
    char temp2[] = "LWXXXXXX";
    char temp3[] = "LWXXXXXX";
# endif

    index_lines = contents_lines = 0;
    strcpy(section,"0");
    /*
     * Create two temporary files to store the index and the actual info
     * while we work through the file.
     */
# ifdef ultrix
    if ((index = fopen(f1 = tempnam("/tmp","hlp"),"w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
    if ((body = fopen(f2 = tempnam("/tmp","hlp"),"w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
    if ((contents = fopen(f3 = tempnam("/tmp","hlp"),"w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
# else
#  ifdef MKSTEMP
    if ((index = fdopen(f1 = mkstemp(temp1), "w+")) == NULL) {
        perror("tmpfile");
        exit(1);
    }
    if ((body = fdopen(f2 = mkstemp(temp2), "w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
    if ((contents = fdopen(f3 = mkstemp(temp3), "w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
#  else
    if ((index = fopen(f1 = mktemp(temp1),"w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
    if ((body = fopen(f2 = mktemp(temp2),"w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
    if ((contents = fopen(f3 = mktemp(temp3),"w+")) == NULL) {
	perror("tmpfile");
	exit(1);
    }
#  endif
# endif
    do {
	flag = fgetc(in);
	if (flag == EOF)
	    break;
	else if (flag == '\n') {
	    flag = ' ';
	    len = 0;
	    line[0] = '\0';
	} else {
	    if (fgets(line,ENTRYSIZE+1,in) == NULL) break;
	    len = strlen(line);
	    if (len == ENTRYSIZE && line[ENTRYSIZE-1] != '\n') {
		if (flag != '!' && flag != '{') {
		  fprintf(stderr,"Line too long--truncated\n");
		  fprintf(stderr,"%*s>>\n",ENTRYSIZE,line);
		}
		while (fgetc(in) != '\n') continue;
	    }
	    line[len-1] = '\0';
	}
	switch (flag) {
	    case '\\' :
		switch (line[0]) {
		    case '%' :
			fputs("\\%\n",body);
			break;
		    case '#' :
			if (strcmp(section,"0") != 0)
			    fprintf(index,"%8ld\n",ftell(body));
			break;
		    default :
			if (strcmp(section,"0") != 0)
			    fprintf(index,"%8ld\n",ftell(body));
			for (i = 0;i < KEYSIZE && line[i];i++)
			    section[i] = line[i];
			section[i] = '\0';
			if (strcmp(section,"0") != 0) {
			    index_lines++;
			    fprintf(index,"%4s %8ld",section,ftell(body));
			}
			break;
		}
		break;
	    case '+' :
		contents_lines++;
		fprintf(contents,"%s\n",line);
		fprintf(body,"%s\n",line);
		break;
	    case ' ' :
		if (strcmp(section,"0") == 0) {
		    contents_lines++;
		    fprintf(contents,"%s\n",line);
		} else
		    fprintf(body,"%s\n",line);
		break;
	    case '{' :
	    case '!' :
		break;
	    default :
		fprintf(stderr,"Illegal flag character.\n");
		fprintf(stderr,"%c%s>>\n",flag,line);
		break;
	}
    } while (!(flag == '\\' && line[0] == '#'));
    rewind(index);
    rewind(body);
    rewind(contents);
    fprintf(out,"%ld %ld\n",index_lines,contents_lines);
    while (fgets(line,ENTRYSIZE+1,index) != NULL)
	fputs(line,out);
    while (fgets(line,ENTRYSIZE+1,contents) != NULL)
	fputs(line,out);
    while (fgets(line,ENTRYSIZE+1,body) != NULL)
	fputs(line,out);
    fclose(index);
    fclose(contents);
    fclose(body);
#ifdef MKSTEMP
    unlink(temp1);
    unlink(temp2);
    unlink(temp3);
#else
    unlink(f1);
    unlink(f2);
    unlink(f3);
#endif
}
