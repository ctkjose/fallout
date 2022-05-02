//
//  langRuntime.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/22/21.
//

#include <stdlib.h>

#include "langRuntime.h"
#include "langParser.h"
#include "langICode.h"
#include "langFramework.h"


void runtimeRaiseError(langSTATE st, char *msg){
	
	size_t sz = strlen(msg)+30;
	char *errmsg = (char *) malloc(sz * sizeof(char));
	snprintf(errmsg, sz, "Runtime Error: [Line: %d] %s\n", st->scope->srcLine, msg);
	
	
	langRaiseRuntimeError(st, errmsg);
}


SCOPE runtimeScopeCreate(langSTATE st, SCOPE parent){
	SCOPE scope = (SCOPE) malloc(sizeof(struct langScope));
	if(!scope) return scope;
	
	scope->st = st;
	scope->symtab = symTabCreate(st);
	scope->returnValue = valueMakeUndefined(st);
	scope->fnArgsPosition = 0;
	
	if(parent){
		scope->parent = parent;
	}else{
		scope->parent = NULL;
	}
	
	
	return scope;
}

void runtimeScopePush(langSTATE st, SCOPE scope){
	if(!st) return;
	
	if(st->scopeSize + 1 >= kSZ_MAX_SCOPE_DEPTH ){
		runtimeRaiseError(st,"Stack overflow!");
		return;
	}
	st->scopeStack[++st->scopeIdx] = scope;
	st->scopeSize++;
	
	st->scope = scope;
}
void runtimeScopePop(langSTATE st){
	if(!st) return;
	
	if(st->scopeIdx <= 0){
		return;
	}
	st->scopeIdx--;
	st->scopeSize--;
	
	st->scope = st->scopeStack[st->scopeIdx];
	
}


