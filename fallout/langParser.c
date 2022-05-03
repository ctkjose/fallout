//
//  langParser.c
//  fallout
//
//  Created by JOSE L CUEVAS on 8/21/21.
//


#include "langParser.h"
#include "langICode.h"
//#include "langRuntime.h"


const char *langKeywords[] = {
	"var", "let", "if","else","while","for", "function"
};


void parserRaiseSyntaxError(struct langParseState *parser, char *msg){
	
	if(!parser->st) return;
	parser->abort = 1;
	parser->st->abort = 1;
	
	
	//langRaiseSyntaxError(st, msg);
	
	ERROR err = langCreateError(kERR_SYNTAX, msg, 1, parser->srcLine);
	langRaiseError(parser->st, err);
}
void parserRaiseError(struct langParseState * parser, const char *msg){
	parser->abort = 1;
	parser->st->abort = 1;
	
    //FIX change error type
	ERROR err = langCreateError(kERR_SYNTAX, (char *) msg, 1, parser->srcLine);
	langRaiseError(parser->st, err);
}

int parserAccept(struct langParseState * parser, int c){
	if(parser->lastCH == c){
		parserGetChar(parser);
		return 1;
	}
	return 0;
}
int parserExpect(struct langParseState * parser, int c){
	if(parser->lastCH == c){
		parserGetChar(parser);
		return 1;
	}else{
		char err[255];
		sprintf(err, "Expected character %c", c);
		parserRaiseSyntaxError(parser, err);
		return 0;
	}
	return 0;
}

void parserGetChar(struct langParseState * parser){
	Character c;
	int l;
	
	l = utf8CharacterFromCString((CString) parser->src, &c);
	parser->lastCHLen = l;
	
	parser->srcIDX += l;
	//printf("@parserGetChar([IDX=%d] [ch=%X=%c] [sz=%d])\n", parser->srcIDX, c,c, l);
	
	parser->src += l;
	if (c == '\r' && *parser->src == '\n'){
		++parser->src;
		parser->srcIDX += 1;
	}
	
	if(parserCharIsNewLine(c)){
		parser->srcLine++;
		c = '\n';
	}
	
	parser->lastCH = c;
	return;
}
int parserCharIsNewLine(int c){
	return (c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029);
}
int parserCharIsWhite(int c){
	
	return c == 0x9 || c == 0xB || c == 0xC || c == 0x20 || c == 0xA0 || c == 0xFEFF;
}
int parserCharIsHex(int c){
	return isdigit(c) || ishex(c);
}
int parserCharConvertHex(int c){
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}

int parserUnEscape(struct langParseState * parser){
	if (parserAccept(parser, '\\')){
		int val = 0;
		/* handle UTF escape sequences \uXXXX */
		if (parserAccept(parser, 'u')){
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF); parserGetChar(parser);
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF); parserGetChar(parser);
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF); parserGetChar(parser);
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF);
		
			return val;
		}else if (parserAccept(parser, 'x')){
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF); parserGetChar(parser);
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF);
			
			return val;
		}else{
			switch (parser->lastCH) {
				case 'n':
					return '\n';
				case 'r':
					return '\r';
				case 't':
					return '\t';
				case '\\':
					return '\\';
				case '\'':
					return '\'';
				case '0':
					return '0';
				case '"':
					return '"';
				case 'b':
					return '\b';
				case 'f':
					return '\f';
				case 'v':
					return '\v';
				default:
					return '\\';
			}
		}
		
	
	}
	
	return parser->lastCH;
	
error:
	langLogMessage("Syntax Error: invalid escape sequence");
	return 0;
}

int parserIsAllowedInIdentifier(int c, int flgStartChar){
	return (flgStartChar) ? (isalpha(c) || c == '$' || c == '_' || isalpharune(c)) : (isalpha(c) || isdigit(c) || c == '$' || c == '_' || isalpharune(c));
}


langNODE tokenCreate(){
	langNODE node = malloc(sizeof(struct langNode));
	node->type = 0;
	node->srcLine = 0;
	node->operator = 0;
	node->vInteger = 0;
    node->vFloat = 0.0;
	node->keyword = 0;
	node->text = NULL;
	
	return node;
}
void tokenFree(langNODE node){
	if(!node) return;
	if(node->text) free((void *)node->text);
	
	free(node);
}

int tokenIsKeyword(struct langNode *node){
	
	int c = (int)(sizeof (langKeywords) / sizeof (langKeywords)[0]);
	for(int i=0;i<c;i++){
		if(strcmp(node->text, langKeywords[i]) == 0){
			node->type = kTK_KEYWORD;
			node->keyword = kKEWORDS_START+i;
			return 1;
		}
	}
	
	
	return 0;
}
int tokenAcceptKeyword(struct langParseState *parser, int keyword){
	
	struct langNode * node;
	node = tokenNext(parser);
	if(!node){
		return 0;
	}
	if( node->type != kTK_IDENTIFIER ){
		tokenRestore(parser,node);
		return 0;
	}
	if(! tokenIsKeyword(node) ){
		tokenRestore(parser,node);
		return 0;
	}
	
	if(node->keyword != keyword){
		tokenRestore(parser,node);
		return 0;
	}
	
	printf("is keyword: %d=%d=%s\n", node->type, node->keyword, node->text);
	tokenFree(node);
	return 1;
}
int tokenAcceptOperator(struct langParseState *parser, int op){
	
	struct langNode * node;
	node = tokenNext(parser);
	if(!node){
		return 0;
	}
	if( (node->type == kTK_OPERATOR) && (node->operator == op)){
		return 1;
	}
	
	tokenRestore(parser,node);
	return 0;
}
int tokenExpectOperator(struct langParseState *parser, int op){
	
	struct langNode * node;
	node = tokenNext(parser);
	if(!node){
		return 0;
	}
	if( (node->type == kTK_OPERATOR) && (node->operator == op)){
		return 1;
	}
	
	//tokenRestore(parser,node);
	return 0;
}
void tokenRestore(struct langParseState *parser, langNODE node){
	/* Puts back a token, to use later*/
	parser->holdNode = node;
	parser->hasHoldNode = 1;
}

