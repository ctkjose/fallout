//
//  utf.c

//  Small library to handle UTF8 strings.
//  Inspired by Rob Pike & Ken Thompson UTF work on Plan9 Library
//  https://github.com/yhcote/plan9c
//  https://en.wikipedia.org/wiki/UTF-8
//
//  Created by JOSE L CUEVAS on 5/1/22.
//  https://exponentialworks.com
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utf.h"




int utf_cp_bounds[4][4] = {
    /* upper bound, bits mask, lead of 1st byte */
    { 0x7F, 0x7F, 0, 0x80}, //1 byte
    { 0x7FF, 0x3F00, 0xC0, 0xE0}, //2 bytes
    { 0xFFFF, 0xF0000, 0xE0, 0xF0}, //3 bytes
    { 0x10FFFF, 0x7000000, 0xF0, 0xF8 } //4 bytes
};

UString utf8Create(int sz){
    UString s;
    s = malloc(sizeof(struct uStringData));
    if(!s) return NULL;
    s->text = malloc(sizeof(char) * sz);
    s->asize = sz;
    
    s->strlen = 0;
    s->length = 0;
    
    return s;
}

UString utf8CreateWithCString(CString value){
    UString s;
    size_t sz = strlen(value) + 1;
    s = malloc(sizeof(struct uStringData));
    if(!s) return NULL;
    s->text = malloc(sizeof(char) * sz);
    s->asize = (int) sz;
    
    s->strlen = (int) sz-1;
    s->length = 0;
    
    memcpy(s->text, value, sz);
    s->text[s->asize] = '\0';
    
    char *p = value;
    while(p){
        char ch = *p;
        if(ch == '\0') break;
        int sz = utf8BytesFromChar(ch);
        p+= sz;
        s->length++;
    }
    
    return s;
}

int utf8AppendUString(UString str, UString value){
    const int buffsz = 255;
    
    if(!str) return 0;
    if(!value) return str->length;
    
    size_t sz = value->strlen;
    
    if( (str->strlen + sz) > str->asize){
        str->asize += buffsz + 1;
        str->text = realloc(str->text, sizeof(char)*str->asize);
    }
    
    
    strncpy(str->text+str->strlen, value->text, sz);
    str->strlen += sz;
    str->text[str->strlen]='\0';
    
    str->length += value->length;
    
    return str->length;
}

int utf8AppendCString(UString str, CString value){
    const int buffsz = 255;
    
    if(!str) return 0;
    if(!value) return str->length;
    
    size_t sz = strlen(value);
    
    if( (str->strlen + sz) > str->asize){
        str->asize += buffsz + 1;
        str->text = realloc(str->text, sizeof(char)*str->asize);
    }
    
    
    strncpy(str->text+str->strlen, value, sz);
    str->strlen += sz;
    str->text[str->strlen]='\0';
    
    
    char *p = value;
    while(p){
        char ch = *p;
        if(ch == '\0') break;
        int sz = utf8BytesFromChar(ch);
        p+= sz;
        str->length++;
    }
    

    return str->length;
}

int utf8AppendCharacter(UString str, Character cp){
    const int buffsz = 255;
    
    if(!str) return 0;
    if(cp == 0) return str->length;
   
    int bytes = utf8BytesForCodepoint(cp);
    
    if( (str->strlen + bytes) > str->asize){
        str->asize += buffsz + 1;
        str->text = realloc(str->text, sizeof(char)*str->asize);
    }
    
    
    
    
    char *p = utf8CharacterToCString(cp);
    if(p){
        strncpy(str->text+str->strlen, p, bytes);
    }
    
    str->strlen += bytes;
    str->length++;
    str->text[str->strlen]='\0';
    
    return str->length;
}

///Returns the Codepoint of the unicode character at  the index `pos`.
///
///Returns 0 if  error.
Character utf8CharacterAtIndex(CString str, int pos){

    int i = 0;
    char *p = str;
    
    while(p){
        char ch = *p;
        if(ch == '\0') break;
        int sz = utf8BytesFromChar(ch);
        if( i == pos){
            if(sz == 1) return ch;
            Character rune;
            sz = utf8CharacterFromCString(p, &rune);
            return rune;
        }
        p++;
        i++;
    }
    
    return 0;
}

