/** Stiuca Roxana **/

#include "lib.h"

/**
 * Prints a certain number of tabs.
 * Input: fout - the output file;
 *		  tabs - the number of tabs needed.
 * Output: N\A.
 */
void putTabs(FILE *fout, int tabs) {
	int i;

	for(i = 0; i < tabs; i++) {
		fprintf(fout, "\t");
	}
}

/**
 * Parse the list of style attributes and prints them in the right format.
 * Input: fout - the output file;
 *		  aux - the list of attributes.
 * Output: N\A.
 */
void formatStyleAttributes(FILE *fout, TAttr aux) {
	if(!aux) {
		return;
	}

	fprintf(fout, " style=\"");

	while(aux) {
		fprintf(fout, "%s: %s;", aux->name, aux->value);
		
		if(aux->next) {
			fprintf(fout, " ");
		}

		aux = aux->next;
	}

	fprintf(fout, "\"");
}

/**
 * Parse the list of other attributes and prints them in the right format.
 * Input: fout - the output file;
 *		  aux - the list of attributes.
 * Output: N\A.
 */
void formatOtherAttributes(FILE *fout, TAttr aux) {
	if(!aux) {
		return;
	}

	fprintf(fout, " ");

	while(aux) {
		fprintf(fout, "%s=\"%s\"", aux->name, aux->value);
		if(aux->next) {
			fprintf(fout, " ");
		}

		aux = aux->next;
	}
}

/**
 * Corresponds to the "format" command. Prints the tree.
 * Input: fout - the output file;
 * 		  arb - the node of the tree that is formatted now;
 *        tabs - the number of tabs necessary at the start of each line.
 * Output: N\A.
 */
void formatCode(FILE *fout, TArb arb, int tabs) {
	if(!arb) {
		return;
	}

	/* Format the initial line: <tag style=".."> */
	putTabs(fout, tabs);
	fprintf(fout, "<%s", arb->info->type);
	formatStyleAttributes(fout, arb->info->style);
	formatOtherAttributes(fout, arb->info->otherAttributes);

	if(arb->info->isSelfClosing) {
		fprintf(fout, "/>\n");
		return;
	}

	fprintf(fout, ">\n");

	if(arb->info->contents) {
		putTabs(fout, tabs + 1);
		fprintf(fout, "%s\n", arb->info->contents);
	}

	/* Format children tags, one tab more from the parent */
	formatCode(fout, arb->firstChild, tabs + 1);

	/* Format last line: <tag/> */
	putTabs(fout, tabs);
	fprintf(fout, "</%s>\n", arb->info->type);

	/* Format sibling tags */
	formatCode(fout, arb->nextSibling, tabs);
}

/**
 * Finds the node with given id and returns it.
 * Input: arb - the tree searched;
 *        id - the id wanted.
 * Output: the node in the tree with the right id.
 */
TArb findId(TArb arb, char *id) {
	if(!arb) {
		return NULL;
	}

	if(strcmp(id, arb->info->id) == 0) {
		return arb;
	}

	TArb child = arb->firstChild;

	/* Search for the right child whose if matches the prefix of
	the given id. */
	while(child != NULL && strncmp(id, child->info->id, 
									strlen(child->info->id))) {
		child = child->nextSibling;
	}

	return findId(child, id);
}

/**
 * Resetes the ids of a tree starting from arb and its children.
 * Input: arb - the node in the tree
 *        id - the new id of the node
 * Output: N\A.
 */
void redoIdOrder(TArb arb, char *id) {
	if(arb == NULL) {
		return;
	}

	memcpy(arb->info->id, id, strlen(id));

	TArb child = arb->firstChild;
	int currChild = 1;

	while(child != NULL) {
		char id_aux[strlen(id) + 3];
		sprintf(id_aux, "%s.%d", id, currChild);
		redoIdOrder(child, id_aux);

		child = child->nextSibling;
		currChild++;
	}
}