langNODE tokenNext(struct langParseState * parser){
	if(!parser) return NULL;
	
	//printf("@parserGetChar(Node [%c]------\n", parser->lastCH);
	if(parser->hasHoldNode){
		parser->hasHoldNode = 0;
		return parser->holdNode;
	}
	
	
	langNODE node = tokenCreate();
	if(!node) return NULL;
	
	node->operator = 0;
	node->text = NULL;
	node->keyword = 0;
	node->srcLine = parser->srcLine;
	int sz = 0;
	
	
	while(1){
	//printf("parserGetChar(Node[%d/%d]=[%c][%X]\n", parser->srcIDX, parser->srcLen, parser->lastCH, parser->lastCH);
		while(parserCharIsWhite(parser->lastCH)){
			parserGetChar(parser);
		}
		
		//printf("parserGetChar(Node[%d/%d]=[%c][%X]\n", parser->srcIDX, parser->srcLen, parser->lastCH, parser->lastCH);
		
		if(parser->lastCH == '\n'){
			parser->srcLine++;
			parserGetChar(parser);
			continue;
		}
		
		if(parser->lastCH == 0){
			tokenFree(node);
			return NULL;
		}
		if(isdigit(parser->lastCH)){
			langParserReadNumber(parser, node);
			parser->lastNode = node;
			return node;
		}
		
		if(parser->lastCH == '"'){
			langParserReadString(parser, node);
			return node;
		}
		
		if(parserAccept(parser,'/')){
			if(parserAccept(parser, '/')){ //is a comment
				
			}
		}
		
		if(is_special(parser->lastCH)){
			langParserReadOperator(parser,node);
			return node;
		}
		
		parserUnEscape(parser);
		
		if(parserIsAllowedInIdentifier(parser->lastCH,1)){
			langParserReadIdentifier(parser, node);
			
			return node;
		}
		
		return NULL;
	}
	
	parserGetChar(parser);
	return NULL;
}


void langParserReadOperator(struct langParseState * parser, struct langNode * node){
	int opType = 0;
	char name[6] = "";
	switch (parser->lastCH) {
		case '=':
			opType = opType_Assign;
			strcpy(name, "=");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_LogicalEqual;
				strcpy(name, "==");
			}
			break;
		case ':':
			opType = opType_Colon; parserGetChar(parser); strcpy(name, ":"); break;
		case ';':
			opType = opType_ST; parserGetChar(parser); strcpy(name, ";"); break;
		case ',':
			opType = opType_Comma; parserGetChar(parser); strcpy(name, ","); break;
		case '.':
			opType = opType_Dot; parserGetChar(parser); strcpy(name, "."); break;
		case '^':
			opType = opType_Pow; parserGetChar(parser); strcpy(name, "^"); break;
		case '(':
			opType = opType_LP; parserGetChar(parser); strcpy(name, "("); break;
		case ')':
			opType = opType_RP; parserGetChar(parser); strcpy(name, ")"); break;
		case '{':
			opType = opType_LKey; parserGetChar(parser); strcpy(name, "{"); break;
		case '}':
			opType = opType_RKey; parserGetChar(parser); strcpy(name, "}"); break;
		case '[':
			opType = opType_LBracket; parserGetChar(parser); strcpy(name, "["); break;
		case ']':
			opType = opType_RBracket; parserGetChar(parser); strcpy(name, "]"); break;
		case '%':
			opType = opType_Modulo; parserGetChar(parser); strcpy(name, "%"); break;
		case '<':
			opType = opType_Less;
			strcpy(name, "<");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_LessEqual;
				strcpy(name, "<=");
			}else if(parserAccept(parser,'>')){
				opType = opType_NotEqual;
				strcpy(name, "!=");
			}
			break;
		case '>':
			opType = opType_Greater;
			strcpy(name, ">");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_GreaterEqual;
				strcpy(name, ">=");
			}
			break;
		
		case '!':
			opType = opType_Neg;
			strcpy(name, "!");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_NotEqual;
				strcpy(name, "!=");
			}
			break;
		case '+':
			opType = opType_Plus;
			strcpy(name, "+");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_Add_Assign;
				strcpy(name, "+=");
			}
			break;
		case '-':
			opType = opType_Minus;
			strcpy(name, "-");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_Add_Assign;
				strcpy(name, "-=");
			}
			break;
		case '*':
			opType = opType_Multiply;
			strcpy(name, "*");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_Multiply_Assign;
				strcpy(name, "*=");
			}
			break;
		case '/':
			opType = opType_Divide;
			strcpy(name, "/");
			parserGetChar(parser);
			if(parserAccept(parser,'=')){
				opType = opType_Multiply_Assign;
				strcpy(name, "/=");
			}
			break;
		
		default:
			opType = (int) parser->lastCH;
			parserGetChar(parser);
			break;
	}

	if(opType > 0){
		node->type = kTK_OPERATOR;
		node->operator = opType;
		node->text = strdup(name);
	}
}


void langParserReadNumber(struct langParseState * parser, struct langNode * node){
	printf("@langParserReadNumber....\n");
	
	char s[30] = "";
	int i = 0;
	
	node->type = kTK_INTEGER;
	
	
	
	if (parserAccept(parser, '0')) {
		if (parserAccept(parser, 'x') || parserAccept(parser, 'X')) {
			int val = 0;
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF); parserGetChar(parser);
			
			if (!parserCharIsHex(parser->lastCH)) goto error;
			val = (val << 4) | (parserCharConvertHex(parser->lastCH) & 0xF);
			
			
			node->vInteger = val;
			return;
		}
		
		s[i++] = '\0';
	}
	//get the whole part
	while(isdigit(parser->lastCH)){
		if(i==30) break;
		s[i++] = parser->lastCH;
		parserGetChar(parser);
	}
    s[i] = '\0';
	
	if(parser->lastCH == '.'){
        node->type = kTK_FLOAT;
		s[i++] = '.';
		parserGetChar(parser);
		while(isdigit(parser->lastCH) || (parser->lastCH == 'e') || (parser->lastCH == 'E') || (parser->lastCH == '+') || (parser->lastCH == '-') ){
			if(i==30) break;
			s[i++] = parser->lastCH;
			parserGetChar(parser);
		}
        s[i] = '\0';
	}
	
	
	printf("number str=[%s]\n", s);
    if(node->type == kTK_FLOAT){
        node->vFloat = atof(s);
    }else{
        node->vInteger = atol(s);
    }
	return;
	
