//
//  langCommon.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#ifndef langCommon_h
#define langCommon_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "utf.h"

#include "langPosix.h"
#include "langLimits.h"
#include "langTypes.h"
#include "langConstants.h"

#include "langState.h"

#include "langUTFString.h"

typedef char * CSTRING;

//PSTRING macro to create a CString of 255 characters
#define CSTRING(name,sz) char *name = (char *) malloc((sz+1) * sizeof(char));

#define isalpha(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define isdigit(c) (c >= '0' && c <= '9')
#define ishex(c) ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define is_special(c) (( (c == '=') || (c == '+')  || (c == '-') || (c == '*') || (c == '/') || (c == '<') || (c == '>') || (c=='~') || (c == '(')  || (c == ')')  || (c == '{')  || (c == '}') || (c == '[') || (c == ']') || (c==';') || (c==',') || (c == '.')) ? 1 : 0)


#define is_token_op(a) (a->type == kTK_OPERATOR)
#define is_op_logical(a) ((a == opType_LogicalEqual) || (a == opType_NotEqual) || (a == opType_Less) || (a == opType_LessEqual) || (a == opType_Greater) || (a == opType_GreaterEqual) )
#define is_op_additive(a) ((a == opType_Plus) || (a == opType_Minus) || (a == opType_OR))
#define is_op_multiplicative(a) ((a == opType_Multiply) || (a == opType_Divide) || (a == opType_Modulo) || (a == opType_AND))

char * strClone(char * a);
int strSafeCopy(char *dest, int size, char *src);
int strCopy(char * a, char * b);

char * langCreateID(const char *prefix, int sz);

void *oom(void *ptr);


#pragma mark DICTIONARY

uint64_t dictionaryComputeKey(const char* key);
void * dictionaryGet(hashDictionary* table, const char* key);
langDictIterator dictionaryGetIterator(hashDictionary* table);
int dictionaryNext(langDictIterator* it);

#endif /* langCommon_h */
