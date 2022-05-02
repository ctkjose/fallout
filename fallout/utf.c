/*
 * The authors of this software are Rob Pike and Ken Thompson.
 *              Copyright (c) 2002 by Lucent Technologies.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR LUCENT TECHNOLOGIES MAKE
 * ANY REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
#include <stdlib.h>
#include <string.h>

#include "utf.h"

typedef unsigned char uchar;

enum {
	Bit1 = 7,
	Bitx = 6,
	Bit2 = 5,
	Bit3 = 4,
	Bit4 = 3,
    Bit5 = 2,
	
	T1	= ((1<<(Bit1+1))-1) ^ 0xFF,	/* 0000 0000 */
	Tx	= ((1<<(Bitx+1))-1) ^ 0xFF,	/* 1000 0000 */
	T2	= ((1<<(Bit2+1))-1) ^ 0xFF,	/* 1100 0000 */
	T3	= ((1<<(Bit3+1))-1) ^ 0xFF,	/* 1110 0000 */
	T4	= ((1<<(Bit4+1))-1) ^ 0xFF,	/* 1111 0000 */
    T5  = ((1<<(Bit5+1))-1) ^ 0xFF,    /* 1111 1000 */
	
	Rune1 = (1<<(Bit1+0*Bitx))-1,		/* 0000 0000 0111 1111 */
	Rune2 = (1<<(Bit2+1*Bitx))-1,		/* 0000 0111 1111 1111 */
	Rune3 = (1<<(Bit3+2*Bitx))-1,		/* 1111 1111 1111 1111 */
    Rune4 = (1<<(Bit4+3*Bitx))-1,        /* 0001 1111 1111 1111 1111 1111 */
	
	Maskx = (1<<Bitx)-1,			/* 0011 1111 */
	Testx = Maskx ^ 0xFF,			/* 1100 0000 */
	
    SurrogateMin = 0xD800,
    SurrogateMax = 0xDFFF,
    
	Bad	= Runeerror,
};

///Returns how many char took the UTF sequence, max is defined in UTFmax.
///
///Chartorune copies at most UTFmax bytes starting at `str` to one rune at `r` and returns the number of bytes copied.
///If the input is not exactly in UTF format, chartorune will convert to Runeerror (0xFFFD) and return 1.
int chartorune(Rune *rune, const char *str) {
	int c, c1, c2, c3;
	int l;
	
	/*
	 * one character sequence
	 *	00000-0007F => T1
	 */
	c = *(unsigned char *)str;
	if(c < Tx) {
		*rune = c;
		return 1;
	}
	
	/*
	 * two character sequence
	 *	0080-07FF => T2 Tx
	 */
	c1 = *(unsigned char *)(str+1) ^ Tx;
	if(c1 & Testx)
		goto bad;
	if(c < T3) {
		if(c < T2)
			goto bad;
		l = ((c << Bitx) | c1) & Rune2;
		if(l <= Rune1)
			goto bad;
		*rune = l;
		return 2;
	}
	
	/*
	 * three character sequence
	 *	0800-FFFF => T3 Tx Tx
	 */
	c2 = *(unsigned char*)(str+2) ^ Tx;
	if(c2 & Testx)
		goto bad;
	if(c < T4) {
		l = ((((c << Bitx) | c1) << Bitx) | c2) & Rune3;
		if(l <= Rune2)
			goto bad;
		*rune = l;
		return 3;
	}
    
    /*
     * four character sequence (21-bit value)
     *    10000-1FFFFF => T4 Tx Tx Tx
     */
    c3 = *(uchar*)(str+3) ^ Tx;
    if (c3 & Testx)
        goto bad;
    if (c < T5) {
        l = ((((((c << Bitx) | c1) << Bitx) | c2) << Bitx) | c3) & Rune4;
        if (l <= Rune3 || l > Runemax)
            goto bad;
        *rune = (Rune)l;
        return 4;
    }

	
	/*
	 * bad decoding
	 */
bad:
	*rune = Bad;
	return 1;
}

///Returns how many char took the UTF sequence, max is defined in UTFmax.
///
///Copies one rune at `rune` to at most UTFmax bytes starting at `str` and returns the number of bytes copied.
int runetochar(char *str, const Rune *rune) {
	int c;
	
	/*
	 * one character sequence
	 *	00000-0007F => 00-7F
	 */
	c = *rune;
	if(c <= Rune1) {
		str[0] = c;
        str[1] = '\0';
		return 1;
	}
	
	/*
	 * two character sequence
	 *	0080-07FF => T2 Tx
	 */
	if(c <= Rune2) {
		str[0] = T2 | (c >> 1*Bitx);
		str[1] = Tx | (c & Maskx);
        str[2] = '\0';
		return 2;
	}
	
    /*
     * If the Rune is out of range or a surrogate half, convert it to the error rune.
     * Do this test here because the error rune encodes to three bytes.
     * Doing it earlier would duplicate work, since an out of range
     * Rune wouldn't have fit in one or two bytes.
     */
    if (c > Runemax)
        c = Runeerror;
    if (SurrogateMin <= c && c <= SurrogateMax)
        c = Runeerror;
    /*
     * three character sequence
     *    0800-FFFF => T3 Tx Tx
     */
    if (c <= Rune3) {
        str[0] = (char)(T3 |  (c >> 2*Bitx));
        str[1] = (char)(Tx | ((c >> 1*Bitx) & Maskx));
        str[2] = (char)(Tx |  (c & Maskx));
        str[3] = '\0';
        return 3;
    }
    /*
     * four character sequence (21-bit value)
     *     10000-1FFFFF => T4 Tx Tx Tx
     */
    str[0] = (char)(T4 | (c >> 3*Bitx));
    str[1] = (char)(Tx | ((c >> 2*Bitx) & Maskx));
    str[2] = (char)(Tx | ((c >> 1*Bitx) & Maskx));
    str[3] = (char)(Tx | (c & Maskx));
    str[4] = '\0';
    
    return 4;
}

