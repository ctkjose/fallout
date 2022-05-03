//
//  langParser.h
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//

#ifndef langParser_h
#define langParser_h

#include "langCommon.h"


#define alloc_token()  (struct langNode *) malloc(sizeof(struct langNode))



void parserRaiseError(struct langParseState * parser, const char *err);
void parserRaiseSyntaxError(struct langParseState *parser, char *msg);


//scanner
int parserIsAllowedInIdentifier(int c, int flgStartChar);
int parserUnEscape(struct langParseState * parser);
int parserCharConvertHex(int c);
int parserCharIsHex(int c);
int parserCharIsWhite(int c);
int parserCharIsNewLine(int c);
void parserGetChar(struct langParseState * parser);
int parserExpect(struct langParseState * parser, int c);
int parserAccept(struct langParseState * parser, int c);
void parserRaiseError(struct langParseState * parser, const char *err);
void parserRaiseSyntaxError(struct langParseState *parser, char *msg);


//parsing/lexer
void tokenFree(langNODE node);
langNODE tokenCreate(void);

langNODE tokenNext(struct langParseState * parser);
int tokenIsKeyword(struct langNode *node);
void tokenRestore(struct langParseState *parser, langNODE node);

int tokenExpectOperator(struct langParseState *parser, int op);
int tokenAcceptOperator(struct langParseState *parser, int op);
int tokenAcceptKeyword(struct langParseState *parser, int keyword);

void debugDumpToken(langNODE node, char* prefix);


void langParserReadOperator(struct langParseState * parser, struct langNode * node);
void langParserReadNumber(struct langParseState * parser, struct langNode * node);
void langParserReadString(struct langParseState * parser, struct langNode * node);
void langParserReadIdentifier(struct langParseState * parser, struct langNode * node);


ICODE icodeAllocate(langPARSER parser, int type);
ICODE icodeFromIdentifier(langPARSER parser, langNODE node);


int parseExpressionSimple(langPARSER parser, ICTAB table);
int parseExpressionFactor(langPARSER parser, ICTAB table);
int parseExpressionTerm(langPARSER parser, ICTAB table);


int parseIdentifier(langPARSER parser,  ICTAB table, char *name, ICODE parent);
int parseAssigment(langPARSER parser, ICTAB table, char *name, ICODE parent);
void parseKeyword(langPARSER parser, ICTAB table, int keyword);
int parseIfStatement(langPARSER parser, ICTAB table);
int parseWhileStatement(langPARSER parser, ICTAB table);
void parseFormalParameter(langPARSER parser, OBJECT fnObj);
VALUE parseFuncDeclaration(langPARSER parser);
int parseFuncStatement(langPARSER parser, ICTAB table);
int parseDeclarationVariable(langPARSER parser, ICTAB table);
void parserFunctionBody(langPARSER parser, ICTAB table, int del);



ICODE parseICodeExpression(langPARSER parser);
ICODE parserICodeBlock(langPARSER parser);
ICODE parseICodeFNCall(langPARSER parser, ICODE icFNREF, int opConsumeLP);
ICODE parseICodeAssignment(langPARSER parser, ICODE leftSide);
ICODE parseICodeLeftHand(langPARSER parser);

//FUNCTION parseSourceCode(langSTATE st, const char *source);
ICTAB parserSource(langPARSER parser, const char *source);

#endif /* langParser_h */
