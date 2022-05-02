//
//  langStatements.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/22/21.
//

#ifndef langStatements_h
#define langStatements_h

#include "langCommon.h"

void icodeFree(ICODE e);
ICODE icodeCreate(int type);
void icodePush(ICODE owner, ICODE e);

ICTAB icodeTableCreate(void);
void icodeTableFree(ICTAB table);

void icodeTablePush(ICTAB table, ICODE e);
ICODE icodeTablePop(ICTAB table);
ICODE icodeTableGet(ICTAB table, int c);
void icodeTableRestore(ICTAB table, int c);
int icodeTableAcceptOperator(ICTAB table, int op);
void icodeTableRewind(ICTAB table);

#pragma mark CODE STATEMENTS
/* A CS is an executable block (aggregate) of statements */




#pragma mark VALUES



VALUE valueCreate(langSTATE st, int type);
void valueFree(VALUE value);

int valueGetType(const char *typespec);

VALUE valueMakeUndefined(langSTATE st);
VALUE valueMakeNULL(langSTATE st);
VALUE valueMakeNumber(langSTATE st, double v);
VALUE valueMakeBool(langSTATE st, int v);
VALUE valueMakeString(langSTATE st, char *v);

VALUE valueMakeVariable(langSTATE st, char *name, VALUE value);
VALUE valueMakeCFunction(langSTATE st, fnDefinition fnDef);
VALUE valueMakeObject(langSTATE st, int type, char *name);

void valueAssign(langSTATE st, VALUE var, VALUE value);

int valueToBool(langSTATE st, VALUE value);
double valueToDouble(langSTATE st, VALUE value);
char *valueToString(langSTATE st, VALUE value);
VALUE valueFromStatement(langSTATE st, ICODE e);



#pragma mark SYMTAB
VALUES symTabCreate(langSTATE st);
void symTabFree(VALUES values);
int symTabInsert(langSTATE st, VALUES symtab, VALUE value);
VALUE symTabLookup(langSTATE st, VALUES symtab, char *name);

#pragma mark OBJECTS
OBJECT objectCreate(langSTATE st, int type, char *name, OBJECT *prototype);
void objectFree(OBJECT obj);
void objectSetThis(OBJECT obj, VALUE var);
void objectAddProperty(langSTATE st, OBJECT obj, VALUE value);

#pragma mark FUNCTIONS


FUNCTION functionCreate(langSTATE st, char *name);
void functionFree(FUNCTION fn);


#pragma DEBUG

void debugDumpFunction(FUNCTION fn, int lvl);
void debugDumpStatements(ICTAB table, char *prefix, int lvl);
void debugDumpSymbols(VALUES table, char *prefix, int lvl);
void debugDumpSymbol(VALUE value, char *prefix, int lvl );

#endif /* langStatements_h */
