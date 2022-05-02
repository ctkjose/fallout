//
//  langState.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#ifndef langState_h
#define langState_h

#include "langTypes.h"


struct langState * langStateCreate(void);

void langAbort(langSTATE st);
void langRaiseRuntimeError(langSTATE st, char *msg);
void langRaiseSyntaxError(langSTATE st, char *msg);

void langRaiseError(langSTATE st, ERROR err);
ERROR langCreateError(int errCode, char *msg, int isFatal, int srcLine);

#endif /* langState_h */