error:
	langLogMessage("Syntax Error: invalid number format");
	return;
}

void langParserReadString(struct langParseState * parser, struct langNode * node){
	printf("@langParserReadString....\n");
	UString s = utf8Create(255);
	
	int del = parser->lastCH;
	parserGetChar(parser);
	int ch  = parserUnEscape(parser);
	
    while (ch != del) {
		if (ch == 0){
			parserRaiseSyntaxError(parser, "Syntax Error, string not terminated");
			return;
		}
		
        utf8AppendCharacter(s, ch);
		
		parserGetChar(parser);
		ch = parserUnEscape(parser);
	}
	
	
	
	if( parserExpect(parser, del) ){
		node->type = kTK_STRING;
		node->text = malloc(s->length * sizeof(char));
		strcpy((char *)node->text, s->text);
		
		free(s);
	}

}

void langParserReadIdentifier(struct langParseState * parser, struct langNode * node){
	printf("@langParserReadIdentifier....\n");
	UString s = utf8Create(255);
    utf8AppendCharacter(s, parser->lastCH);
	
	parserGetChar(parser);
	int ch = parserUnEscape(parser);
	while(parserIsAllowedInIdentifier(ch,0)){
		
        utf8AppendCharacter(s, ch);
			   
		printf("word=[%d][%X]=[%s]\n", ch,ch, s->text );
		parserGetChar(parser);
		ch = parserUnEscape(parser);

	}
	
	printf("word_final=[%d][%X]=[%s]\n", parser->lastCH,parser->lastCH, s->text );
	
	node->type = kTK_IDENTIFIER;
	node->text = malloc(s->length * sizeof(char));
	strcpy((char *) node->text, s->text);
	
	free(s);
}

ICODE icodeFromIdentifier(langPARSER parser, langNODE node){
	ICODE ic = icodeAllocate(parser, kST_LITERAL);
	ic->ref = valueCreate(parser->st, kValueString);
	ic->ref->value.string = strdup((char *) node->text);
	
	return ic;
}
ICODE icodeAllocate(langPARSER parser, int type){
	ICODE ic = icodeCreate(type);
	ic->srcLine = parser->srcLine;
	return ic;
}


