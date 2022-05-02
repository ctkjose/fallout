//
//  langUTFString.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#ifndef langUTFString_h
#define langUTFString_h

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "utf.h"


typedef struct uString * ustr;
struct uString {
	char * text;
	int length;
	int size;
};

struct uString* utf8_CreateString(int sz);
//void utf8_PushUNI(ustr str, Rune ch)
void utf8_PushCharCode(ustr str, Rune ch);
void utf8_Terminate(ustr str);
int utf8_RuneAtIndex(ustr str, int i);
char * utf8_CharAtIndex(ustr str, int i);

#endif /* langUTFString_h */
