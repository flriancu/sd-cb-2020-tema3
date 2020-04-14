/** Stiuca Roxana **/

#include "lib.h"

/**
 * Sets *curr to be the index of the last non-whitespace character in
 * a string.
 * Input: buf - string, curr - pointer to index of last non-whitespace.
 * Output: N\A.
 */
void trimLeadingWhitespace(char *buf, int *curr) {
	while(*curr > 0 && isspace(buf[(*curr)-1])) {
		(*curr)--;
	}
}

/**
 * Parses input file until end of CSS selector (") and creates a
 * TSelector with the information read.
 * Input: fin - the input file.
 * Output: The CSS selector.
 */
TSelector ParseSelector(FILE *fin) {
	TSelector cssSelector;

	cssSelector.id = cssSelector.className = cssSelector.type = NULL;
	cssSelector.parentType = cssSelector.ancestorType = NULL;

	char buf[100];
	int curr = 0;

	char c = fgetc(fin);

	while(c != '\"') {
		if(c == '.') {
			/* read class */
			curr = 0;
			c = fgetc(fin);

			while(c != '\"') {
				buf[curr++] = c;
				c = fgetc(fin);
			}

			buf[curr++] = '\0';

			cssSelector.className = (char *) calloc(curr + 1, sizeof(char));
			CheckMemoryError(cssSelector.className);
			memcpy(cssSelector.className, buf, curr);
		} else if(c == '#') {
			/* read id */
			curr = 0;
			c = fgetc(fin);

			while(c != '\"') {
				buf[curr++] = c;
				c = fgetc(fin);
			}

			buf[curr++] = '\0';

			cssSelector.id = (char *) calloc(curr + 1, sizeof(char));
			CheckMemoryError(cssSelector.id);
			memcpy(cssSelector.id, buf, curr);
		} else {
			// read element, check last character for '>' '.' '\"' ' '
			curr = 0;

			while(c != '\"' && c != ' ' && c != '>' && c != '.') {
				buf[curr++] = c;
				c = fgetc(fin);
			}

			buf[curr++] = '\0';

			if(c == '\"' || c == '.') {
				cssSelector.type = (char *) calloc(curr + 1, sizeof(char));
				CheckMemoryError(cssSelector.type);
				memcpy(cssSelector.type, buf, curr);
			} else if(c == '>') {
				cssSelector.parentType = (char *) calloc(curr + 1, sizeof(char));
				CheckMemoryError(cssSelector.parentType);
				memcpy(cssSelector.parentType, buf, curr);

				c = fgetc(fin);
			} else if(c == ' ') {
				cssSelector.ancestorType = (char *) calloc(curr + 1, sizeof(char));
				CheckMemoryError(cssSelector.ancestorType);
				memcpy(cssSelector.ancestorType, buf, curr);

				c = fgetc(fin);
			}
		}
	}

	return cssSelector;
}

/**
 * Puts in buf string the characters read while currentState doesn't
 * change.
 * Input: fin - the input file;
 *        state - the current state;
 *        buf - buffer for the new information;
 *        *curr - the length of the word read.
 * Output: the new state.
 */
TParseState ParseCurrentState(
	FILE *fin, TParseState state,
	char *buf, int *curr
	) {

	*curr = 0;

	char c = fgetc(fin);
	TParseState currentState = Interpret(state, c);

	while(c != EOF && currentState == state) {
		buf[(*curr)++] = c;
		c = fgetc(fin);
		currentState = Interpret(currentState, c);
	}

	trimLeadingWhitespace(buf, curr);
	buf[(*curr)++] = '\0';

	return currentState;
}

/**
 * Creates list of style attributes contained in a string.
 * Input: attrStr - string with info of style attributes.
 * Output: the head of the list of style attributes.
 */
TAttr ParseStyleAttribute(char *attrStr) {
	TAttr head = NULL, last = NULL;

	int len = (int)strlen(attrStr);
	char buf[100];
	int curr = 0, i;

	for(i = 0; i < len; i++) {
		if(isspace(attrStr[i])) {
			continue;
		} else if(attrStr[i] == ':') {
			//put buf in attr->name
			TAttr aux = InitTAttr();
			CheckMemoryError(aux);

			if(last == NULL) {
				head = last = aux;
			} else {
				last->next = aux;
				last = aux;
			}

			buf[curr++] = '\0';

			aux->name = (char *) calloc(curr + 1, sizeof(char));
			CheckMemoryError(aux->name);
			memcpy(aux->name, buf, curr);

			curr = 0; //incep sa citesc value
		} else if(attrStr[i] == ';') {
			buf[curr++] = '\0';

			last->value = (char *) calloc(curr + 1, sizeof(char));
			CheckMemoryError(last->value);
			memcpy(last->value, buf, curr);

			curr = 0;
		} else {
			buf[curr++] = attrStr[i];
		}
	}

	return head;
}