VALUE stackExecExpressionSimple(langSTATE st, ICTAB table){
	
	VALUE value1 = stackExecExpressionTerm(st, table);
	
	if(!value1){
		return NULL;
	}
	
	ICODE ic = icodeTablePop(table);
	if( !ic || !is_op(ic) || !is_op_additive(ic->code) ){
		if(ic) icodeTableRestore(table, -1);
		return value1;
	}
	
	
	double v2 = 0;
	double v1 = valueToDouble(st, value1);
	
	VALUE value2;
	
	VALUE vReturn = valueCreate(st, kValueNumber);
	vReturn->value.number = 0;
	
	int isBool = 0;
	do {
		__sync_debug_info(ic)
		value2 = stackExecExpressionFactor(st, table);
	
		if(ic->code == opType_Multiply){
			isBool = 0;
			v2 = valueToDouble(st, value2);
			v1 = v1 * v2;
			
			
		}else if(ic->code == opType_Plus){
			isBool = 0;
			v2 = valueToDouble(st, value2);
			v1 = v1 + v2;
			
			vReturn->value.number = v1;
		}else if(ic->code == opType_OR){
			isBool = 1;
			v1 = (v1 >= 0);
			v2 = valueToBool(st, value2);
			v1 = (v1 || v2) ? 1 : 0;
		}
		
		ic = icodeTablePop(table);
	}while(ic && is_op(ic) && is_op_additive(ic->code));
	
	if(ic){
		icodeTableRestore(table, -1);
	}
	
	if(isBool){
		vReturn->type = kValueBool;
		vReturn->value.boolean = v1;
	}else{
		vReturn->value.number = v1;
	}
	
	return vReturn;
	
}
VALUE stackExecExpressionTerm(langSTATE st, ICTAB table){
	
	VALUE value1 = stackExecExpressionFactor(st, table);
	
	if(!value1){
		return NULL;
	}
	
	ICODE ic = icodeTablePop(table);
	if( !ic || !is_op(ic) || !is_op_multiplicative(ic->code) ){
		if(ic) icodeTableRestore(table, -1);
		return value1;
	}
	
	
	double v2 = 0;
	double v1 = valueToDouble(st, value1);
	
	VALUE value2;
	
	VALUE vReturn = valueCreate(st, kValueNumber);
	vReturn->value.number = 0;
	
	int isBool = 0;
	do {
		__sync_debug_info(ic)
		value2 = stackExecExpressionFactor(st, table);
	
		if(ic->code == opType_Multiply){
			isBool = 0;
			v2 = valueToDouble(st, value2);
			v1 = v1 * v2;
			
			
		}else if(ic->code == opType_Divide){
			isBool = 0;
			v2 = valueToDouble(st, value2);
			v1 = v1 * v2;
			
			vReturn->value.number = v1;
		}else if(ic->code == opType_Modulo){
			isBool = 1;
			v2 = valueToDouble(st, value2);
			v1 = (double) ((int)v1 % (int)v2);
			
			vReturn->value.number = v1;
			
		}else if(ic->code == opType_AND){
			isBool = 1;
			v1 = (v1 >= 0);
			v2 = valueToBool(st, value2);
			v1 = (v1 && v2) ? 1 : 0;
		}
		
		
		ic = icodeTablePop(table);
	}while(ic && is_op(ic) && is_op_multiplicative(ic->code));
	
	if(ic){
		icodeTableRestore(table, -1);
	}
	
	if(isBool){
		vReturn->type = kValueBool;
		vReturn->value.boolean = v1;
	}else{
		vReturn->value.number = v1;
	}
	
	return vReturn;
	
	
}
VALUE stackExecExpressionFactor(langSTATE st, ICTAB table){
	
	VALUE rValue = NULL;
	ICODE ic = icodeTablePop(table);
	
	if(!ic) return NULL;
	__sync_debug_info(ic)
	
	if(ic->type == kST_GETREF){
		rValue = statementGetRef(st, ic->args, NULL);
		return rValue;
	}else if(ic->type == kST_LITERAL){
		return ic->ref;
	}else if(ic->type == kST_OP){
		
		if(ic->code == opType_Neg){
			rValue = stackExecExpressionFactor(st, table);
#pragma mark TODO negate rValue;
		}else if(ic->code == opType_LP){
			rValue = stackExecExpression(st, table);
			int b = !valueToBool(st, rValue);
			rValue = valueMakeBool(st, b);
			
			if(!icodeTableAcceptOperator(table, opType_RP)){
				runtimeRaiseError(st, "Expecting operator `]` in expression factor.");
				return rValue;
			}
				
		}
	}
	
	return rValue;
}
int runtimeCompareValue(langSTATE st, int op, VALUE l, VALUE r){
	
#pragma mark TODO logic for AND/OR
	
	if(l->type == kValueString){
		char *s1 = valueToString(st, l);
		char *s2 = valueToString(st, r);
		
		int sv = strcmp(s1, s2);
		
		if(op == opType_LogicalEqual) return (sv == 0);
		if(op == opType_NotEqual) return (sv != 0);
		if(op == opType_Less) return (sv < 0);
		if(op == opType_LessEqual) return (sv <= 0);
		if(op == opType_Greater) return (sv > 0);
		if(op == opType_GreaterEqual) return (sv >= 0);
		return 0;
	}else if(l->type == kValueNumber ){
		double v1 = valueToDouble(st, l);
		double v2 = valueToDouble(st, r);
		
		if(op == opType_LogicalEqual) return (v1 == v2);
		if(op == opType_NotEqual) return (v1 != v2);
		if(op == opType_Less) return (v1 < v2);
		if(op == opType_LessEqual) return (v1 <= v2);
		if(op == opType_Greater) return (v1 > v2);
		if(op == opType_GreaterEqual) return (v1 >= v2);
		return 0;
	}else if( l->type == kValueBool ){
		double v1 = valueToBool(st, l);
		double v2 = valueToBool(st, r);
		
		if(op == opType_LogicalEqual) return (v1 == v2);
		if(op == opType_NotEqual) return (v1 != v2);
		if(op == opType_Less) return (v1 < v2);
		if(op == opType_LessEqual) return (v1 <= v2);
		if(op == opType_Greater) return (v1 > v2);
		if(op == opType_GreaterEqual) return (v1 >= v2);
		return 0;
	}
	
		
	return 0;
}
VALUE stackExecExpression(langSTATE st, ICTAB table){
	
	VALUE value1 = stackExecExpressionSimple(st, table);
	
	if(!value1){
		return NULL;
	}
	
	ICODE ic = icodeTablePop(table);
	
	
	if( !ic || !is_op(ic) || !is_op_logical(ic->code) ){
		if(ic) icodeTableRestore(table, -1);
		return value1;
	}
	
	
	VALUE vReturn = valueCreate(st, kValueBool);
	vReturn->value.boolean = valueToBool(st, value1);
	   
	do {
		__sync_debug_info(ic)
		vReturn->value.boolean = runtimeCompareValue(st, ic->code, value1, stackExecExpressionTerm(st, table));
		ic = icodeTablePop(table);
	}while(ic && is_op(ic) && is_op_logical(ic->code));
	
	if(ic){
		icodeTableRestore(table, -1);
	}
	
	return vReturn;
}
char * stackGetCString(langSTATE st, ICTAB table){
	VALUE v = stackGetValue(st, table);
	if(!v) return "";
	
	char *s = valueToString(st, v);
	
	return s;
}
int stackGetBool(langSTATE st, ICTAB table){
	VALUE v = stackGetValue(st, table);
	if(!v) return 0;
	
	int b = valueToBool(st, v);
	
	return b;
}
double stackGetNumber(langSTATE st, ICTAB table){
	VALUE v = stackGetValue(st, table);
	if(!v) return 0;
	
	double d = valueToDouble(st, v);
	
	return d;
}