/**
 * Corresponds to the "add" command. Searches for the parent's id and
 * appends to its list of children a new node.
 * Input: fin - the input file;
 *        fout - the output file;
 *        root - the root of the html tags' tree.
 * Output: N\A.
 */
void addTagCommand(FILE *fin, FILE *fout, TArb root) {
	char id[100], trash[100];
	fseek(fin, 4, SEEK_CUR);
	fscanf(fin, "%s", id);

	fseek(fin, 10, SEEK_CUR);

	TArb parent = findId(root, id);
	if(parent == NULL) {
		fgets(trash, 100, fin);	// read until endline
		fprintf(fout, "Add tag failed: node with id %s not found!\n", id);
		return;
	}

	if(parent->firstChild == NULL) {
		char id_aux[100];
		sprintf(id_aux, "%s.1", id);

		parent->firstChild = ParseArb(fin, PARSE_CONTENTS, id_aux);
	} else {
		TArb child = parent->firstChild;
		while(child->nextSibling != NULL) {
			child = child->nextSibling;
		}

		char id_aux[100];
		sprintf(id_aux, " %s.%d", id, 1 + atoi(child->info->id + strlen(id)));

		child->nextSibling = ParseArb(fin, PARSE_CONTENTS, id_aux);
	}

	fseek(fin, 1, SEEK_CUR); // skip until endline
}

/**
 * Corresponds to the "deleteRecursively" command when the CSS selector is
 * an id. Searches for the id in the children list of arb and if found
 * deletes it and its sub-trees.
 * Input: arb - the parent's node;
 *		  id - the id searched for
 * Output: 1 if id found, else 0.
 */
int deleteRecursively_Id(TArb arb, char *id) {
	if(arb == NULL) {
		return 0;
	}

	TArb child = arb->firstChild, prevChild = NULL;

	while(child != NULL && strncmp(id, child->info->id, strlen(child->info->id))) {
		prevChild = child;
		child = child->nextSibling;
	}

	if(child == NULL) {
		return 0;
	}

	if(strcmp(id, child->info->id) == 0) {
		if(prevChild == NULL) {
			arb->firstChild = child->nextSibling;
		} else {
			prevChild->nextSibling = child->nextSibling;
		}

		redoIdOrder(arb, arb->info->id);
		freeTArb(child);
		return 1;
	} else {
		return deleteRecursively_Id(child, id);
	}
}

/**
 * Corresponds to the "deleteRecursively" command when the CSS selector is
 * either type, className or both.
 * Input: arb - the parent's node;
 *		  type - the type searched for or NULL if not used;
 *		  className - the class searched for or NULL if not used.
 * Output: 1 if found, else 0.
 */
int deleteRecursively(TArb arb, char *type, char *className) {
	if(arb == NULL) {
		return 0;//not found
	}

	TArb child = arb->firstChild, prevChild = NULL;
	int found = 0;

	while(child != NULL) {
		if((type == NULL || strcmp(type, child->info->type) == 0) &&
			(className == NULL || isClass(child, className))) {

			found = 1;

			if(prevChild == NULL) {
				arb->firstChild = child->nextSibling;
			} else {
				prevChild->nextSibling = child->nextSibling;
			}

			TArb aux = child;
			child = child->nextSibling;
			redoIdOrder(arb, arb->info->id);
			freeTArb(aux);
		} else {
			if(deleteRecursively(child, type, className)) {
				found = 1;
			}
			prevChild = child;
			child = child->nextSibling;
		}
	}

	return found;
}

/**
 * Corresponds to the "deleteRecursively" command when the CSS selector is
 * the parent's type and type, such as div>p.
 * Input: arb - the parent's node;
 *        parentType - the parent's tag type;
 *        type - the type of the child searched for.
 * Output: 1 if found, else 0.
 */
