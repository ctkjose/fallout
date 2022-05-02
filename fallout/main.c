//
//  main.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#include "fallout.h"
#include "regexp.h"

void testRegEx(void);
void testPrimitives(void);
void testUTF8(void);

void onRuntimeError(langSTATE st, char *msg){
	printf("@onRuntimeError\n");
	fprintf(stderr, "%s", msg);
	exit(EXIT_FAILURE);
}
void onParserError(langSTATE st, char *msg){
	printf("@onParserError\n");
	fprintf(stderr, "%s", msg);
	exit(EXIT_FAILURE);
}

void runScriptWithSource(langSTATE st, char *source){
	
	if(!source) return;
	
	runtimeExecuteSource(st, source);
	
	
	int ok = 0;
	VALUE v;
	///ok = executeAssigment(cs,  cs->statements->items[0]);

	printf("SCRIPT DONE.....\n");
	return;
}
void runScriptWithPath(langSTATE st, char *filename){
	long sz = 0;
	
	FILE * fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "[FALLOUT][ERROR] cannot open file '%s': %s\n", filename, strerror(errno));
		return;
	}
	
	if (fseek(fp, 0, SEEK_END) < 0) {
		fclose(fp);
		fprintf(stderr, "[FALLOUT][ERROR] cannot read file '%s' [ERR-BG001]: %s\n", filename, strerror(errno));
		return;
	}
	
	sz = ftell(fp);
	if (sz < 0) {
		fclose(fp);
		fprintf(stderr, "[FALLOUT][ERROR] cannot read file '%s' [ERR-BG002]: %s\n", filename, strerror(errno));
		return;
	}
	
	if (fseek(fp, 0, SEEK_END) < 0) {
		fclose(fp);
		fprintf(stderr, "[FALLOUT][ERROR] cannot read file '%s' [ERR-BG003]: %s\n", filename, strerror(errno));
		return;
	}
	if (fseek(fp, 0, SEEK_SET) < 0) {
		fclose(fp);
		fprintf(stderr, "[FALLOUT][ERROR] cannot read file '%s' [ERR-BG004]: %s\n", filename, strerror(errno));
		return;
	}
	
	int szln = 255;
	char firstLine[255];
	
	if(fgets(firstLine, szln, fp) == NULL){
		fclose(fp);
		fprintf(stderr, "[FALLOUT][ERROR] cannot read file '%s' [ERR-BG005]: %s\n", filename, strerror(errno));
		return;
	}
	
	char* shebang = strstr(firstLine, "#!");
	//printf("size=%i\n",sz);
	if(!shebang) {
		fseek(fp, 0, SEEK_SET); //rewind
	}else{
		sz = sz - strlen(firstLine); //reduce buffer;
	}
	
	char *src = malloc(sizeof(char) * (sz + 1)); /* add space for string terminator */
	
	long szread = fread(src, 1, (size_t)sz, fp);
	if (szread != sz) {
		fclose(fp);
		fprintf(stderr, "[FALLOUT][ERROR] cannot read file '%s' [ERR-BG006] [%lu][%lu]\n", filename,sz,szread );
		return;
	}
	
	fclose(fp);
	
	src[sz] = 0; /* zero-terminate string containing file data */

	runScriptWithSource(st, src);
}
int main(int argc, const char * argv[]) {
	// insert code here...
	char script_file[PATH_MAX];
	char *res;
	
    setlocale(LC_ALL, "en_US.utf8");
    
    testUTF8();
    exit(1);
    
	char *env = getenv("PATH_TRANSLATED1");
	if (!env) {
		env = getenv("SCRIPT_FILENAME");
	};
	
	if (env) {
		res = realpath(env, script_file);
	}else if(argc > 1){
		res = realpath(argv[1], script_file);
	}else{
		langLogMessage("[FALLOUT] No script file specified1\n");
		exit(0);
	}
	
	if (!res) {
		langLogMessage("[FALLOUT] No script file specified2\n");
		exit(0);
	}
	
	//fprintf(stderr, "[FALLOUT] LOAD [%s]\n", script_file);
	langStdOutMessage("[FALLOUT] LOAD [%s]\n", script_file);
	
	
	
	
	//testPrimitives();
	langSTATE st = langStateCreate();
	
	//setup some callbacks
	st->onRuntimeError = onRuntimeError;
	st->onSyntaxError = onParserError;
	
	
	runScriptWithPath(st, script_file);
	
	//testRegEx();
}
void testUTF8(){
    //ustr s = utf8_CreateString(255);
    char *cstr = "😀jöse";
    
    unsigned long sz = strlen(cstr);
    ustr str;
    str = malloc(sizeof(struct uString));
    if(!str) return;
    
    str->text = (char *) malloc(sizeof(char) * sz + 1);
    str->size = (int)sz;
    str->length = 0;
    
    str->size = strSafeCopy(str->text, sz + 1, cstr);
    str->length = utflen(str->text);
    
    printf("dst[%s] sz=%u length=%u\n", str->text, str->size, str->length);
    
    int bytes = 0;
    char *ch1 = (char *) malloc(sizeof(char) * 5);
    for(int i = 0; i < str->length; i++){
        Rune ch = runeAtIndex(str->text, i);
        memset(ch1,0, 5);
        bytes = runetochar(ch1, &ch);
        printf("RUNE[%u]=[%#x]=[U+%-4.4x](%u/b)=[%s]\n", i, ch, ch,bytes, ch1);
        for(int i = 0; ch1[i] && i < 5; ++i) {
            printf("%hhx ", ch1[i]);
        }
        puts("\n");
    }
    
    
    
    
}
void testPrimitives(){
	
	langSTATE st = langStateCreate();
	FUNCTION fn = functionCreate(st, "doThis");
	
	debugDumpFunction(fn,0);
}
void testRegEx(){
	
	const char *error;
	const char *s = "JOSEcuevas";
	Reprog *p;
	Resub m;
	int i;

	
	p = regcomp("^[A-Z]+[a-z]{2}", 0, &error);
	if (!p) {
		fprintf(stderr, "regcomp: %s\n", error);
		return;
	}

	printf("nsub = %d\n", p->nsub);
	if (!regexec(p, s, &m, 0)) {
		for (i = 0; i < m.nsub; ++i) {
			long n = m.sub[i].ep - m.sub[i].sp;
			if (n > 0){
				printf("match %d: s=%d e=%d n=%ld '%.*s'\n", i, (int)(m.sub[i].sp - s), (int)(m.sub[i].ep - s), n, n, m.sub[i].sp);
			}else{
				printf("match %d: n=0 ''\n", i);
			}
		}
	} else {
		printf("no match\n");
	}

}

