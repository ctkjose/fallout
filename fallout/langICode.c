//
//  langStatements.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/22/21.
//

#include "langICode.h"
#include "langRuntime.h"



void icodeFree(ICODE e){
	if(!e){
		return;
	}
	if(e->args){
		icodeTableFree(e->args);
	}
	
	
	free(e->name);
	free(e->proto);
	free(e->object);
	free(e);
	
}
ICODE icodeCreate(int type){
	ICODE e;
	e = (ICODE) malloc(sizeof(struct langIcode));
	
	e->type = type;
	e->name = NULL;
	e->proto = NULL;
	e->object = NULL;
	e->args = NULL;
	e->code=0;
	e->flags=0;
	e->srcLine=0;
	return e;
}
void icodePush(ICODE owner, ICODE e){
	
	if(!e || !owner) return;
	
	if(!owner->args){
		owner->args = icodeTableCreate();
	}
	
	ICODE *elems;
	ICTAB table = owner->args;
	if(!table) return;
	
	if (table->length == 0) {
		elems = malloc(sizeof(ICODE));
		if(!elems) return;
		*elems = e;
		table->items = elems;
		table->length = 1;
	} else {
		elems = realloc(table->items, (table->length + 1) * sizeof(ICODE));
		if(!elems) return;
		table->items = elems;
		elems += table->length++;
		*elems = e;
	}
}


ICTAB icodeTableCreate(){
	ICTAB table;
	table = (ICTAB) malloc(sizeof(struct langIcodeTable));
	table->idx = -1;
	table->length = 0;
	table->position = -1;
	table->items = NULL;
	return table;
}

void icodeTableFree(ICTAB table){
	unsigned int i;
	
	if (!table) {
		return;
	}
	
	for (i = 0; i < table->length; i++) {
		icodeFree(table->items[i]);
	}
	free(table->items);
}

void icodeTablePush(ICTAB table, ICODE e){
	ICODE *elems;
	
	if(!table || !e) return;
	
	if (table->length == 0) {
		elems = malloc(sizeof(ICODE));
		if(!elems) return;
		*elems = e;
		table->items = elems;
		table->length = 1;
	} else {
		elems = realloc(table->items, (table->length + 1) * sizeof(ICODE));
		if(!elems) return;
		table->items = elems;
		elems += table->length++;
		*elems = e;
	}
}

int icodeTableAcceptOperator(ICTAB table, int op){
	ICODE ic = icodeTablePop(table);
	if(!ic) return 0;
	
	if(ic->type != kST_OP){
		icodeTableRestore(table, -1);
		return 0;
	}
	if(ic->code != op){
		icodeTableRestore(table, -1);
		return 0;
	}
	
	return 1;
}

ICODE icodeTablePop(ICTAB table){
	if(!table) return NULL;
	if(table->length <= 0) return NULL;
	if(table->position + 1 >= table->length) return NULL;
	
	return table->items[++table->position];
}
ICODE icodeTableGet(ICTAB table, int c){
	if(!table) return NULL;
	if(table->length <= 0) return NULL;
	
	if(table->position + c >= table->length) return NULL;
	table->position += c;
	return table->items[table->position];
}

void icodeTableRestore(ICTAB table, int c){
	if(!table) return;
	if(table->length <= 0) return;
	
	int p = table->position + c;
	if(p >= table->length) p = table->length-1;
	if(p < 0) p = -1;
	
	table->position = p;
}
void icodeTableRewind(ICTAB table){
    
    int i;
    if(!table) return;
    table->position = -1;
    
    
    for (i = 0; i < table->length; i++) {
        ICODE ic;
        ic = table->items[i];
        
        if(ic->args){
            icodeTableRewind(ic->args);
        }
    }
}



#pragma VALUES and SYMBOLS

VALUE valueCreate(langSTATE st, int type){
	VALUE value;
	size_t sz = sizeof(SYMBOL);
	value = (VALUE) malloc(sz);
	if(!value){
		langRaiseRuntimeError(st, "Unable to allocate memory for value");
		return NULL;
	}
	value->type = type;
	value->refCount = 0;
	
	value->attr = 0;
	value->refCount = 0;
	
	value->name = NULL;
	value->parent = NULL;
	value->next = NULL;
	
	
	return value;
}

void valueFree(VALUE value){
	
	if(value->name){
		free(value->name);
	}

	free(value);
}