int deleteRecursively_Parent(TArb arb, char *parentType, char *type) {
	if(arb == NULL) {
		return 0;
	}

	int found = 0;

	if(strcmp(arb->info->type, parentType) == 0) {
		TArb child = arb->firstChild, prevChild = NULL;

		while(child != NULL) {
			if(strcmp(type, child->info->type) == 0) {
				found = 1;

				if(prevChild == NULL) {
					arb->firstChild = child->nextSibling;
				} else {
					prevChild->nextSibling = child->nextSibling;
				}

				redoIdOrder(arb, arb->info->id);
				TArb aux = child;
				child = child->nextSibling;
				freeTArb(aux);
			} else {
				prevChild = child;
				child = child->nextSibling;
			}
		}
	}

	if(deleteRecursively_Parent(arb->nextSibling, parentType, type)) {
		found = 1;
	}

	if(deleteRecursively_Parent(arb->firstChild, parentType, type)) {
		found = 1;
	}

	return found;
}

/**
 * Corresponds to the "deleteRecursively" command when the CSS selector is
 * the ancestor's type and type, such as div p.
 * Input: arb - the ancestor's node;
 *        parentType - the ancestor's tag type;
 *        type - the type of the child searched for.
 * Output: 1 if found, else 0.
 */
int deleteRecursively_Ancestor(TArb arb, char *ancestorType, char *type) {
	if(arb == NULL) {
		return 0;
	}

	int found = 0;

	if(strcmp(arb->info->type, ancestorType) == 0) {
		/* If found, then the condition for the ancestor's subtree is
		the same for deleteRecursively when the CSS selector is just
		the tag's type. */
		found = deleteRecursively(arb, type, NULL);
	}

	if(deleteRecursively_Ancestor(arb->nextSibling, ancestorType, type)) {
		found = 1;
	}

	if(deleteRecursively_Ancestor(arb->firstChild, ancestorType, type)) {
		found = 1;
	}

	return found;
}

/**
 * Corresponds to the "deleteRecursively" command. Reads the rest of the
 * command and calls the right function depending on the CSS selector.
 * Input: fin - the input file;
 *        fout - the output file;
 *        root - the root of the html code's tree
 * Output: N\A.
 */
void deleteRecursivelyCommand(FILE *fin, FILE *fout, TArb root) {
	fseek(fin, 11, SEEK_CUR);
	TSelector cssSelector = ParseSelector(fin);
	char *failedMsg = "Delete recursively failed: no node found for selector";

	if(cssSelector.id != NULL) {
		if(!deleteRecursively_Id(root, cssSelector.id)) {
			fprintf(fout, "%s #%s!\n", failedMsg, cssSelector.id);
		}
	} else if(cssSelector.ancestorType != NULL) {
		if(!deleteRecursively_Ancestor(root, cssSelector.ancestorType, 
									cssSelector.type)) {
			fprintf(fout, "%s %s %s!\n", failedMsg, cssSelector.ancestorType,
					cssSelector.type);
		}
	} else if(cssSelector.parentType != NULL) {
		if(!deleteRecursively_Parent(root, cssSelector.parentType,
								cssSelector.type)) {
			fprintf(fout, "%s %s>%s!\n", failedMsg, cssSelector.parentType,
				cssSelector.type);
		}
	} else {
		if(!deleteRecursively(root, cssSelector.type, cssSelector.className)) {
			if(cssSelector.type == NULL) {
				fprintf(fout, "%s .%s!\n", failedMsg, cssSelector.className);
			} else if(cssSelector.className == NULL) {
				fprintf(fout, "%s %s!\n", failedMsg, cssSelector.type);
			} else {
				fprintf(fout, "%s %s.%s!\n", failedMsg, cssSelector.type, 
					cssSelector.className);
			}
		}
	}

	freeTSelector(cssSelector);
}

/**
 * Override a node's style attributes.
 * Input: arb - the node to be modified;
 *        attrStr - string containing the info for the new style attributes.
 * Output: N\A.
 */
void overrideStyle(TArb arb, char *attrStr) {
	freeTAttr(arb->info->style);
	arb->info->style = ParseStyleAttribute(attrStr);
}

/**
 * Appends to a node's style attributes list.
 * Input: arb - the node to be modified;
 *        attrStr - string containing the info for the new style attributes.
 * Output: N\A.
 */
