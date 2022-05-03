//
//  langTypes.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/22/21.
//

#ifndef langTypes_h
#define langTypes_h

#include "langLimits.h"

#pragma mark GENERAL



typedef struct langDictEntry hashItem;
struct langDictEntry {
	const char* key;  // key is NULL if this slot is empty
	void* value;
};

typedef struct langDictionary hashDictionary;
typedef hashDictionary * DICTIONARY;
struct langDictionary {
	hashItem* entries;  // hash slots
	size_t capacity;    // size of _entries array
	size_t length;      // number of items in hash table
};

// Hash table iterator: create with ht_iterator, iterate with ht_next.
typedef struct {
	const char* key;  // current key
	void* value;      // current value

	// Don't use these fields directly.
	hashDictionary* _table;       // reference to hash table being iterated
	size_t _index;    // current index into ht._entries
} langDictIterator;

#pragma mark STATE

typedef struct langState * langSTATE;
typedef void (*FNHOOK)(langSTATE st);
typedef void (*FNERRORHANDLER)(langSTATE st, char*);


#pragma mark PARSER
typedef struct langError ERROR;
typedef struct langNode * langNODE;
typedef struct langParseState * langPARSER;

#pragma mark VALUES




typedef struct langOBJECT SYMBOLFN;
typedef struct langOBJECT SYMBOLOBJ;
typedef SYMBOLFN * FUNCTION;
typedef SYMBOLOBJ * OBJECT;

typedef struct langSymbol SYMBOL;
typedef SYMBOL * SYM;
typedef SYMBOL * VALUE;

typedef struct langSymbols SYMBOLS;
typedef SYMBOLS * VALUES;




#pragma mark STATEMENTS

typedef struct langIcode *ICODE;
typedef struct langIcodeTable *ICTAB;


typedef struct langScope *SCOPE;


#pragma mark C-BRIDGE

typedef void (* CFUNCTION)(langSTATE, ICTAB);
typedef struct langFuncDefinition fnDefinition;
typedef fnDefinition * FNDEF;

typedef struct langParameter fnParameter;


typedef struct langObjectDefinition objDefinition;
typedef objDefinition * OBJDEF;

#pragma mark OBJECTS

#define kOBJT


#pragma mark STRUCTS


struct langState {
	int data;
	int abort;
	int currentLine;
	ICTAB root;
	
	int scopeIdx;
	int scopeSize;
	SCOPE scope;
	SCOPE scopeStack[kSZ_MAX_SCOPE_DEPTH];
	
	FNHOOK onInitState;
	FNERRORHANDLER onRuntimeError;
	FNERRORHANDLER onSyntaxError;
	FNERRORHANDLER onConsolePrint;
};


struct langNode {
	const char *text;
	int keyword;
	int type;
	int srcLine;
	int operator;
    long vInteger;
    double vFloat;
};

struct langError {
	int code;
	int data;
	int fatal;
	int srcLine;
	char *srcFile;
	char *msg;
};

struct langParseState {
	langSTATE st;
	const char *src;
	int srcIDX;
	int srcLine;
	int srcLen;
	int abort;
	int flgInBlock;
	int flgInFunction;
	char errorMessage[255];
	Character lastCH;
	int lastCHLen;
	struct langNode *lastNode;
	struct langNode *holdNode;
	int hasHoldNode;
};


struct langScope {
	langSTATE st;
    int isRoot;
	VALUES symtab;
	VALUE returnValue;
	SCOPE parent;
	int srcLine;
	int paramCount;
    fnParameter *params;
    
    OBJECT obj;
};

struct langSymbols {
	int length;
    int isRoot;
	VALUE *items;
};

struct langSymbol {
	union {
		int boolean;
		long asInteger;
        double asFloat;
		char *string;  //utf8
		OBJECT obj;
	} value;
	
	char *name;
	int type;
	int attr;
	int refCount;
	VALUE parent;
	VALUE next;
	
};



struct langIcode {
	int type;
	char *name;
	char *proto;
	char *object;
	
	int code;
	int flags;
	int srcLine;
	VALUE ref;
	ICTAB args;

};
struct langIcodeTable {
	int idx;
	int length;
	int position;
	ICODE *items;
};

struct langParameter {
    int idx;
    CString name;
    VALUE value;
    ICODE initValue; //an Expression statement
};

struct langOBJECT {
	char name[kSZ_MAX_FN_NAME + 1];
	char *id;
	int type;
	int flags;

	VALUES symtab;
	VALUE selfValue;
	VALUE proto;
	
	//Function like Objects
    VALUES closure;
	ICTAB statements;
	VALUES args;
    
    int isClosure;
    int paramCount;
    fnParameter *params;
    ICODE fnBlock;
	
	void *data;
	//implementation data
	union {
		struct {
			int argc;
            int flag;
			CFUNCTION callback;
		} cfn; //built-in cfunction
	} imp;
};


#pragma C-BRIDGE

struct langFuncDefinition {
	char * name;
	CFUNCTION callback;
	int argc;
	int flag;
};

struct langObjectDefinition {
	char * name; /* Name of object */
	char * proto; /* Name of prototype object */
	CFUNCTION fnInit; /* Initialization function */
	CFUNCTION fn; /* When called like a function */
	int fnArgCount; /* Number of arguments for function */
	CFUNCTION constructor; /*when called with new */
	int consArgCount; /*Number of arguments for constructor */
	
	size_t dataSize; /* Size of our private data*/

	int methodCount; /*how many methods*/
	fnDefinition *methods;
};



#endif /* langTypes_h */