VALUE valueMakeNULL(langSTATE st){
	VALUE value = valueCreate(st, kValueNULL);
	if(!value) return NULL;
	value->value.number = 0;
	
	return value;
}
VALUE valueMakeUndefined(langSTATE st){
	VALUE value = valueCreate(st, kValueUndefined);
	if(!value) return NULL;
	value->value.number = 0;
	
	return value;
}
VALUE valueMakeNumber(langSTATE st, double v){
	VALUE value = valueCreate(st, kValueNumber);
	if(!value) return NULL;
	value->value.number = v;
	
	return value;
}
VALUE valueMakeBool(langSTATE st, int v){
	VALUE value = valueCreate(st, kValueBool);
	if(!value) return NULL;
	value->value.boolean = v;
	
	return value;
}
VALUE valueMakeString(langSTATE st, char *v){
	VALUE value = valueCreate(st, kValueString);
	if(!value) return NULL;
	value->value.string = strdup(v);
	return value;
}


void valueAssign(langSTATE st, VALUE var, VALUE value){
    if(!st) return;
    
    int t = (value ? value->type : kValueUndefined);
    
    
    var->type = t;
    //var->name = strdup(name);
    
    if(value->type == kValueBool){
        var->value.boolean = value->value.boolean;
    }else if(value->type == kValueNumber){
        var->value.number = value->value.number;
    }else if(value->type == kValueString){
        if(value->value.string){
            var->value.string = strdup(value->value.string);
        }else{
            var->value.string = NULL;
        }
    }else if(value->type == kValueNULL || value->type == kValueUndefined){
        var->value.number = 0;
    }
    
    
}

VALUE valueMakeVariable(langSTATE st, char *name, VALUE value){
	if(!st) return NULL;
	int t = (value ? value->type : kValueUndefined);
	
	VALUE var = valueCreate(st, t);
	if(!var) return NULL;
	
	var->type = t;
	var->name = strdup(name);
	
	if(value->type == kValueBool){
		var->value.boolean = value->value.boolean;
	}else if(value->type == kValueNumber){
		var->value.number = value->value.number;
	}else if(value->type == kValueString){
		if(value->value.string){
			var->value.string = strdup(value->value.string);
		}else{
			var->value.string = NULL;
		}
	}else if(value->type == kValueNULL || value->type == kValueUndefined){
		var->value.number = 0;
	}
	
	return var;
}

///Return type constant for a type represented by its name
int valueGetType(const char *typeName){
  //sanity(typespec);

    if (strcmp(typeName, "null") == 0) {
        return kValueNULL;
    } else if (strcmp(typeName, "bool") == 0) {
        return kValueBool;
    } else if (strcmp(typeName, "int") == 0) {
        return kValueNumber;
    } else if (strcmp(typeName, "float") == 0) {
        return kValueNumber;
    } else if (strcmp(typeName, "string") == 0) {
        return kValueString;
    } else if (strcmp(typeName, "function") == 0) {
        return kValueFunc;
    } else if (strcmp(typeName, "object") == 0) {
        return kValueObject;
    } else if (strcmp(typeName, "array") == 0) {
        return kValueNULL;
    }
    
    return kValueNULL;
}
int valueToBool(langSTATE st, VALUE value){
	if(!value) return 0;
	
	if(value->type == kValueBool) return value->value.boolean;
	if(value->type == kValueNumber) return (value->value.number > 0);
	if(value->type == kValueString){
		if(!value->value.string) return 0;
		if(strlen(value->value.string) > 0) return 1;
	}
	if(value->type == kValueNULL) return 0;
	
	return 0;
}
double valueToDouble(langSTATE st, VALUE value){
	if(!value) return 0;
	
	if(value->type == kValueBool) return (double) value->value.boolean;
	if(value->type == kValueNumber) return value->value.number;
	if(value->type == kValueString){
		if(!value->value.string) return 0;
		if(strlen(value->value.string) > 0) return atof(value->value.string);
	}
	if(value->type == kValueNULL) return 0;
	if(value->type == kValueUndefined) return 0;
	
	return 0;
}
char *valueToString(langSTATE st, VALUE value){
	if(!value) return "";
	
	if(value->type == kValueBool) return (value->value.boolean) ? "1" : "0";
	if(value->type == kValueNumber){
		CSTRING(out, kSZ_MAX_PSTR)
		snprintf(out,kSZ_MAX_PSTR,"%f",value->value.number);
		return out;
	}
	if(value->type == kValueString){
		if(!value->value.string) return 0;
		if(strlen(value->value.string) > 0) return strdup(value->value.string);
	}
	
	if(value->type == kValueNULL) return "NULL";
	if(value->type == kValueUndefined) return "undefined";
	
	
	return "";
}


VALUE valueFromStatement(langSTATE st, ICODE e){
	if(!e || !e->ref) return valueMakeNULL(st);
	return e->ref;
}