void appendStyle(TArb arb, char *attrStr) {
	TAttr list = ParseStyleAttribute(attrStr);

	TAttr old = arb->info->style, prev_old = NULL;

	/* Start by updating the old attributed. */
	while(old != NULL) {
		TAttr new = list, prev_new = NULL;

		while(new != NULL) {
			if(strcmp(old->name, new->name) == 0) {
				free(old->value);
				old->value = new->value;
				new->value = NULL;

				if(prev_new == NULL) {
					list = new->next;
				} else {
					prev_new->next = new->next;
				}

				TAttr aux = new;
				new = new->next;
				aux->next = NULL;
				freeTAttr(aux);
			} else {
				prev_new = new;
				new = new->next;
			}
		}

		prev_old = old;
		old = old->next;
	}

	/* Appends the attributes that were not found already in the list. */
	if(prev_old == NULL) {
		arb->info->style = list;
	} else {
		prev_old->next = list;
	}
}

/**
 * Corresponds to either the "appendStyle" or "overrideStyle" command
 * (depending on isOverride) when the CSS selector is the id.
 * Input: arb - the current node in the tree;
 *        id - the id searched for;
 *        attrStr - string containing the info for the new style attributes;
 *        isOverride - 1 for overrideStyle command, 0 for appendStyle.
 * Output: 1 if found, else 0.
 */
int changeStyle_Id(TArb arb, char *id, char *attrStr, int isOverride) {
	if(arb == NULL) {
		return 0;
	}

	if(strcmp(arb->info->id, id) == 0) {
		if(isOverride) {
			overrideStyle(arb, attrStr);
		} else {
			appendStyle(arb, attrStr);
		}

		return 1;
	}

	if(changeStyle_Id(arb->nextSibling, id, attrStr, isOverride)) {
		return 1;
	}

	if(changeStyle_Id(arb->firstChild, id, attrStr, isOverride)) {
		return 1;
	}

	return 0;
}

/**
 * Corresponds to either the "appendStyle" or "overrideStyle" command
 * (depending on isOverride) when the CSS selector is type/className/both.
 * Input: arb - the current node in the tree;
 *        type - the type searched for or NULL if unused;
 *        className - the class searched for or NULL if unused;
 *        attrStr - string containing the info for the new style attributes;
 *        isOverride - 1 for overrideStyle command, 0 for appendStyle.
 * Output: 1 if found, else 0.
 */
int changeStyle(
	TArb arb,
	char *type, char *className, char *attrStr,
	int isOverride
	) {

	if(arb == NULL) {
		return 0;
	}

	int found = 0;

	if((type == NULL || strcmp(type, arb->info->type) == 0) &&
		(className == NULL ||  isClass(arb, className))) {

		found = 1;

		if(isOverride) {
			overrideStyle(arb, attrStr);
		} else {
			appendStyle(arb, attrStr);
		}
	}

	if(changeStyle(arb->nextSibling, type, className, attrStr, isOverride)) {
		found = 1;
	}

	if(changeStyle(arb->firstChild, type, className, attrStr, isOverride)) {
		found = 1;
	}

	return found;
}

/**
 * Corresponds to either the "appendStyle" or "overrideStyle" command
 * (depending on isOverride) when the CSS selector is the parentType>type.
 * Input: arb - the current node in the tree;
 *        parentType - the parent's type searched for;
 *        type - the type searched for;
 *        attrStr - string containing the info for the new style attributes;
 *        isOverride - 1 for overrideStyle command, 0 for appendStyle.
 * Output: 1 if found, else 0.
 */