ICTAB parserSource(langPARSER parser, const char *source){
	
	parser->src = source;
	
	parser->lastNode = NULL;
	parser->srcIDX = -1;
	parser->srcLine = 1;
	parser->srcLen = (int) strlen(source);
	parser->lastCH = 0;
	
	parser->flgInBlock = 0;
	parser->flgInFunction = 0;
	
	parser->hasHoldNode = 0;
	parser->lastNode = NULL;
	
	ICTAB table = icodeTableCreate();
	
	parserGetChar(parser);
	parserFunctionBody(parser, table,0);
	
	return table;
}
void parserFunctionBody(langPARSER parser, ICTAB table, int del){
	
	if(!parser) return;
	if(!table) return;
	
	parser->flgInBlock = 1;
	parser->flgInFunction = 0;
	
	struct langNode * node;
	int stateHasLeftHand = 0;
	ICODE icLeftHand;
	ICODE stmt;
	while(1){
		if(parser->st->abort) break;
		
		node = tokenNext(parser);
		if(!node){
			break;
		}
		if( stateHasLeftHand && icLeftHand ){
			if(node->type == kTK_OPERATOR){
				if(node->operator == opType_ST){
					icodeTablePush(table, icLeftHand);
					continue;
				}else if(node->operator == opType_Assign){
					stateHasLeftHand = 0;
					stmt = parseICodeAssignment(parser, icLeftHand);
					if(stmt){
						icodeTablePush(table, stmt);
					}
					continue;
				}else if(node->operator == opType_LP){
					stmt = parseICodeFNCall(parser, icLeftHand, 0);
					if(stmt){
						icodeTablePush(table, stmt);
					}
					
					tokenAcceptOperator(parser, opType_ST);
					continue;
				}
			}
		}
		if( node->type == kTK_IDENTIFIER ){
		
			if( tokenIsKeyword(node) ){
				//printf("is keyword: %d=%d=%s\n", node->type, node->keyword, node->text);
				parseKeyword(parser, table, node->keyword);
				tokenFree(node);
				continue;
			}else{
				tokenRestore(parser,node);
				icLeftHand = parseICodeLeftHand(parser);
				if(icLeftHand){
					stateHasLeftHand = 1;
				}
			}
			
			continue;
		}
	}//end while
}
void parseKeyword(langPARSER parser, ICTAB table, int keyword){
	
	if(!parser || !table || !keyword) return;
	
	//e->name = strdup("jose");
	
	if(keyword == kST_VAR){
		if( parseDeclarationVariable(parser, table) ){
			//push
		}
	}else if(keyword == kST_IF){
		parseIfStatement(parser, table);
    }else if(keyword == kST_WHILE){
        parseWhileStatement(parser, table);
    }else if(keyword == kST_FUNCTION){
        parseFuncStatement(parser, table);
	}
}
int parseExpression(langPARSER parser, ICTAB table){
	printf("@ParseExpression\n");
	if(!parseExpressionSimple(parser, table)) return 0;
	
	struct langNode *node;
	node = tokenNext(parser);
	if(!node){
		return 0;
	}
	
	while( is_token_op(node) && is_op_logical(node->operator) ){
		ICODE e;
		
		e = icodeAllocate(parser, kST_OP);
		e->code = node->operator;
		e->name = strdup((char *) node->text);
		icodeTablePush(table, e);
		free(node);
		
		if(!parseExpressionTerm(parser, table)) return 0;
		
		node = tokenNext(parser);
		if(!node) break;
	}
	
	if(node){
		tokenRestore(parser,node);
	}
	
	return 1;
}
int parseExpressionSimple(langPARSER parser, ICTAB table){
	
	//TODO handle -TERM or +TERM
	
	ICTAB TARGET = table;
	langNODE node;
	
	//TODO unary NEG operator
	int has_unary_op = 0;
	int unary_factor = 1;
	ICODE unaryExpr;
	
	
	
	node = tokenNext(parser);
	if(!node){
		return 0;
	}
	

	if( is_token_op(node) && ( (node->operator == opType_Plus) || (node->operator == opType_Minus) ) ){
		
		if(node->operator == opType_Minus){
			has_unary_op = 1;
			
			unary_factor = -1;
			unaryExpr = icodeAllocate(parser, kST_OP);
			ICODE uFx = icodeAllocate(parser, kST_LITERAL);
			uFx->srcLine = parser->srcLine;
			uFx->ref = valueCreate(parser->st,kValueInteger);
			uFx->ref->value.asInteger = -1;
			icodeTablePush(unaryExpr->args, uFx);
			
			
		}
		
		tokenFree(node);
		
	}else{
		tokenRestore(parser,node);
	}
	
	
	if(!parseExpressionTerm(parser, table)) return 0;
	
	node = tokenNext(parser);
	
	
	while( node && is_token_op(node) && is_op_additive(node->operator) ){
		ICODE e;
		
		if(has_unary_op){
			TARGET = unaryExpr->args;
		}
		e = icodeAllocate(parser, kST_OP);
		e->code = node->operator;
		e->name = strdup((char *)node->text);
		icodeTablePush(TARGET, e);
		free(node);
		
		if(!parseExpressionTerm(parser, TARGET)) return 0;
		if(has_unary_op){
			has_unary_op = 0;
			icodeTablePush(table, unaryExpr);
			TARGET = table;
		}
		
		node = tokenNext(parser);
		if(!node) break;
	}
	
	if(node){
		tokenRestore(parser,node);
	}
	
	return 1;
}
int parseExpressionTerm(langPARSER parser, ICTAB table){
	
	parseExpressionFactor(parser, table);
	
	langNODE node;
	node = tokenNext(parser);
	if( !node ){
		return 0;
	}
	
	
	while( node && is_token_op(node) &&  is_op_multiplicative(node->operator) ){
		ICODE e;
		
		debugDumpToken(node, "TERM:");
		
		e = icodeAllocate(parser, kST_OP);
		e->code = node->operator;
		icodeTablePush(table, e);
		tokenFree(node);
		
		if( !parseExpressionFactor(parser, table) ) return 0;
		
		node = tokenNext(parser);
		if( !node ) break;
	}
	
	if( node ){
		tokenRestore(parser,node);
	}
	
	return 1;
}
int parseExpressionFactor(langPARSER parser, ICTAB table){
	
	struct langNode * node;
	
	node = tokenNext(parser);
	if(!node) return 0;
	
	debugDumpToken(node, "FACTOR:");
	
	//variable
	if(node->type == kTK_INTEGER){
		ICODE e = icodeAllocate(parser, kST_LITERAL);
		e->srcLine = parser->srcLine;
		e->ref = valueCreate(parser->st,kValueInteger);
		e->ref->value.asInteger = node->vInteger;
		icodeTablePush(table, e);
		return 1;
    }else if(node->type == kTK_FLOAT){
        ICODE e = icodeAllocate(parser, kST_LITERAL);
        e->srcLine = parser->srcLine;
        e->ref = valueCreate(parser->st, kValueFloat);
        e->ref->value.asFloat = node->vFloat;
        icodeTablePush(table, e);
        return 1;
	}else if(node->type == kTK_STRING){
		ICODE e = icodeAllocate(parser, kST_LITERAL);
		e->srcLine = parser->srcLine;
		e->ref = valueCreate(parser->st,kValueString);
		e->ref->value.string = strdup((char *) node->text);
		icodeTablePush(table, e);
		return 1;
	}else if(node->type == kTK_IDENTIFIER){
		tokenRestore(parser, node);
		ICODE e = parseICodeLeftHand(parser);
		if(e){
			icodeTablePush(table, e);
			return 1;
		}else{
			return 0;
		}
	}else if(node->type == kTK_OPERATOR){
		if(node->operator == opType_Neg){
			ICODE e = icodeAllocate(parser, kST_OP);
			e->srcLine = parser->srcLine;
			e->code = node->operator;
			e->name = strdup((char *)node->text);
			icodeTablePush(table, e);
			return parseExpressionFactor(parser, table);
		}else if(node->operator == opType_LP){
			ICODE e = parseICodeExpression(parser);
			e->srcLine = parser->srcLine;
			icodeTablePush(table, e);
			
			tokenFree(node);
			
			if(!tokenAcceptOperator(parser, opType_RP)){
				parserRaiseSyntaxError(parser, "Expecting operator \")\" in expression.");
				return 0;
			}
			
			
			return 1;
		}else{
			tokenRestore(parser,node);
			return 0;
		}
	}else{
		tokenRestore(parser,node);
		return 0;
	}
	
	return 0;
}