VALUE stackGetValue(langSTATE st, ICTAB table){
	if(!st || !table) return NULL;
	
	ICODE ic = icodeTablePop(table);
	if(!ic) return NULL;
	
	__sync_debug_info(ic)
	
	VALUE v = NULL;
	if(ic->type == kST_EXPR){
		v = stackExecExpression(st, ic->args);
		return v;
	}else if(ic->type == kST_LITERAL){
		return ic->ref;
	}else if(ic->type == kST_GETREF){
		v = statementGetRef(st, ic->args, NULL);
		return v;
	}
	return NULL;
}

int stackGetParameterCount(langSTATE st, ICTAB table){
	SCOPE fnScope = st->scope;
	if(table->length <= 0) return 0;
	
	int c = (table->length - 1) - fnScope->fnArgsPosition;
	
	return c;
}

//First parameter is idx=0
VALUE stackGetParameter(langSTATE st, ICTAB table, int idx){
	SCOPE fnScope = st->scope;
	
	VALUE UndefinedValue = valueCreate(st,kValueUndefined);
	UndefinedValue->value.number = 0;
	
	if(table->length <= 0) return UndefinedValue;
	
	int p = fnScope->fnArgsPosition + idx;
	if(p >= table->length) return UndefinedValue;
	
	table->position = p;
	
	valueFree(UndefinedValue);
	return stackGetValue(st, table);
}
VALUE runtimeExecuteStack(langSTATE st, ICTAB table){
	
	int i;
	SCOPE scope;
	if(st->scopeIdx < 0){
		scope = runtimeScopeCreate(st,NULL);
		runtimeScopePush(st, scope);
	}
	
	//printf("runtimeExecuteICodeTable(SCOPE=%d)\n", st->scopeIdx);
	
	SCOPE scopeFn = NULL;
	for (i = 0; i < table->length; i++) {
		ICODE ic;
		ic = table->items[i];
		__sync_debug_info(ic)
		
		if(ic->type == kST_VARINIT){
			statementVariableInit(st, ic->args);
        }else if(ic->type == kST_ASSIGNMENT){
            statementAssignment(st, ic->args);
		}else if(ic->type == kST_FNCALL){
			statementFNCall(st, ic->args);
		}else if(ic->type == kST_IF){
			statementIF(st, ic->args);
        }else if(ic->type == kST_WHILE){
            statementWHILE(st, ic->args);
		}
	}
		
	return NULL;
}
void runtimeInitState(langSTATE st){
	
	if(!st) return;
	
	if(st->scopeIdx < 0){
		SCOPE scope = runtimeScopeCreate(st,NULL);
		runtimeScopePush(st, scope);
	}
	
	falloutInitFramework(st);
	
	if(st->onInitState){
		(*st->onInitState)(st);
	}
}

