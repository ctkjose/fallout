#  C Functions


Language methods are implemented with native C-Fucntions.

## Signature 

```c
void aFunction(langSTATE st, ICTAB table){
	//Our code...	
}
```

```c
typedef void (* CFUNCTION)(langSTATE, ICTAB);
```


## Useful macros

Checks if passed state and stack table are valid
```c
FLL_CHECK_STATE()
```

Get number of arguments in stack: 
```cpp
int c = FLL_ARGC(); 
```

Get a value instance for an argument in stack.
```c
VALUE aValue = FLL_ARGV(1);
```

Get primitive values from an argument in the stack.
```c
char *aString = FLL_STRING_ARGV(i);
double aDouble = FLL_DOUBLE_ARGV(i);
int aInteger = FLL_INT_ARGV(i);
int aBool = FLL_BOOL_ARGV(i);
```

A C-Functions is a native implementation
```c

void impFnPrint(langSTATE st, ICTAB table){
	FLL_CHECK_STATE()
	
	//int c = FLL_ARGC(); //
	
	/*
	VALUE vMsg = FLL_ARGV(1);
	if(!FLL_IS_SET(vMsg)){
		langRaiseRuntimeError(st, "Missing message parameter");
		return;
	}
	
	char *s = valueToString(st, vMsg);
	*/
	
	char *s = FLL_STRING_ARGV(0);
	if(!s) return;
	printf("object.print(%s)\n", s);
}
```