///Returns the index of the first instance of `needle` found at `haystack`.
///
///Returns -1 if `needle` is not found.
int  utf8IndexOf(CString haystack, CString needle){

    int i = 0;
    char *p = haystack;
    size_t nsz = strlen(needle);
    
    while(p){
        char ch = *p;
        if(ch == '\0') break;
        int sz = utf8BytesFromChar(ch);
        int cmp = 0;
        if(sz == 1){
            if(ch == *needle){
                cmp = 1;
            }
        }else{
            if( strncmp((const char *)p, (const char *)needle, sz) == 0 ){
                cmp = 1;
            }
        }
        if(cmp){
            if( strncmp((const char *)p, (const char *)needle, nsz) == 0 ){
                return i;
            }
        }
        p++;
        i++;
    }
    
    return -1;
}
///Returns the number of bytes for a codepoint that starts with the char in `ch`.
///
///The char `ch` is the first byte of the codepoint.
int utf8BytesFromChar(char ch){
    int bytes;
    unsigned int byte = (unsigned int) ch;
    if(byte <= 0) return 0;
         
    for( bytes=0; bytes <= 3; bytes++){
       if( (utf_cp_bounds[bytes][3] & byte) == utf_cp_bounds[bytes][2] ) break;
    }
    
    if( bytes > 3 ) return 0; //Error
    return bytes + 1;
}
///Returns the number of bytes for the codepoint in `rune`.
int utf8BytesForCodepoint(Character rune){
    int bytes;
    if(rune <= 0) return 0;
         
    for( bytes=0; bytes <= 3; bytes++){
       if( rune <= utf_cp_bounds[bytes][0]) break;
    }
    
    if( bytes > 3 ) return 0; //Error
    return bytes + 1;
}


///Sets `rune` to the codepoint at the current position of the `str`.
///
///Returns the number of bytes used by the codepoint.  If the codepoint is invalid return 0.
///
///The parameter `rune` is initialized to 0.
int utf8CharacterFromCString(CString str, Character *rune){
    int bytes, b;
    
    *rune = 0;
    if(!str) return 0;
    
    unsigned int byte = (unsigned int) *str;
    if(byte == 0) return 0;
    
    for( bytes=0; bytes <= 3; bytes++){
       if( (utf_cp_bounds[bytes][3] & byte) == utf_cp_bounds[bytes][2] ) break;
    }
    
    if( bytes > 3 ) return 0; //Error
    
    int out=0;
    int v, m;
    
    for(b=bytes; b >= 0; b--){
        byte = (unsigned int) str[b];
        m = (b > 0) ? 0x03F : (0xFF ^ utf_cp_bounds[bytes-b][3]);
        //printf("BYTE[%d] m=%x\n", b, m);
        v = (m & byte) << (bytes-b)*6;
        out = out ^ v;
        //printf("v=%x rune=%02x\n", v, out);
    }
    
    *rune = out;
    return bytes + 1;
    
}
char* utf8CharacterToCString(int cp){
    
    char *str = (char *) malloc(sizeof(char) * UTFmax + 1);
    
    
    //memset(str,0, 5);
    
    int bytes, b, v, m, l;

    for( bytes=0; bytes<=3; bytes++){
        if( cp <= utf_cp_bounds[bytes][0]) break;
    }
    
    for(b=bytes; b>=0;b--){
        v = (cp >> (bytes-b)*6);
        //printf("cp=%x\n", v);
        m = (b > 0) ? 0x3F : 0xFF ^ utf_cp_bounds[b][3];
        l = (b > 0) ? 0x80 : utf_cp_bounds[bytes][2];
        //printf("BYTE[%d] (v=%x m=%x)=%x\n",b, v, m, (m & v));
        v = (m & v) | l;
        //printf("BYTE[%d]=%x\n", b, v);
        str[b] = (char) v;
    }
    
    str[bytes+1] = '\0';
    
    
    return str;
}
