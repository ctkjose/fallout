//
//  langFramework.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/23/21.
//

#ifndef langFramework_h
#define langFramework_h

#include "langCommon.h"
#include <stdio.h>

#pragma mark FRAMEWORK

#define FLL_CHECK_STATE()  if(!st || !table) return;

#define FLL_IS_UNDEFINED(v)  (v->type == kValueUndefined)
#define FLL_IS_SET(v)  (v && v->type != kValueUndefined && v->type != kValueNULL)

#define	FLL_ARGC() stackGetParameterCount(st, table)
#define	FLL_ARGV(i) stackGetParameter(st, table, i)

#define FLL_STRING_ARGV(i)  valueToString(st, stackGetParameter(st, table, i))
#define FLL_DOUBLE_ARGV(i)  valueToNumber(st, stackGetParameter(st, table, i))
#define FLL_INT_ARGV(i)  (int)valueToNumber(st, stackGetParameter(st, table, i))
#define FLL_BOOL_ARGV(i)  (int)valueToBool(st, stackGetParameter(st, table, i))


void falloutInitFramework(langSTATE st);

#endif /* langFramework_h */