int changeStyle_Parent(
	TArb arb,
	char *parentType, char *type, char *attrStr,
	int isOverride
	) {

	if(arb == NULL) {
		return 0;
	}

	int found = 0;

	if(strcmp(parentType, arb->info->type) == 0) {
		TArb child = arb->firstChild;

		while(child != NULL) {
			if(strcmp(type, child->info->type) == 0) {
				found = 1;

				if(isOverride) {
					overrideStyle(child, attrStr);
				} else {
					appendStyle(child, attrStr);
				}
			}

			child = child->nextSibling;
		}
	}

	if(changeStyle_Parent(arb->nextSibling, parentType, 
			type, attrStr, isOverride)) {

		found = 1;
	}

	if(changeStyle_Parent(arb->firstChild, parentType,
			type, attrStr, isOverride)) {

		found = 1;
	}

	return found;
}

/**
 * Corresponds to either the "appendStyle" or "overrideStyle" command
 * (depending on isOverride) when the CSS selector is the ancestorType type.
 * Input: arb - the current node in the tree;
 *        ancestorType - the ancestor's type searched for;
 *        type - the type searched for;
 *        attrStr - string containing the info for the new style attributes;
 *        isOverride - 1 for overrideStyle command, 0 for appendStyle.
 * Output: 1 if found, else 0.
 */
int changeStyle_Ancestor(
	TArb arb,
	char *ancestorType, char *type, char *attrStr,
	int isOverride
	) {

	if(arb == NULL) {
		return 0;
	}

	int found = 0;

	if(strcmp(ancestorType, arb->info->type) == 0) {
		TArb child = arb->firstChild;

		while(child != NULL) {
			if(changeStyle(child, type, NULL, attrStr, isOverride)) {
				found = 1;
			}

			child = child->nextSibling;
		}
	}

	if(changeStyle_Ancestor(arb->nextSibling, ancestorType,
			type, attrStr, isOverride)) {

		found = 1;
	}

	return found;
}

/**
 * Corresponds to the either "overrideStyle" command or "appendStyle"
 * (depending on isOverride). Reads the rest of the command and calls the
 * right function depending on the CSS selector.
 * Input: fin - the input file;
 *        fout - the output file;
 *        root - the root of the html code's tree
 *        isOverride - 1 for overrideStyle command, 0 for appendStyle.
 * Output: N\A.
 */
void changeStyleCommand(FILE *fin, FILE *fout, TArb root, int isOverride) {
	fseek(fin, 11, SEEK_CUR);
	TSelector cssSelector = ParseSelector(fin);

	fseek(fin, 8, SEEK_CUR);
	char attrStr[100];
	fgets(attrStr, 100, fin);
	attrStr[strlen(attrStr)-2] = '\0';

	char failedMsg[100];
	if(isOverride) {
		strcpy(failedMsg, "Override style failed: no node found for selector");
	} else {
		strcpy(failedMsg, "Append to style failed: no node found for selector");
	}

	if(cssSelector.id != NULL) {
		if(!changeStyle_Id(root, cssSelector.id, attrStr, isOverride)) {
			fprintf(fout, "%s #%s!\n", failedMsg, cssSelector.id);
		}
	} else if(cssSelector.ancestorType != NULL) {
		if(!changeStyle_Ancestor(root, cssSelector.ancestorType,
							cssSelector.type, attrStr, isOverride)) {
			fprintf(fout, "%s %s %s!\n", failedMsg,
				cssSelector.ancestorType, cssSelector.type);
		}
	} else if(cssSelector.parentType != NULL) {
		if(!changeStyle_Parent(root, cssSelector.parentType,
						 cssSelector.type, attrStr, isOverride)) {
			fprintf(fout, "%s %s>%s!\n", failedMsg,
				cssSelector.parentType, cssSelector.type);
		}
	} else {
		if(!changeStyle(root, cssSelector.type,
					cssSelector.className, attrStr, isOverride)) {
			if(cssSelector.type == NULL) {
				fprintf(fout, "%s .%s!\n", failedMsg, cssSelector.className);
			} else if(cssSelector.className == NULL) {
				fprintf(fout, "%s %s!\n", failedMsg, cssSelector.type);
			} else {
				fprintf(fout, "%s %s.%s\n", failedMsg, cssSelector.type, 
					cssSelector.className);
			}
		}
	}

	freeTSelector(cssSelector);
}