//
//  utf.h
//  fallout


#ifndef utf_h
#define utf_h


typedef unsigned short Rune;	/* 16 bits */

enum {
	UTFmax		= 3,		/* maximum bytes per rune */
	Runesync	= 0x80,		/* cannot represent part of a UTF sequence (<) */
	Runeself	= 0x80,		/* rune and UTF sequences are the same (<) */
	Runeerror	= 0xFFFD,	/* decoding error in UTF */
    Runemax     = 0x10FFFF,    /* maximum rune value */
};


int	chartorune(Rune *rune, const char *str);
int	runetochar(char *str, const Rune *rune);
int	runelen(int c);
int	utflen(const char *s);
int utfnlen(const char *s, long m);

int runeAtIndex(char *str, int i);
char* utfrune(const char *s, Rune c);
char* utfrrune(const char *s, Rune c);
const char* utfutf(const char *s1, const char *s2);
char* utfecpy(char *to, char *e, const char *from);

int		isalpharune(Rune c);
int		islowerrune(Rune c);
int		isspacerune(Rune c);
int		istitlerune(Rune c);
int		isupperrune(Rune c);
Rune		tolowerrune(Rune c);
Rune		totitlerune(Rune c);
Rune		toupperrune(Rune c);


#endif /* utf_h */
