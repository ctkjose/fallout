//
//  utf.h
//  utf_lib
//
//  Created by JOSE L CUEVAS on 5/1/22.
//

#ifndef utf_h
#define utf_h


typedef unsigned int Character;
typedef char * CString;
typedef struct uStringData * UString;
struct uStringData {
    char * text;
    int length; //actual unicode length (how many codepoints)
    int asize; //allocated size
    int strlen; //lenght of *text
};

#define UTFmax 4
extern int utf_cp_bounds[4][4];

UString utf8Create(int sz);
UString utf8CreateWithCString(CString value);

int utf8AppendUString(UString str, UString value);
int utf8AppendCharacter(UString str, Character cp);
int utf8AppendCString(UString str, CString value);

Character utf8CharacterAtIndex(CString str, int pos);
int utf8IndexOf(CString haystack, CString needle);
int utf8BytesFromChar(char ch);
int utf8BytesForCodepoint(Character rune);

char* utf8CharacterToCString(int cp);
int utf8CharacterFromCString(CString str, Character *rune);

#endif /* utf_h */