int parseDeclarationVariable(langPARSER parser, ICTAB table){
	
	struct langNode * node;
	node = tokenNext(parser);
	
	debugDumpToken(node, "VARTOKEN");
	
	if(!node || (node->type != kTK_IDENTIFIER) ){
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "expecting identifier in variable declaration");
		return 0;
	}
	
	if( tokenIsKeyword(node) ){
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "reserved keyword in variable declaration");
		return 0;
	}
	
	printf("parseVarDeclaration [%s]\n", node->text);
	
	ICODE e = icodeAllocate(parser, kST_VARINIT);
	e->args = icodeTableCreate();
	
	//What var must be created
	ICODE varRef = icodeAllocate(parser, kST_LITERAL);
	varRef->ref = valueCreate(parser->st,kValueString);
	varRef->ref->value.string = strdup((char *) node->text);
	icodePush(e, varRef);
	

	tokenFree(node);
	
	icodeTablePush(table, e);

	node = tokenNext(parser);
	
	if(!node || node->type != kTK_OPERATOR){
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "expecting statement terminator \";\" in variable declaration");
		return 0;
	}
		
	/* = | , | ; */
	if(node->operator == opType_Assign){
		tokenFree(node);

		ICODE icEXPR = parseICodeExpression(parser);
		if(!icEXPR){
			parserRaiseSyntaxError(parser,"expecting expression in assigment.");
		}
		icodePush(e, icEXPR);
		
		node = tokenNext(parser);
		if(!node || (node->type != kTK_OPERATOR)){
			parserRaiseSyntaxError(parser, "expecting statement terminator \";\" in variable declaration");
			return 0;
		}
	}else{
        //FIX must be an expression
		//Create assigmnet to UNDEFINED
		ICODE defValue = icodeAllocate(parser, kST_LITERAL);
		defValue->ref = valueCreate(parser->st,kValueUndefined);
		defValue->ref->value.asInteger = 0;
		icodePush(e, defValue);
	}
	
	
	if(node->operator == opType_Comma){
		tokenFree(node);
		parseDeclarationVariable(parser, table);
	}else if(node->operator == opType_ST){
		tokenFree(node);
		printf("var terminated\n");
	}else{
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "expecting statement terminator \";\" in variable declaration");
		return 0;
	}
	
	
	return 1;
}