/**
 * Parses html code and creates recursively tree. Mostly uses the
 * states specified by Interpret.
 * Input: fin - the input file;
 *        currentState - the current parse state;
 *        id - the id of the current node.
 * Output: new node of tree.
 */
TArb ParseArb(FILE *fin, TParseState currentState, char *id) {
	TArb arb = InitTArb();
	CheckMemoryError(arb);

	arb->info->id = (char *) calloc(strlen(id) + 1, sizeof(char));
	CheckMemoryError(arb->info->id);
	if(strlen(id) >= 1) {
		memcpy(arb->info->id, id + 1, strlen(id) - 1);
		arb->info->id[strlen(id)] = '\0';
	}

	TArb lastChild = NULL;
	int currChild = 1;

	TAttr lastOtherAttr = NULL;

	char c = fgetc(fin);
	currentState = Interpret(currentState, c);

	char buf[1000];
	int curr = 0;

	while(c != EOF && currentState != PARSE_CLOSING_TAG) {
		if(currentState == PARSE_TAG_TYPE) {
			if(arb->info->type != NULL) {
				fseek(fin, -1, SEEK_CUR);

				char id_aux[100];
				sprintf(id_aux, "%s.%d", id, currChild++);

				TArb child = ParseArb(fin, currentState, id_aux);

				if(lastChild == NULL) {
					arb->firstChild = lastChild = child;
				} else {
					lastChild->nextSibling = child;
					lastChild = child;
				}


				currentState = PARSE_CONTENTS;
			} else {
				fseek(fin, -1, SEEK_CUR);
				currentState = ParseCurrentState(fin, currentState, buf,&curr);

				arb->info->type = (char *) calloc(curr + 1, sizeof(char));
				CheckMemoryError(arb->info->type);
				memcpy(arb->info->type, buf, curr);

				fseek(fin, -1, SEEK_CUR);
			}
		}

		if(currentState == PARSE_ATTRIBUTE_NAME) {
			TAttr attr = InitTAttr();
			CheckMemoryError(attr);

			fseek(fin, -1, SEEK_CUR);
			currentState = ParseCurrentState(fin, currentState, buf, &curr);
			attr->name = (char *) calloc(curr + 1, sizeof(char));
			CheckMemoryError(attr->name);
			memcpy(attr->name, buf, curr);

			while(c != EOF && currentState != PARSE_ATTRIBUTE_VALUE) {
				c = fgetc(fin);
				currentState = Interpret(currentState, c);
			}

			//fseek(fin, -1, SEEK_CUR);
			currentState = ParseCurrentState(fin, currentState, buf, &curr);
			attr->value = (char *) calloc(curr + 1, sizeof(char));
			CheckMemoryError(attr->value);
			memcpy(attr->value, buf, curr);

			if(strcmp(attr->name, "style") == 0) {
				arb->info->style = ParseStyleAttribute(attr->value);
				freeTAttr(attr);
			} else {
				if(lastOtherAttr == NULL) {
					arb->info->otherAttributes = lastOtherAttr = attr;
				} else {
					lastOtherAttr->next= attr;
					lastOtherAttr = attr;
				}
			}

			//fseek(fin, -1, SEEK_CUR);
		}

		if(currentState == PARSE_SELF_CLOSING) {
			arb->info->isSelfClosing = 1;
			break;
		}

		if(currentState == PARSE_CONTENTS) {
			c = fgetc(fin);
			currentState = Interpret(currentState, c);
			while(c != EOF && currentState == PARSE_CONTENTS && (isspace(c) || c == '>')) {
				c = fgetc(fin);
				currentState = Interpret(currentState, c);
			}

			if(currentState == PARSE_CONTENTS) {
				fseek(fin, -1, SEEK_CUR);
				currentState = ParseCurrentState(fin, currentState, buf, &curr);
				arb->info->contents = (char *) calloc(curr + 1, sizeof(char));
				CheckMemoryError(arb->info->contents);
				memcpy(arb->info->contents, buf, curr);
			}
		}

		c = fgetc(fin);
		currentState = Interpret(currentState, c);
	}

	while(c != EOF && currentState == PARSE_CLOSING_TAG) {
		c = fgetc(fin);
		currentState = Interpret(currentState, c);
	}

	return arb;
}

/**
 * Parses input for html code and creates root of tree.
 * Input: inputName - name of input file.
 * Output: root of the html code's tree.
 */
TArb ParseInput(char *inputName) {
	FILE *fin = fopen(inputName, "rt");

	if(fin == NULL) {
		fprintf(stderr, "Cannot open file %s.\n", inputName);
		exit(-1);
	}

	char id[100];
	id[0] = '\0';

	TArb root = ParseArb(fin, PARSE_CONTENTS, id);
	fclose(fin);

	return root;
}