#pragma mark OBJECTS

OBJECT objectCreate(langSTATE st, int type, char *name, OBJECT *prototype){
	OBJECT obj;
	if(!st) return NULL;
	
	obj = (OBJECT) malloc(sizeof(SYMBOLOBJ));
	obj->type = type;
	obj->id = langCreateID("OBJ", 10);
	obj->symtab = symTabCreate(st);
	
	obj->args = NULL;
	obj->args = NULL;
	obj->thisValue = NULL;
	obj->proto = NULL;
	
	obj->data = NULL;
	
	//Function like Objects
	obj->args = symTabCreate(st);
	obj->statements = icodeTableCreate();

	
	if(name){
		strncpy(obj->name, name, kSZ_MAX_FN_NAME);
	}else{
		strncpy(obj->name, obj->id, kSZ_MAX_FN_NAME);
	}
	
#pragma mark TODO Implement proto
	
	return obj;
}

void objectFree(OBJECT obj){
	unsigned int i;
	
	if (!obj) {
		return;
	}
	
	if(obj->type == kObjTypeFunc){
		icodeTableFree(obj->statements);
		symTabFree(obj->args);
	}
	
	symTabFree(obj->symtab);
	
	if(obj->thisValue){
		obj->thisValue->refCount--;
		obj->thisValue = NULL;
	}
	
	free(obj);
}

void objectSetThis(OBJECT obj, VALUE var){
	if(!obj || !var) return;
	
	obj->thisValue = var;
	var->refCount++;
}
VALUE valueMakeObject(langSTATE st, int type, char *name){

	if(!st) return NULL;
	
	OBJECT obj = objectCreate(st, type, name, NULL);
	if(!obj) return NULL;
	
	VALUE value = valueCreate(st, kValueObject);
	if(!value){
		objectFree(obj);
		return NULL;
	}
	
	value->name = strdup(obj->name);
	value->value.obj = obj;
	

	return value;
}
VALUE valueMakeCFunction(langSTATE st, fnDefinition fnDef){
	if(!st) return NULL;
	
	VALUE value = valueMakeObject(st, kObjTypeFuncNative, fnDef.name);
	if(!value) return NULL;
	
	OBJECT obj = value->value.obj;
	
	obj->imp.cfn.argc = fnDef.argc;
	obj->imp.cfn.callback = fnDef.callback;
	obj->args = symTabCreate(st);
	
	return value;
}
void objectAddProperty(langSTATE st, OBJECT obj, VALUE value){
	if(!st) return;
	if(!value) return;
	
	symTabInsert(st, obj->symtab, value);

}
#pragma mark FUNCTIONS



FUNCTION functionCreate(langSTATE st, char *name){
	FUNCTION fn;
	if(!st) return NULL;
	
	fn = objectCreate(st, kObjTypeFunc, name, NULL);
	if(!fn) return NULL;
	
	fn->args = symTabCreate(st);
	fn->statements = icodeTableCreate();

	return fn;
}

void functionFree(FUNCTION fn){
	unsigned int i;
	
	if (!fn) {
		return;
	}
	
	icodeTableFree(fn->statements);
	symTabFree(fn->args);
	symTabFree(fn->symtab);
	
	free(fn);
}
void functionPush(FUNCTION fn, ICODE ic){
	
	if(!fn || !ic || !fn->statements) return;

	icodePush(fn->statements, ic);
}



#pragma mark SYMTAB


VALUES symTabCreate(langSTATE st){
	VALUES values;
	values = (VALUES) malloc(sizeof(SYMBOLS));
	if(!values){
		langRaiseRuntimeError(st, "Unable to allocate memory");
		return NULL;
	}
	values->length = 0;
	values->items = malloc(sizeof(struct langSymbol *));
	values->items[0] = NULL;
	
	return values;
}
void symTabFree(VALUES values){
	if(!values){
		return;
	}
	
	for (int i = 0; i < values->length; i++) {
		valueFree(values->items[i]);
	}

}
int symTabInsert(langSTATE st, VALUES symtab, VALUE value){
	if(!symtab){
		langRaiseRuntimeError(st, "Reference to a NULL value used");
		return 0;
	}
	if(!symtab){
		langRaiseRuntimeError(st, "Reference to a NULL value used");
		return 0;
	}
	
	VALUE *entries;
	
	
	entries = realloc(symtab->items, (symtab->length+1) * sizeof(VALUE));
	if(!entries){
		langRaiseRuntimeError(st, "Unable to allocate memory for value");
		return 0;
	}
	printf("[symTabInsert][count=%i][sym=%s]\n", symtab->length, value->name);
	
	entries[symtab->length] = value;
	symtab->length++;
	symtab->items = entries;
	
	return 1;
}