void runtimeExecuteState(langSTATE st){
	if(!st) return;
	runtimeInitState(st);
	
	VALUE returnValue = runtimeExecuteStack(st, st->root);
	
	debugDumpSymbols(st->scopeStack[0]->symtab, "ROOTSCOPE", 1);
}
void runtimeExecuteSource(langSTATE st, const char *source){
	langPARSER parser;
	parser = (struct langParseState *) malloc(sizeof(struct langParseState));
	if(!parser) return;
	
	if(!st->root){
		langRaiseRuntimeError(st, "Unable to allocate icode table.");
		return;
	}
	

	parser->st = st;
	
	st->root = parserSource(parser, source);
	debugDumpStatements(st->root, "SCRIPTROOT", 1);
	
	runtimeExecuteState(st);
	
	
	
	return;
}


VALUE runtimeExecuteCFunction(langSTATE st, OBJECT fn, ICTAB table){
	
	VALUE UndefinedValue = valueCreate(st,kValueUndefined);
	UndefinedValue->value.number = 0;
	
	if(!st || !fn) return UndefinedValue;
	
	if(fn->type != kObjTypeFuncNative){
		return UndefinedValue;
	}
	
	int argc = fn->imp.cfn.argc;
	CFUNCTION cfn = fn->imp.cfn.callback;
	
	if(!cfn){
		runtimeRaiseError(st, "RUNTIME ERROR: Unable to execute function. [FNCALL-500]");
		return UndefinedValue;
	}
	
	SCOPE fnScope = runtimeScopeCreate(st, st->scope);
	if(!fnScope){
		runtimeRaiseError(st, "RUNTIME ERROR: Unable to execute function. [FNCALL-501]");
		return UndefinedValue;
	}

	valueFree(UndefinedValue);
	
	fnScope->fnArgsPosition = table->position;
	fnScope->returnValue = UndefinedValue;
	runtimeScopePush(st, fnScope);
	
	(*cfn)(st, table);
	
	runtimeScopePop(st);
	
	return fnScope->returnValue;
}
/*
 Executes a function call, icode FNCALL(35)
 */

void statementIF(langSTATE st, ICTAB table){
	
	if(!st || !table || !st->scope) return;


	int isOk = stackGetBool(st, table);
	
	ICODE block = NULL;
	if(isOk){
		block = icodeTablePop(table);
	}else if(table->length > 1){
		block = icodeTableGet(table, 2);
	}
	
	if(!block || !block->args){
		return;
	}
	
	runtimeExecuteStack(st, block->args);
	return;
}
void statementWHILE(langSTATE st, ICTAB table){
    
    if(!st || !table || !st->scope) return;


   int isOk = stackGetBool(st, table);
    
    
    if(table->length != 2) return;
    
    ICODE block = icodeTablePop(table);
    
    while(isOk){
        
        if(!block || !block->args){
            break;
        }
        
        runtimeExecuteStack(st, block->args);
        
        icodeTableRewind(table);
        isOk = stackGetBool(st, table);
        
        if(isOk) icodeTableRewind(block->args);
    }
        
    return;
}
VALUE statementFNCall(langSTATE st, ICTAB table){
	
	if(!st || !table || !st->scope) return NULL;

	VALUE UndefinedValue = valueCreate(st,kValueUndefined);
	UndefinedValue->value.number = 0;
	
	VALUE fnRef = stackGetValue(st, table);
	
	if(!fnRef) return UndefinedValue;
	
	char msg[256];
	snprintf(msg, 255, "Value '%s' is not callable.", fnRef->name);
	
	if(fnRef->type != kValueObject){
		
		runtimeRaiseError(st, msg);
		return UndefinedValue;
	}
	
	
	OBJECT obj = fnRef->value.obj;
	if(!obj){
		runtimeRaiseError(st, msg);
		return UndefinedValue;
	}
	
	VALUE returnValue = UndefinedValue;
	

	if(obj->type == kObjTypeFunc){
		///valueFree(UndefinedValue);
		return UndefinedValue;
	}else if(obj->type == kObjTypeFuncNative){
		valueFree(UndefinedValue);
		returnValue = runtimeExecuteCFunction(st, obj, table);
		return returnValue;
	}else{
		runtimeRaiseError(st, msg);
		return UndefinedValue;
	}
	
	
}
/*
 Returns a value from an icode GETREF(30)
 */
