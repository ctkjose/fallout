//
//  langState.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#include "langCommon.h"
#include "langICode.h"


langSTATE langStateCreate(){
	langSTATE st;
	st = (struct langState *) malloc(sizeof(struct langState));
	if(!st){
		langLogMessage("Unable to initialize runtime [ERR LANG001]");
		return NULL;
	}
	st->abort = 0;
	
	st->scope = NULL;
	st->scopeIdx = -1;
	st->scopeSize = 0;
	
	st->root = icodeTableCreate();
	
	
	st->onInitState = NULL;
	st->onRuntimeError = NULL;
	st->onSyntaxError = NULL;
	st->onConsolePrint = NULL;
	
	
	return st;
}

void langAbort(langSTATE st){
	
	st->abort = 1;
	
}
void langRaiseRuntimeError(langSTATE st, char *msg){
	st->abort = 1;
	
	if(st->onRuntimeError){
		(*st->onRuntimeError)(st, msg);
	}
	
	//langStdErrMessage("Fatal Error: [Line: %d] %s\n", st->currentLine, msg);
	//exit(1);
}
void langRaiseSyntaxError(langSTATE st, char *msg){
	st->abort = 1;
	
	if(st->onSyntaxError){
		(*st->onSyntaxError)(st, msg);
	}
	
	langStdErrMessage("Fatal Error: [Line: %d] %s\n", st->currentLine, msg);
	//exit(1);
}

ERROR langCreateError(int errCode, char *msg, int isFatal, int srcLine){
	
	ERROR err;
	err.code = errCode;
	err.srcLine = srcLine;
	err.fatal = isFatal;
	
	size_t sz = strlen(msg)+1;
	err.msg = (char *) malloc(sz * sizeof(char));
	strncpy(err.msg, msg, sz);
	
	return err;
}

void langRaiseError(langSTATE st, ERROR err){
	
	size_t sz = strlen(err.msg) + 30;
	char *msg = (char *) malloc(sz * sizeof(char));
	
	snprintf(msg, sz, "Error: [Line: %d] %s\n", err.srcLine, err.msg);
	if(err.fatal){
		st->abort = 1;
		//abort();
	}
	
	if(err.code== kERR_SYNTAX){
		langRaiseSyntaxError(st, msg);
	}else{
		langRaiseRuntimeError(st, msg);
	}
}

/*
void *js_malloc(js_State *J, int size)
{
	void *ptr = J->alloc(J->actx, NULL, size);
	if (!ptr)
		js_outofmemory(J);
	return ptr;
}
*/
