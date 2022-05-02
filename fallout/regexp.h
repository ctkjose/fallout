//
//  regexp.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#ifndef regexp_h
#define regexp_h

#include "utf.h"

#define regcompx js_regcompx
#define regfreex js_regfreex
#define regcomp js_regcomp
#define regexec js_regexec
#define regfree js_regfree

typedef struct Reprog Reprog;
typedef struct Resub Resub;
typedef struct Reclass Reclass;
typedef struct Renode Renode;
typedef struct Reinst Reinst;
typedef struct Rethread Rethread;

Reprog *regcompx(void *(*alloc)(void *ctx, void *p, int n), void *ctx,
	const char *pattern, int cflags, const char **errorp);
void regfreex(void *(*alloc)(void *ctx, void *p, int n), void *ctx,
	Reprog *prog);

Reprog *regcomp(const char *pattern, int cflags, const char **errorp);
int regexec(Reprog *prog, const char *string, Resub *sub, int eflags);
void regfree(Reprog *prog);

enum {
	/* regcomp flags */
	REG_ICASE = 1,
	REG_NEWLINE = 2,

	/* regexec flags */
	REG_NOTBOL = 4,

	/* limits */
	REG_MAXSUB = 10
};

#define emit regemit
#define next regnext
#define accept regaccept

#define nelem(a) (int)(sizeof (a) / sizeof (a)[0])

#define REPINF 255
#define MAXSUB REG_MAXSUB
#define MAXPROG (32 << 10)

struct Resub {
	int nsub;
	struct {
		const char *sp;
		const char *ep;
	} sub[REG_MAXSUB];
};

struct Reclass {
	Rune *end;
	Rune spans[64];
};

struct Reprog {
	Reinst *start, *end;
	int flags;
	int nsub;
	Reclass cclass[16];
};



#endif /* regexp_h */