VALUE statementGetRef(langSTATE st, ICTAB table, VALUE parent){
	
	if(!st || !table || !st->scope) return NULL;
	
	
	char *s = stackGetCString(st, table);
	if(!s) return NULL;
	
	
	VALUE UndefinedValue = valueCreate(st,kValueUndefined);
	UndefinedValue->value.number = 0;
	
	
	char msg[256];
	OBJECT obj = NULL;
	VALUES symtab = NULL;
	
	
	if(parent){
		if(parent->type == kValueObject){
			obj = parent->value.obj;
			if(obj->type == kObjTypeObject){
				symtab = obj->symtab;
			}
		}
		
	}else{
		symtab = st->scope->symtab;
	}
	
	
	if(!symtab){
		//runtimeRaiseError(st, "Unable to resolve reference to value");
		return UndefinedValue;
	}
	
	
	VALUE vRef = symTabLookup(st, symtab, s);
	if(!vRef){
		//snprintf(msg, 255, "Value '%s' is undefined.", s);
		//runtimeRaiseError(st, msg);
		
		return UndefinedValue;
	}
	
	/*
	 if(vRef->type == kValueObject){
		OBJECT obj = vRef->value.obj;
		
		if(obj->type == kObjTypeObject){
			symtab = obj->symtab;
		}
	}
	*/
	
	ICODE ic = icodeTablePop(table);
	if( ic ){
		if(ic->type == kST_GETREF){
			vRef = statementGetRef(st, ic->args, vRef);
		}
	}
	
	
	//debugDumpSymbol(vRef, "object", 1);
	
	if(!vRef) return UndefinedValue;
	return vRef;
}
void statementAssignment(langSTATE st, ICTAB table){
    printf("statementAssignment()\n");

    
    if(!st) return;
    if(!st->scope) return;
    
    VALUE var = stackGetValue(st, table);
    if(!var){
        runtimeRaiseError(st, "Expecting variable.");
        return;
    }
    
    VALUE rValue = (table->length > 1) ? stackGetValue(st, table) : valueMakeNULL(st);
    
    if(!rValue){
        runtimeRaiseError(st, "Expecting value.");
        return;
    }
    
    
    valueAssign(st, var, rValue);
    
}
void statementVariableInit(langSTATE st, ICTAB table){
	printf("statementVariableInit()\n");

	
	if(!st) return;
	if(!st->scope) return;
	
	
	char *name = stackGetCString(st, table);
	if(!name || !strlen(name)){
		runtimeRaiseError(st, "Expecting identifier in variable name.");
		return;
	}
	
	VALUE initValue = stackGetValue(st, table);
	if(!initValue){
		runtimeRaiseError(st, "Expecting initial value.");
		return;
	}
	
	VALUE var = valueMakeVariable(st, name, initValue);
	if(!initValue){
		runtimeRaiseError(st, "Unable to create variable.");
		return;
	}
	
	SCOPE scope = st->scope;
	symTabInsert(st, scope->symtab, var);
	
}