int parseStatement(langPARSER parser, ICTAB table){
	
	if(!parser) return 0;
	if(!table) return 0;
	if(parser->abort) return 0;
	
	parser->flgInBlock = 1;
	parser->flgInFunction = 0;
	
	int stmtDone = 0;
	struct langNode * node  = NULL;
	int stateHasLeftHand = 0;
	ICODE icLeftHand = NULL;
	ICODE stmt = NULL;
	
	
	
	do{
		if(parser->abort){
			stmtDone = 1;
			break;
		}
		
		node = tokenNext(parser);
		if(!node){
			stmtDone = 1;
			break;
		}
		if( stateHasLeftHand && icLeftHand ){
			if(node->type == kTK_OPERATOR){
				if(node->operator == opType_ST){
					icodeTablePush(table, icLeftHand);
					stmtDone = 1;
					break;
				}else if(node->operator == opType_Assign){
					stateHasLeftHand = 0;
					stmt = parseICodeAssignment(parser, icLeftHand);
					if(stmt){
						icodeTablePush(table, stmt);
					}
					stmtDone = 1;
					break;
				}else if(node->operator == opType_LP){
					stmt = parseICodeFNCall(parser, icLeftHand, 0);
					if(stmt){
						icodeTablePush(table, stmt);
					}
					
					tokenAcceptOperator(parser, opType_ST);
					stmtDone = 1;
					break;
				}
			}
		}else if(node->type == kTK_OPERATOR){
			if(node->operator == opType_ST){
				stmtDone = 1;
				break;
			}else if(node->operator == opType_LP){
				tokenRestore(parser,node);
				stmt = parseICodeExpression(parser);
				if(stmt){
					icodeTablePush(table, stmt);
				}else{
					parserRaiseSyntaxError(parser,"Expecting expression.");
					return 0;
				}
				
				stmtDone = 1;
				break;
			}
		}
		if( node->type == kTK_IDENTIFIER ){
		
			if( tokenIsKeyword(node) ){
				printf("is keyword: %d=%d=%s\n", node->type, node->keyword, node->text);
				parseKeyword(parser, table, node->keyword);
				tokenFree(node);
				stmtDone = 1;
				break;
			}else{
				tokenRestore(parser,node);
				icLeftHand = parseICodeLeftHand(parser);
				if(icLeftHand){
					stateHasLeftHand = 1;
				}
			}
			
			continue;
		}
	}while(!stmtDone);
	
	return 1;
}
int parseExpressionStatement(langPARSER parser, ICTAB table){
	return 1;
}
int parseIfStatement(langPARSER parser, ICTAB table){
	
	
	if(!tokenExpectOperator(parser, opType_LP)){
		parserRaiseSyntaxError(parser, "Expecting `(` in IF statement");
		return 0;
	}
	
	ICODE stmt = icodeAllocate(parser, kST_IF);
	stmt->args = icodeTableCreate();
	
	
	ICODE icEXPR = parseICodeExpression(parser);
	if(!icEXPR){
		parserRaiseSyntaxError(parser,"Expecting expression in IF statement.");
		return 0;
	}
	
	//debugDumpStatements(icEXPR->args, "IFEXPR", 1);
	icodePush(stmt, icEXPR);
	
	if(!tokenExpectOperator(parser,opType_RP)){
		parserRaiseSyntaxError(parser, "Expecting `)` in IF statement");
		return 0;
	}
	
	//What var must be created
	ICODE block = NULL;
	
	if(tokenAcceptOperator(parser, opType_LKey)){
		block = parserICodeBlock(parser);
		
	}else{
		block = icodeAllocate(parser, kST_BLOCK);
		block->args = icodeTableCreate();
		parseStatement(parser, block->args);
	}
	
	if(block){
		debugDumpStatements(block->args, "IFBLOCK", 1);
		icodePush(stmt, block);
	}else{
		parserRaiseSyntaxError(parser, "Expecting statements in IF statement");
		return 0;
	}
	
	icodeTablePush(table, stmt);
	
	if(!tokenAcceptKeyword(parser, kST_ELSE)){
		return 1;
	}
	
	ICODE elseBlock = NULL;
	if(tokenAcceptOperator(parser, opType_LKey)){
		elseBlock = parserICodeBlock(parser);
	}else{
		elseBlock = icodeAllocate(parser, kST_BLOCK);
		elseBlock->args = icodeTableCreate();
		parseStatement(parser, elseBlock->args);
	}
	
	if(elseBlock){
		debugDumpStatements(elseBlock->args, "IFBLOCK", 1);
		icodePush(stmt, elseBlock);
	}else{
		parserRaiseSyntaxError(parser, "Expecting statements in IF statement");
		return 0;
	}
	
	return 1;
}
int parseWhileStatement(langPARSER parser, ICTAB table){
    
    
    if(!tokenExpectOperator(parser, opType_LP)){
        parserRaiseSyntaxError(parser, "Expecting `(` in WHILE statement");
        return 0;
    }
    
    ICODE stmt = icodeAllocate(parser, kST_WHILE);
    stmt->args = icodeTableCreate();
    
    
    ICODE icEXPR = parseICodeExpression(parser);
    if(!icEXPR){
        parserRaiseSyntaxError(parser,"Expecting expression in WHILE statement.");
        return 0;
    }
    
    //debugDumpStatements(icEXPR->args, "IFEXPR", 1);
    icodePush(stmt, icEXPR);
    
    if(!tokenExpectOperator(parser,opType_RP)){
        icodeFree(stmt);
        parserRaiseSyntaxError(parser, "Expecting `)` in WHILE statement");
        return 0;
    }
    
    //What var must be created
    ICODE block = NULL;
    
    if(tokenAcceptOperator(parser, opType_LKey)){
        block = parserICodeBlock(parser);
        
    }else{
        block = icodeAllocate(parser, kST_BLOCK);
        block->args = icodeTableCreate();
        parseStatement(parser, block->args);
    }
    
    if(block){
        //debugDumpStatements(block->args, "WHILEBLOCK", 1);
        icodePush(stmt, block);
    }else{
        icodeFree(stmt);
        parserRaiseSyntaxError(parser, "Expecting statements in WHILE statement");
        return 0;
    }
    
    icodeTablePush(table, stmt);
    return 1;
}
void parseFormalParameter(langPARSER parser, ICTAB table, OBJECT fnObj){
    
    int argc = 0;
     
    struct langNode * node;
    node = tokenNext(parser);
    
    debugDumpToken(node, "PARAMTOKEN");
    
    if(!node || (node->type != kTK_IDENTIFIER) ){
        if(node) tokenRestore(parser, node);
        parserRaiseSyntaxError(parser, "expecting identifier in parameter declaration");
        return;
    }
    
    if( tokenIsKeyword(node) ){
        if(node) tokenRestore(parser, node);
        parserRaiseSyntaxError(parser, "reserved keyword in parameter declaration");
        return;
    }
    
    argc = (++fnObj->paramCount);
    if(argc == 1){
        fnObj->params = (fnParameter *) malloc(sizeof(fnParameter) );
    }else{
        fnObj->params = (fnParameter *) realloc(fnObj->params, argc * sizeof(fnParameter) );
    }
    fnParameter *e = &fnObj->params[argc - 1];
    
    e->idx = argc-1;
    e->name = strdup((char *) node->text);
    e->initValue = NULL;
    
    node = tokenNext(parser);
    if(!node || node->type != kTK_OPERATOR){
        if(node) tokenRestore(parser, node);
        return;
    }
    
    if(node->operator == opType_Assign){
        tokenFree(node);

        e->initValue = parseICodeExpression(parser);
        if(! e->initValue){
            parserRaiseSyntaxError(parser,"expecting expression in assigment.");
        }
        
        
        node = tokenNext(parser);
        if(!node || (node->type != kTK_OPERATOR)){
            return;
        }
    }
    
    if(node->operator == opType_Comma){
        tokenFree(node);
        parseFormalParameter(parser, table, fnObj);
    }else if(node->operator == opType_RP){
        tokenRestore(parser, node);
    }else{
        if(node) tokenRestore(parser, node);
        return;
    }
    
    
    return;
}
int parseFuncStatement(langPARSER parser, ICTAB table){
    //FUNCTION NAME (ARGS...){}
    //FUNCTION (ARGS...){}
    
    
    ICTAB icParams = icodeTableCreate();
    
    ICODE fnDeclare = icodeAllocate(parser, kST_FUNCTION);
    fnDeclare->args = icodeTableCreate();
    

    struct langNode * node;
    node = tokenNext(parser);
    if(!node){
        icodeFree(fnDeclare);
        return 0;
    }
    
    CString name;
   
    int isClosure = 0;
    if(node->type == kTK_IDENTIFIER){
        name = strdup((char *) node->text);
        
        node = tokenNext(parser);
        if(!node){
            icodeFree(fnDeclare);
            parserRaiseSyntaxError(parser, "expecting formal parameters in function declaration");
            return 0;
        }
        
    }else{
        isClosure = 1;
        name = langCreateID("FNANOM", 10);
    }
    
    VALUE vObj = valueMakeObject(parser->st, kObjTypeFunc, name);
    if(!vObj){
        icodeFree(fnDeclare);
        parserRaiseError(parser, "Unable to create function definition.");
        return 0;
    }
    
    
    fnDeclare->ref = vObj;
    OBJECT fnObj = vObj->value.obj;
    
    strncpy(fnObj->name, name, kSZ_MAX_FN_NAME);
    
    fnObj->isClosure = isClosure;
    fnObj->paramCount = 0;
    
    if( (node->type != kTK_OPERATOR) || (node->operator != opType_LP)){
        tokenRestore(parser, node);
        icodeFree(fnDeclare);
        parserRaiseSyntaxError(parser, "expecting formal parameters in function declaration");
        return 0;
    }
    
    parseFormalParameter(parser, icParams, fnObj);
    if(!tokenExpectOperator(parser, opType_RP)){
        parserRaiseSyntaxError(parser, "Expecting `(` in WHILE statement");
        return 0;
    }
    
    
    if(!tokenExpectOperator(parser, opType_LKey)){
        icodeFree(fnDeclare);
        parserRaiseSyntaxError(parser, "expecting function block");
        return 0;
    }
    
    fnObj->fnBlock = parserICodeBlock(parser);
    
    if(!fnObj->fnBlock){
        icodeFree(fnDeclare);
        parserRaiseSyntaxError(parser, "expecting function block");
        return 0;
    }
    
    
    //debugDumpStatements(fnObj->fnBlock->args, "FNBLOCK", 1);
    icodeTablePush(table, fnDeclare);
    
    return 1;
}

