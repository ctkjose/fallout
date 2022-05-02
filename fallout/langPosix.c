//
//  langPosix.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#include "langPosix.h"

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

void langLogMessage(const char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
}
void langStdErrMessage(char* fmt, ...){
	const size_t sz = 1500;
	char msg[sz];
	
	va_list args;
	va_start( args, fmt );
	vsnprintf( msg, sz, fmt, args );
	va_end( args );
	
	
	fputs(msg, stderr);
	fputc('\n', stderr);
}
void langStdOutMessage(char* fmt, ...){
	const size_t sz = 1500;
	char msg[sz];
	
	va_list args;
	va_start( args, fmt );
	vsnprintf( msg, sz, fmt, args );
	va_end( args );
	
	
	fputs(msg, stdout);
	fputc('\n', stdout);
}