VALUE symTabLookup(langSTATE st, VALUES symtab, char *name){
	if(!symtab){
		return NULL;
	}
	
	for(int i=0; i < symtab->length; i++){
		VALUE v = symtab->items[i];
		if(!v) continue;
		if(!v->name) continue;
		if(strcmp(v->name, name) == 0){
			return v;
		}
	}
	
	return NULL;
}




#pragma mark DEBUG & DUMP

void debugDumpObject(VALUE value, int lvl){
	
	int i = 0;
	char tab[50];
	for(i=1;i<=lvl;i++){
		tab[i-1] = '\t';
	}
	tab[i-1]='\0';
	
	OBJECT obj = value->value.obj;
	
	printf("%s name:[%s] ", tab, value->name);
	if(obj->type == kObjTypeObject){
		printf("(object)");
	}else if(obj->type == kObjTypeFunc){
		printf("(function)");
	}else if(obj->type == kObjTypeFuncNative){
		printf("(native function)");
	}else if(obj->type == kObjTypeObjectNative){
		printf("(native object)");
	}
	
	printf("\n");
	if(obj->symtab){
		debugDumpSymbols(obj->symtab, "fn.symbols", lvl+1);
	}
	
	if(obj->type == kObjTypeFunc && obj->args){
		debugDumpSymbols(obj->args, "fn.args", lvl+1);
	}
	
	
	if(obj->statements){
		debugDumpStatements(obj->statements, "expression", lvl+1);
	}
	
}
void debugDumpSymbol(VALUE value, char *prefix, int lvl ){
	
	int i = 0;
	char tab[50];
	for(i=1;i<=lvl;i++){
		tab[i-1] = '\t';
	}
	tab[i-1]='\0';
	
	
	if(!value){
		printf("%s ERROR <NULL>\n", tab);
		return;
	}
	
	if(value->type == kValueString){
		printf("%s name:[%s] value:[%s]\n", tab, value->name, value->value.string);
	}else if(value->type == kValueNumber){
		printf("%s (number) name:[%s] value:[%f]\n", tab, value->name, value->value.number);
	}else if(value->type == kValueBool){
		printf("%s (boolean) name:[%s] value:[%d]\n", tab, value->name, value->value.boolean);
	}else if(value->type == kValueUndefined){
		printf("%s (undefined) name:[%s]\n", tab, value->name);
	}else if(value->type == kValueNULL){
		printf("%s (null) name:[%s] \n", tab, value->name);
	
	}else if(value->type == kValueObject){
		OBJECT obj = value->value.obj;
		if(obj->type == kObjTypeObject){
			printf("%s (object) name:[%s] \n", tab, value->name);
		}else if(obj->type == kObjTypeFunc){
			printf("%s (function) name:[%s] \n", tab, value->name);
		}else if(obj->type == kObjTypeFuncNative){
			printf("%s (native function) name:[%s] \n", tab, value->name);
		}else if(obj->type == kObjTypeObjectNative){
			printf("%s (native object) name:[%s] \n", tab, value->name);
		}else{
			printf("%s (object) name:[%s] \n", tab, value->name);
		}
		if(obj->symtab){
			debugDumpSymbols(obj->symtab, "obj.symbols", lvl+1);
		}
		
		if(obj->type == kObjTypeFunc && obj->args){
			debugDumpSymbols(obj->args, "fn.args", lvl+1);
		}
		//debugDumpObject(value, lvl);
	}else{
		printf("%s (other)\n", tab);
	}
}
void debugDumpSymbols(VALUES table, char *prefix, int lvl){
	if (!table) return;
	
	int i = 0;
	char tab[50];
	for(i=1;i<=lvl;i++){
		tab[i-1] = '\t';
	}
	tab[i-1]='\0';
	
	printf("%s%s (count: %d)\n", tab,prefix, table->length);
	for (i = 0; i < table->length; i++) {
		VALUE value;
		value = table->items[i];
		debugDumpSymbol(value, prefix, lvl);
	}
}
void debugDumpStatements(ICTAB table, char *prefix, int lvl){
   if (!table) return;
	
	int i = 0;
	char tab[50];
	for(i=0;i<lvl;i++){
		tab[i] = '\t';
	}
	
	tab[i]='\0';
	
	//printf("%s%s (count: %d) ---------------------\n", tab, prefix, table->length);
		
	
   for (i = 0; i < table->length; i++) {
	   ICODE e;
	   e = table->items[i];
	   //printf("%s%s[%d] ICODE:TYPE[%d]\n", tab, prefix, i,e->type);
	   if(e->type == kST_LITERAL){
		   
		   if(!e->ref) continue;
		   if(e->ref->type == kValueString){
			   printf("%s%s[%d] LITERAL-STRING (%d) value:[%s]\n", tab, prefix, i, e->type, e->ref->value.string);
		   }else if(e->ref->type == kValueNumber){
			   printf("%s%s[%d] LITERAL-NUMBER (%d) value:[%f]\n", tab, prefix, i, e->type, e->ref->value.number);
		   }else if(e->ref->type == kValueBool){
			   printf("%s%s[%d] LITERAL-BOOLEAN (%d) value:[%d]\n", tab, prefix, i, e->type, e->ref->value.boolean);
		   }else if(e->ref->type == kValueUndefined){
			   printf("%s%s[%d] UNDEFINED (%d)\n", tab, prefix, i, e->type);
		   }else if(e->ref->type == kValueNULL){
			   printf("%s%s[%d] NULL (%d)\n", tab, prefix, i, e->type);
		   }
	   }else if(e->type == kST_GETREF){
		   printf("%s%s[%d] GETREF (%d) (argc:%d)\n", tab, prefix, i, e->type, e->args->length);
		   debugDumpStatements(e->args, "stmnt", lvl+1);
	   }else if(e->type == kST_VARINIT){
		   printf("%s%s[%d] VARINIT (%d) (argc:%d)\n", tab, prefix, i, e->type, e->args->length);
		   debugDumpStatements(e->args, "stmnt", lvl+1);
	   }else if(e->type == kST_EXPR){
		   printf("%s%s[%d] EXPR (%d) (argc:%d)\n", tab, prefix, i, e->type, e->args->length);
		   debugDumpStatements(e->args, "stmnt", lvl+1);
	   }else if(e->type == kST_IF){
		   printf("%s%s[%d] IF (%d) (argc:%d)\n", tab, prefix, i, e->type, e->args->length);
		   debugDumpStatements(e->args, "stmnt", lvl+1);
	   }else if(e->type == kST_BLOCK){
		   printf("%s%s[%d] STMT-BLOCK (%d) (argc:%d)\n", tab, prefix, i, e->type, e->args->length);
		   debugDumpStatements(e->args, "stmnt", lvl+1);
		   
	   }else if(e->type == kST_OP){
		   char op = (e->code <= 127) ? (char) e->code : 32;
		   printf("%s%s[%d] OPERATOR (%d) [%d][%c]\n", tab, prefix, i, e->type, e->code, op);
	   }else if(e->type == kST_FNCALL){
		   printf("%s%s[%d] FN-CALL (%d) [%s]\n", tab, prefix, i, e->type, e->name);
		   debugDumpStatements(e->args, "argument", lvl+1);
	   }else if(e->type == kST_ASSIGNMENT){
		   printf("%s%s[%d] ASSIGNMENT (%d)\n", tab, prefix, i, e->type);
		   debugDumpStatements(e->args, "expression", lvl+1);
	   }else if(e->type == kST_OBJMEMBER){
		   printf("%s%s[%d] kST_OBJMEMBER (%d) [%s]\n", tab, prefix, i,e->type, e->name);
		   debugDumpStatements(e->args, "member", lvl+1);
	   }else if(e->type == kST_IDENTIFIER){
			printf("%s%s[%d] kST_IDENTIFIER (%d) [%s]\n", tab, prefix, i,e->type, e->name);
	   }else if(e->type == kST_BUILTIN){
		   switch (e->code) {
			   case kST_GETREF:
				   printf("%s%s[%d] INTERNAL-GETREF\n", tab, prefix, i);
				   debugDumpStatements(e->args, "ICODE", lvl+1);
				   break;
			   case kST_VAR:
				   printf("%s%s[%d] INTERNAL-VARINIT\n", tab, prefix, i);
				   debugDumpStatements(e->args, "ICODE", lvl+1);
				   break;
				   
			   default:
				   break;
		   }
	   }else{
		   printf("%s%s[%d] (other):TYPE[%d]\n", tab, prefix, i,e->type);
	   }
   }
}
void debugDumpFunction(FUNCTION fn, int lvl){
	int i = 0;
	char tab[50];
	for(i=1;i<=lvl;i++){
		tab[i-1] = '\t';
	}
	tab[i-1]='\0';
	
	printf("%sfn (ID:%s)\n", tab, fn->id);
	
	debugDumpSymbols(fn->args, "fn.args", lvl+1);
	debugDumpSymbols(fn->symtab, "fn.symbols", lvl+1);
	
	debugDumpStatements(fn->statements, "fn.icode", lvl+1);
}