ICODE parserICodeBlock(langPARSER parser){
	
	if(!parser) return NULL;
	
	parser->flgInBlock = 1;
	parser->flgInFunction = 0;
	
	ICODE icBlock = icodeAllocate(parser, kST_BLOCK);
	icBlock->args = icodeTableCreate();
	
	ICTAB table = icBlock->args;
	
	struct langNode * node;
	int stateHasLeftHand = 0;
	ICODE icLeftHand;
	ICODE stmt;
	
	while(1){
		if(parser->st->abort) {
			if(node){
				tokenFree(node);
			}
			break;
		}
		
		node = tokenNext(parser);
		if(!node){
			break;
		}
		if(node->type == kTK_OPERATOR){
			if(node->operator == opType_RKey){
				tokenFree(node);
				return icBlock;
			}
		}
		
		if( stateHasLeftHand && icLeftHand ){
			if(node->type == kTK_OPERATOR){
					
				if(node->operator == opType_ST){
					icodeTablePush(table, icLeftHand);
					continue;
				}else if(node->operator == opType_Assign){
					stateHasLeftHand = 0;
					stmt = parseICodeAssignment(parser, icLeftHand);
					if(stmt){
						icodeTablePush(table, stmt);
					}
					continue;
				}else if(node->operator == opType_LP){
					stmt = parseICodeFNCall(parser, icLeftHand, 0);
					if(stmt){
						icodeTablePush(table, stmt);
					}
					
					tokenAcceptOperator(parser, opType_ST);
					continue;
				}
			}
		}
		if( node->type == kTK_IDENTIFIER ){
		
			if( tokenIsKeyword(node) ){
				//printf("is keyword: %d=%d=%s\n", node->type, node->keyword, node->text);
				parseKeyword(parser, table, node->keyword);
				tokenFree(node);
				continue;
			}else{
				tokenRestore(parser,node);
				icLeftHand = parseICodeLeftHand(parser);
				if(icLeftHand){
					stateHasLeftHand = 1;
				}
			}
			
			continue;
		}
	}//end while
			
	return icBlock;
}
		
ICODE parseICodeExpression(langPARSER parser){
	
	ICODE icEXPR = icodeAllocate(parser, kST_EXPR);
	if(!icEXPR){
		return NULL;
	}
	
	icEXPR->args = icodeTableCreate();
	parseExpression(parser, icEXPR->args);
	
	return icEXPR;
}
ICODE parseICodeFNCall(langPARSER parser, ICODE icFNREF, int opConsumeLP){
	
	if(!icFNREF) return NULL;
	

	ICODE icFNCALL = icodeAllocate(parser, kST_FNCALL);
	icFNCALL->args = icodeTableCreate();
	
	icodePush(icFNCALL, icFNREF); //save ref
	
	if(opConsumeLP){
		if(!tokenExpectOperator(parser, opType_LP)){
			parserRaiseSyntaxError(parser,"expecting an `(` in function call.");
			icodeFree(icFNCALL);
			icodeFree(icFNREF);
			return NULL;
		}
	}
	
	if(tokenAcceptOperator(parser, opType_RP)){
		return icFNCALL;
	}
	
	//parse arguments
	langNODE node = NULL;
	do{
		if(node) tokenFree(node);
		
		ICODE icEXPR = parseICodeExpression(parser);
		if(!icEXPR){
			parserRaiseSyntaxError(parser,"expecting an `(` in function call.");
		}else{
			icodePush(icFNCALL, icEXPR);
		}
		
		node = tokenNext(parser);
	}while(node && (node->type == kTK_OPERATOR) && (node->operator == opType_Comma));
	
	if(node && (node->type == kTK_OPERATOR) && (node->operator == opType_RP)){
		tokenFree(node);
		return icFNCALL;
	}
	
	if(node) tokenFree(node);
	parserRaiseSyntaxError(parser, "expecting closing `)` in function call");
	return icFNCALL;
	
}
ICODE parseICodeLeftHand(langPARSER parser){
	
	
	ICODE icLREF = icodeAllocate(parser, kST_GETREF);
	icLREF->args = icodeTableCreate();
	
	langNODE node = tokenNext(parser);
	if(!node){
		parserRaiseSyntaxError(parser,"expecting an identifier");
		return NULL;
	}
	
	if(node->type == kTK_OPERATOR){
		debugDumpToken(node, "LEFTHAND_OP");
		if(node->operator != opType_LP){
			tokenRestore(parser, node);
			parserRaiseSyntaxError(parser,"expecting an identifier");
			return NULL;
		}
	
		tokenFree(node);
		parseExpressionSimple(parser, icLREF->args);
		
		if(tokenAcceptOperator(parser, opType_RP)){
			parserRaiseSyntaxError(parser, "expecting closing `)` in expression");
			return icLREF;
		}
		
	}else if(node->type == kTK_IDENTIFIER){
		ICODE icMember = icodeAllocate(parser, kST_LITERAL);
		icMember->ref = valueCreate(parser->st, kValueString);
		icMember->ref->value.string = strdup((char *) node->text);
		icodePush(icLREF, icMember);
		
		tokenFree(node);
	}else{
		tokenRestore(parser, node);
		parserRaiseSyntaxError(parser,"expecting an identifier");
		return NULL;
	}
	


	node = tokenNext(parser);
	if(!node){
		return icLREF;
	}

	if(node->type != kTK_OPERATOR){
		tokenRestore(parser,node);
		return icLREF;
	}
	if(node->operator == opType_Dot){
		tokenFree(node);
		
		node = tokenNext(parser);
		if(!node || (node->type != kTK_IDENTIFIER) ){
			if(node) tokenRestore(parser, node);
			parserRaiseSyntaxError(parser, "expecting identifier in expression");
			return icLREF;
		}
		
		if( tokenIsKeyword(node) ){
			tokenRestore(parser, node);
			parserRaiseSyntaxError(parser, "reserved keyword in expression");
			return icLREF;
		}

		tokenRestore(parser,node);
		ICODE stmt = parseICodeLeftHand(parser);
		if(stmt){
			icodePush(icLREF, stmt);
		}
	}else if(node->operator == opType_LBracket){
		tokenFree(node);
		parseExpressionSimple(parser, icLREF->args);
		
		if(!tokenAcceptOperator(parser, opType_RBracket)){
			parserRaiseSyntaxError(parser, "expecting closing `]` in expression");
			return icLREF;
		}
	}else{
		tokenRestore(parser,node);
	}

	debugDumpStatements(icLREF->args, "LEFTHAND", 2);
	return icLREF;
}
int parseIdentifier(langPARSER parser,  ICTAB table, char *name, ICODE parent){
	
	langNODE node;
	
	
	ICODE  e = icodeAllocate(parser, kST_IDENTIFIER);
	e->name = strdup((char*)name);
	
	int flgParseIdentifierOnly = 0;
	if(parent && parent->type == kST_OBJMEMBER){
		flgParseIdentifierOnly = 1;
	}
	
	node = tokenNext(parser);
	if(!node){
		icodeTablePush(table, e);
		return 1;
	}
	
	if(node->type != kTK_OPERATOR){
		tokenRestore(parser, node);
		icodeTablePush(table, e);
		return 1;
	}
	
	if(node->operator == opType_ST){
		tokenRestore(parser,node);
		icodeTablePush(table, e);
		return 1;
	}
	
	if(node->operator == opType_Dot ){
		free(node);
		e->type = kST_OBJMEMBER;
		e->args = icodeTableCreate();
		
		node = tokenNext(parser);
		if(!node || (node->type != kTK_IDENTIFIER)){
			parserRaiseSyntaxError(parser,"expecting an identifier");
			return 0;
		}
		parseIdentifier(parser, e->args, (char *) node->text, e);
		free(node);
		
		icodeTablePush(table, e);
		return 1;
			
	}else if(node->operator == opType_LP){
	free(node);
		e->type = kST_FNCALL;
		e->args = icodeTableCreate();
		parseExpression(parser, e->args);
		
		if(!tokenExpectOperator(parser, opType_RP)){
			parserRaiseSyntaxError(parser,"expecting an identifier");
			return 0;
		}
		icodeTablePush(table, e);
		
	}else{
		tokenRestore(parser,node);
		icodeTablePush(table, e);
	}

	return 1;
}

