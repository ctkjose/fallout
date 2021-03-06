//
//  langFramework.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/23/21.
//

#include "langFramework.h"
#include "langICode.h"
#include "langRuntime.h"


void falloutRegisterGlobal(langSTATE st);

void impFnPrint(langSTATE st, ICTAB table);


void falloutInitFramework(langSTATE st){
	if(!st) return;
	
	falloutRegisterGlobal(st);
}
OBJECT falloutRegisterObject(langSTATE st, objDefinition *def){
	if(!st || !def) return NULL;
	
	
	VALUE value = valueMakeObject(st, kObjTypeObject, def->name);
	if(!value) return NULL;
	
	OBJECT obj = value->value.obj;
    
    
    
	//set up as a function
	obj->args = symTabCreate(st);
	obj->statements = icodeTableCreate();
	
	int i=0;
	for(i=0; i < def->methodCount; i++){
		//fnDefinition *fnDef = &def->methods[i];
		objectAddProperty(st, obj, valueMakeCFunction(st, def->methods[i]));
	}
	
	symTabInsert(st, st->scope->symtab, value);
    
    return obj;
}
void falloutRegisterGlobal(langSTATE st){
	
	if(!st) return;
	
	
	//fnDefinition {char * name, CFUNCTION callback, int argc, int flag}
	fnDefinition ObjMethods[] = {
		{"print", impFnPrint, 1, 0},
	};
	
	
	//objDefinition
	//{ char * name,char * proto,CFUNCTION fnInit, CFUNCTION fn, int fnArgCount, CFUNCTION constructor, int consArgCount, size_t dataSize,
	//int methodCount, fnDefinition *methods }
	
	objDefinition ObjDef = {
		"object", NULL, NULL, NULL, 0, NULL, 0, 0, 1, ObjMethods
	};
	

	OBJECT obj = falloutRegisterObject(st, &ObjDef);

    VALUE var = valueMakeVariable(st, "version", valueMakeString(st, "1.0.1"));
    symTabInsert(st, obj->symtab, var);
    
	/*
	fnDefinition fnPrint;
	
	fnPrint.name = (char *) "print";
	fnPrint.flag = 0;
	fnPrint.argc = 1;
	fnPrint.callback = &impFnPrint;
	
	objectAddProperty(st, obj, valueMakeCFunction(st, fnPrint));
	
	symTabInsert(st, st->scope->symtab, global);
	*/
}


void impFnPrint(langSTATE st, ICTAB table){
	FLL_CHECK_STATE()
	
	int argc = FLL_ARGC();
	
	/*
	VALUE vMsg = FLL_ARGV(1);
	if(!FLL_IS_SET(vMsg)){
		langRaiseRuntimeError(st, "Missing message parameter");
		return;
	}
	
	char *s = valueToString(st, vMsg);
	*/
	
    printf("FALLOUT: ");
    for(int i=0; i< argc; i++){
        char *s = FLL_STRING_ARGV(i);
        if(s) printf("%s", s);
    }
    
    printf("\n");
}
