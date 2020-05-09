/** Stiuca Roxana **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct TNodAttr {
	char *name;
	char *value;
	struct TNodAttr *next;
} TNodAttr, *TAttr;

typedef struct TNodInfo {
	char *type;		/* tipul tag-ului */
	char *id;		/* id-ul nodului */
	TAttr style;	/* lista care contine toate subatributele definite
					in cadrul atributului style al tag-ului */
	TAttr otherAttributes;	/* lista pt restul atributelor */
	int isSelfClosing;		/* este self-closing sau nu */
	char *contents;			/* continutul */
} TNodInfo, *TInfo;

typedef struct TNodArb {
	TInfo info;
	struct TNodArb *nextSibling;
	struct TNodArb *firstChild;
} TNodArb, *TArb;

typedef enum
{
    PARSE_ERROR             = 0,
    PARSE_CONTENTS          = 1,
    PARSE_OPENING_BRACKET   = 2,
    PARSE_TAG_TYPE          = 3,    /* gata */
    PARSE_CLOSING_TAG       = 4,    /* gata */
    PARSE_REST_OF_TAG       = 5,
    PARSE_ATTRIBUTE_NAME    = 6,     /* gata */
    PARSE_ATTRIBUTE_EQ      = 7,     /* gata */
    PARSE_ATTRIBUTE_VALUE   = 8,     /* gata */
    PARSE_SELF_CLOSING      = 9,
} TParseState;

typedef struct {
	char *id;
	char *className;
	char *type;
	char *parentType;
	char *ancestorType;
} TSelector;

/* same as fseek(fi, x, SEEK_CUR) for text-mode files */
#define SKIP_CHARS(fi, x) for (int i = 0; i < (x); ++i) { char c; fscanf((fi), "%c", &c); }

/* C equivalent to std::fstream's peek */
int fpeek(FILE *stream);

/* struct.c */
TAttr InitTAttr();
TInfo InitTInfo();
TArb InitTArb();
int isClass(TArb arb, char *className);
void freeTAttr(TAttr attr);
void freeTInfo(TInfo info);
void freeTArb(TArb arb);
void freeTSelector(TSelector s);
void CheckMemoryError(void *p);

/* interpret.c */
TParseState Interpret(TParseState currentState, char c);

/* parser.c */
void trimLeadingWhitespace(char *buf, int *curr);
TSelector ParseSelector(FILE *fin);
TParseState ParseCurrentState(
	FILE *fin, TParseState state,
	char *buf, int *curr
	);
TAttr ParseStyleAttribute(char *attrStr);
TArb ParseArb(FILE *fin, TParseState currentState, char *id);
TArb ParseInput(char *inputName);

/* commands.c */
void putTabs(FILE *fout, int tabs);
void formatStyleAttributes(FILE *fout, TAttr aux);
void formatOtherAttributes(FILE *fout, TAttr aux);
void formatCode(FILE *fout, TArb arb, int tabs);

TArb findId(TArb arb, char *id);
void redoIdOrder(TArb arb, char *id);

void addTagCommand(FILE *fin, FILE *fout, TArb root);

int deleteRecursively_Id(TArb arb, char *id);
int deleteRecursively(TArb arb, char *type, char *className);
int deleteRecursively_Parent(TArb arb, char *parentType, char *type);
int deleteRecursively_Ancestor(TArb arb, char *ancestorType, char *type);
void deleteRecursivelyCommand(FILE *fin, FILE *fout, TArb root);

void overrideStyle(TArb arb, char *attrStr);
void appendStyle(TArb arb, char *attrStr);
int changeStyle_Id(TArb arb, char *id, char *attrStr, int isOverride);
int changeStyle(
	TArb arb,
	char *type, char *className, char *attrStr,
	int isOverride
	);
int changeStyle_Parent(
	TArb arb,
	char *parentType, char *type, char *attrStr,
	int isOverride
	);
int changeStyle_Ancestor(
	TArb arb,
	char *ancestorType, char *type, char *attrStr,
	int isOverride
	);
void changeStyleCommand(FILE *fin, FILE *fout, TArb root, int isOverride);

/* html.c - DEBUG */
void printArb(TArb arb);