int fullrune(const char *str, int n){
    if (n > 0) {
        int c = *(unsigned char*)str;
        if (c < Tx)
            return 1;
        if (n > 1) {
            if (c < T3)
                return 1;
            if (n > 2) {
                if (c < T4 || n > 3)
                    return 1;
            }
        }
    }
    return 0;
}

///Returns the number of bytes required to convert r into UTF.
int runelen(int c){
	Rune rune;
	char str[10];
	
	rune = c;
	return runetochar(str, &rune);
}


///Returns the length of the UTF string `s` accounting for rune characters.
int utflen(const char *s) {
	int c;
	int n;
	Rune rune;
	
	n = 0;
	for(;;) {
		c = *(unsigned char*)s;
		if(c < Runeself) {
			if(c == 0)
				return n;
			s++;
		} else
			s += chartorune(&rune, s);
		n++;
	}
    
    return n;
}

///Returns the number of complete runes that are represented by the first n bytes of UTF string s.
///If the last few bytes of the string contain an incompletely coded rune, utfnlen will not count them; in this way, it differs from utflen, which includes every byte of the string.
int utfnlen(const char *s, long m){
    int c;
    int n;
    Rune rune;
    const char *es;
    es = s + m;
    for(n = 0; s < es; n++) {
        c = *(unsigned char*)s;
        if(c < Runeself){
            if(c == '\0')
                break;
            s++;
            continue;
        }
        if(!fullrune(s, (int)(es-s)))
            break;
        s += chartorune(&rune, s);
    }
    return n;
}


int runeAtIndex(char *str, int i){
    Rune rune = 0;
    
    int c;
    int n;
    
    
    n = 0;
    for(;;) {
        c = *(unsigned char*)str;
        if(c < Runeself) {
            if(c == 0) break;
            str++;
            rune = (unsigned short) c;
        } else {
            str+= chartorune(&rune, str);
        }
        if(n==i) break;
        n++;
    }
    
    return rune;
}

///Returns a pointer to the first occurrence of a rune in a UTF string
///
///Equivalent to strchr
char* utfrune(const char *s, Rune c){
    long c1;
    Rune r;
    int n;
    if(c < Runesync)        /* not part of utf sequence */
        return strchr(s, (char)c);
    for(;;) {
        c1 = *(unsigned char*)s;
        if(c1 < Runeself) {    /* one byte rune */
            if(c1 == 0)
                return 0;
            if(c1 == c)
                return (char*)s;
            s++;
            continue;
        }
        n = chartorune(&r, s);
        if(r == c)
            return (char*)s;
        s += n;
    }
    return 0;
}

///Returns a pointer to the last occurrence of a rune in a UTF string;
///
///Equivalent to strrchr
char* utfrrune(const char *s, Rune c){
    long c1;
    Rune r;
    const char *s1;
    if(c < Runesync)        /* not part of utf sequence */
        return strrchr(s, (char)c);
    s1 = 0;
    for(;;) {
        c1 = *(unsigned char*)s;
        if(c1 < Runeself) {    /* one byte rune */
            if(c1 == 0)
                return (char*)s1;
            if(c1 == c)
                s1 = s;
            s++;
            continue;
        }
        c1 = chartorune(&r, s);
        if(r == c)
            s1 = s;
        s += c1;
    }
    return 0;
}
///Searches for the first occurrence of a UTF string in another UTF string
///
///utfutf is the same as strstr if the arguments point to valid UTF strings
const char* utfutf(const char *s1, const char *s2){
    const char *p;
    long f, n1;
    size_t n2;
    Rune r;
    n1 = chartorune(&r, s2);
    f = r;
    if(f <= Runesync) {       /* represents self */
        return strstr(s1, s2);
    }
    n2 = strlen(s2);
    for(p=s1; (p=utfrune(p, r)) != 0; p+=n1){
        if(strncmp(p, s2, n2) == 0){
            return p;
        }
    }
    return 0;
}

///Utfecpy copies UTF sequences until a null sequence has been copied, but writes no sequences beyond e.
///If any sequences are copied, to is terminated by a null sequence, and a pointer to that sequence is returned.
///Otherwise, the original s1 is returned.
char* utfecpy(char *to, char *e, const char *from){
    char *end;
    if(to >= e){
        return to;
    }
    end = memccpy(to, from, '\0', (size_t)(e - to));
    if(!end){
        end = e-1;
        while(end>to && (*--end&0xC0)==0x80);
        *end = '\0';
    }else{
        end--;
    }
    return end;
}
