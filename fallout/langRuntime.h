//
//  langRuntime.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/22/21.
//

#ifndef langRuntime_h
#define langRuntime_h

#include "langCommon.h"

#define is_op(a) (a->type == kST_OP)
#define __sync_debug_info(ic) st->scope->srcLine = ic->srcLine;

void runtimeRaiseError(langSTATE st, char *msg);



SCOPE runtimeScopeCreate(langSTATE st, SCOPE parent);
void runtimeScopePush(langSTATE st, SCOPE scope);
void runtimeScopePop(langSTATE st);

void runtimeInitState(langSTATE st);
void runtimeInitGlobalObject(langSTATE st);
void runtimeExecuteSource(langSTATE st, const char *source);
VALUE runtimeExecuteStack(langSTATE st, ICTAB table);
VALUE runtimeExecuteCFunction(langSTATE st, OBJECT fn, ICTAB table);

int runtimeCompareValue(langSTATE st, int op, VALUE l, VALUE r);

VALUE stackExecExpression(langSTATE st, ICTAB table);
VALUE stackExecExpressionSimple(langSTATE st, ICTAB table);
VALUE stackExecExpressionTerm(langSTATE st, ICTAB table);
VALUE stackExecExpressionFactor(langSTATE st, ICTAB table);



double stackGetNumber(langSTATE st, ICTAB table);
int stackGetBool(langSTATE st, ICTAB table);
char * stackGetCString(langSTATE st, ICTAB table);
VALUE stackGetValue(langSTATE st, ICTAB table);
int stackGetParameterCount(langSTATE st, ICTAB table);
VALUE stackGetParameter(langSTATE st, ICTAB table, int idx);

void statementAssignment(langSTATE st, ICTAB table);
void statementIF(langSTATE st, ICTAB table);
void statementWHILE(langSTATE st, ICTAB table);
VALUE statementFNCall(langSTATE st, ICTAB table);
VALUE statementGetRef(langSTATE st, ICTAB table, VALUE parent);
void statementVariableInit(langSTATE st, ICTAB args);


#endif /* langRuntime_h */
