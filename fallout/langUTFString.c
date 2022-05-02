//
//  langUTFString.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#include "langUTFString.h"

ustr utf8_CreateString(int sz){
	ustr s;
	s = malloc(sizeof(struct uString));
	if(!s) return NULL;
	s->text = malloc(sizeof(char) * sz);
	s->size = sz;
	s->length = 0;
	
	return s;
}

//return cstring with bytes in unicode sequence, up to UTFmax
char * utf8_CharAtIndex(ustr str, int i){
	char *buf = malloc(sizeof(char) * (UTFmax + 1));
	buf[0] = '\0';
	Rune rune = utf8_RuneAtIndex(str, i);
	if (rune > 0) {
		buf[runetochar(buf, &rune)] = 0;
	}
	return buf;
}

//Return RUNE as 16bit integer
int utf8_RuneAtIndex(ustr str, int i){
	Rune rune = 0;
	const char *s = str->text;
	while (i-- >= 0) {
		rune = *(unsigned char*)s;
		if (rune < Runeself) {
			if (rune == 0) return 0;
			++s;
		} else{
			s += chartorune(&rune, s);
		}
	}
	return rune;
}

//add a unicode char to the end of the string
void utf8_PushCharCode(ustr str, Rune ch){
	
	int sz = 255;
	int n = runelen(ch);
	if( (str->length + n) > str->size){
		str->size += sz + 1;
		str->text = realloc(str->text, sizeof(char)*str->size);
	}
	
	str->length += runetochar(str->text + str->length, &ch);
}


void utf8_Terminate(ustr str){
	utf8_PushCharCode(str, 0);
}

