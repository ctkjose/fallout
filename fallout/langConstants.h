//
//  langConstants.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/22/21.
//

#ifndef langConstants_h
#define langConstants_h

//==============================================
#pragma mark ERROR CODES
//==============================================

#define kERR_SYNTAX 100
#define kERR_RUNTIME 101

//==============================================
#pragma mark TOKEN TYPES
//==============================================

#define kTK_NULL 0
#define kTK_IDENTIFIER 1
#define kTK_STRING 2
#define kTK_NUMBER 3
#define kTK_BOOL 4
#define kTK_KEYWORD 5
#define kTK_OPERATOR 6


//==============================================
#pragma mark VALUES & SYMBOLS
//==============================================

#define kAttrEnumerable 1
#define kAttrWrittable 2
#define kAttrHasSetter 4
#define kAttrHasGetter 8


#define kObjTypeFunc 1
#define kObjTypeFuncNative 2
#define kObjTypeObject 3
#define kObjTypeObjectNative 3

#define kValueNULL 0
#define kValueUndefined 999
#define kValueNumber 1
#define kValueString 2
#define kValueBool 3
#define kValueFunc 4
#define kValueFuncNative 5
#define kValueObject 6


#define kFNIsNamed = 0x80
#define kFNIsAnon = 0x40
#define kFNArgIsOptional = 0x80
#define kFNArgIsArray = 0x40

//==============================================
#pragma mark STATEMENT TYPES
//==============================================

#define kST_NOP 0
#define kST_EOF 999

#define kST_BUILTIN 1
#define kST_IDENTIFIER 2
#define kST_BLOCK 3
#define kST_OP 4
#define kST_LITERAL 5

//other builtin statements

#define kST_GETREF 30   //resolve a value reference
#define kST_EXPR 31
#define kST_VARINIT 32
#define kST_ARGINIT 33
#define kST_ASSIGNMENT 34
#define kST_FNCALL 35


#define kST_FUNC 103
#define kST_OBJ 104
#define kST_OBJMEMBER 105
#define kST_RETURN 106
#define kST_OBJECT 107

//==============================================
#pragma mark KEYWORDS
//==============================================
/* see langKeywords[] in langParser.c */

#define kKEWORDS_START 200


#define kST_VAR 	kKEWORDS_START
#define kST_LET 	kKEWORDS_START + 1
#define kST_IF 		kKEWORDS_START + 2
#define kST_ELSE 	kKEWORDS_START + 3
#define kST_WHILE   kKEWORDS_START + 4


/* End Keywords */


#define kST_IF_ELSE 3

#define kST_DO 5
#define kST_FOR 4
#define kST_CONTINUE 7
#define kST_BREAK 8
#define kST_SWITCH 9
#define kST_CASE 13
#define kST_DEFAULT 14
#endif /* langConstants_h */


//==============================================
#pragma mark OPERATORS
//==============================================
#define opType_ST ';'

#define opType_Assign '='
#define opType_Equal 340 //==

#define opType_Comma ','
#define opType_Colon ':'
#define opType_Dot '.'

#define opType_Pow '^'
#define opType_LP '('
#define opType_RP ')'
#define opType_LKey '{'
#define opType_RKey '}'
#define opType_LBracket '['
#define opType_RBracket ']'
#define opType_Modulo '%'
#define opType_Plus '+'
#define opType_Minus '-'
#define opType_Multiply '*'
#define opType_Divide '/'


#define opType_Less '<'
#define opType_LessEqual 331
#define opType_NotEqual 332

#define opType_Greater '>'
#define opType_GreaterEqual 333
#define opType_LogicalEqual 334


#define opType_Neg '!'
#define opType_Inc 341
#define opType_Dec 342
#define opType_Add_Assign 343
#define opType_Plus_Assign 344
#define opType_Subtract_Assign 345
#define opType_Multiply_Assign 346
#define opType_Divide_Assign 347

#define opType_AND 360
#define opType_OR 361