ICODE parseICodeAssignment(langPARSER parser, ICODE leftSide){
	
	if(!leftSide) return NULL;
	
	ICODE icAssigment = icodeAllocate(parser, kST_ASSIGNMENT);
	icAssigment->args = icodeTableCreate();
	
	icodePush(icAssigment, leftSide);
    
    ICODE icEXPR = parseICodeExpression(parser);
    if(!icEXPR){
        parserRaiseSyntaxError(parser,"Expecting expression in assignment statement.");
        return 0;
    }
    
    //debugDumpStatements(icEXPR->args, "IFEXPR", 1);
    icodePush(icAssigment, icEXPR);
    
	//parseExpression(parser, icAssigment->args);
	
	if(!tokenExpectOperator(parser, opType_ST)){
		parserRaiseSyntaxError(parser, "expecting statement terminator ;");
		return icAssigment;
	}
	
	return icAssigment;
}



int parseFormalParams(langPARSER parser, ICTAB table){
	
	struct langNode * node;
	node = tokenNext(parser);
	
	debugDumpToken(node, "VARTOKEN");
	
	if(!node || (node->type != kTK_IDENTIFIER) ){
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "expecting identifier in variable declaration");
		return 0;
	}
	
	if( tokenIsKeyword(node) ){
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "reserved keyword in variable declaration");
		return 0;
	}
	
	ICODE e = icodeAllocate(parser, kST_BUILTIN);
	e->code = kST_VARINIT;
	
	printf("parseVarDeclaration [%s]\n", e->name);
	
	VALUE var = valueCreate(parser->st, kValueUndefined);
	var->name = strdup(e->name);
	
	//symTabInsert(parser->st, fn->args, var);
	
	//printf("var[%d]=[%s]\n", cs->table->length, cs->table->items[cs->table->length-1]->name);
	free(node);
	
	node = tokenNext(parser);
	
	if(!node || node->type != kTK_OPERATOR){
		if(node) tokenRestore(parser, node);
		parserRaiseSyntaxError(parser, "expecting statement terminator \";\" in variable declaration");
		return 0;
	}
	
	/* = | , | ; */
	if(node->operator == opType_Equal){
		e->args = icodeTableCreate();
		parseExpression(parser, e->args);
		debugDumpStatements( e->args,"VAR SLIST", 1);
		icodeTablePush(table, e);
		
		node = tokenNext(parser);
		if(!node || (node->type != kTK_OPERATOR)){
			parserRaiseSyntaxError(parser, "expecting statement terminator \";\" in variable declaration");
			return 0;
		}
	}
	
	
	if(node->operator == opType_Comma){
		parseDeclarationVariable(parser, table);
	}else if(node->operator == opType_ST){
		printf("var terminated\n");
	}else{
		parserRaiseSyntaxError(parser, "expecting statement terminator \";\" in variable declaration");
		return 0;
	}
	
	
	return 1;
}

void debugDumpToken(langNODE node, char* prefix){
	if(!node){
		printf("%s NULL\n",prefix);
		return;
	}
	if(node->type == kTK_OPERATOR){
		printf("%s OPERATOR [%c]\n",prefix, node->operator);
	}else if(node->type == kTK_STRING){
		printf("%s STRING [%s]\n",prefix, node->text);
	}else if(node->type == kTK_INTEGER){
		printf("%s NUMBER [%ld]\n",prefix, node->vInteger);
    }else if(node->type == kTK_FLOAT){
        printf("%s NUMBER [%f]\n",prefix, node->vFloat);
    }else if(node->type == kTK_IDENTIFIER){
		printf("%s IDENTIFIER [%s]\n",prefix, node->text);
	}else{
		printf("%s OTHER TYPE=%d\n",prefix, node->type);
	}
}
