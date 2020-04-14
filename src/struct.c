/** Stiuca Roxana **/

#include "lib.h"

/**
 * Initializes empty TAttr node.
 */
TAttr InitTAttr() {
	TAttr aux = (TAttr) calloc(1, sizeof(TNodAttr));

	if(aux) {
		aux->name = NULL;
		aux->value = NULL;
		aux->next = NULL;
	}

	return aux;
}

/**
 * Initializes empty TInfo element.
 */
TInfo InitTInfo() {
	TInfo aux = (TInfo) calloc(1, sizeof(TNodInfo));

	if(aux) {
		aux->type = NULL;
		aux->id = NULL;
		aux->style = NULL;
		aux->otherAttributes = NULL;
		aux->isSelfClosing = 0;
		aux->contents = NULL;
	}

	return aux;
}

/**
 * Initializes empty TArb node.
 */
TArb InitTArb() {
	TArb aux = (TArb) calloc(1, sizeof(TNodArb));

	if(aux) {
		aux->info = InitTInfo();

		if(!aux->info) {
			free(aux);
			return NULL;
		}
		
		aux->nextSibling = NULL;
		aux->firstChild = NULL;
	}

	return aux;
}

/**
 * Checks if a node is of a certain class.
 */
int isClass(TArb arb, char *className) {
	if(arb == NULL) {
		return 0;
	}

	TAttr attr = arb->info->otherAttributes;

	while(attr != NULL && strcmp(attr->name, "class") != 0) {
		attr = attr->next;
	}

	if(attr != NULL && strcmp(attr->value, className) == 0) {
		return 1;
	}

	return 0;
}

/**
 * Frees memory of a TAttr list.
 */
void freeTAttr(TAttr attr) {
	if(attr == NULL) {
		return;
	}

	if(attr->name)	free(attr->name);
	if(attr->value)	free(attr->value);
	freeTAttr(attr->next);
	free(attr);
}

/**
 * Frees memory of a TInfo element.
 */
void freeTInfo(TInfo info) {
	if(info == NULL) {
		return;
	}

	if(info->type)		free(info->type);
	if(info->id)		free(info->id);
	if(info->contents)	free(info->contents);
	freeTAttr(info->style);
	freeTAttr(info->otherAttributes);
	free(info);
}

/**
 * Frees memory of a TArb tree.
 */
void freeTArb(TArb arb) {
	if(arb == NULL) {
		return;
	}

	TArb child = arb->firstChild;

	while(child != NULL) {
		TArb aux = child;
		child = child->nextSibling;
		freeTArb(aux);
	}

	freeTInfo(arb->info);
	free(arb);
}

/**
 * Frees memory of a CSS selector.
 */
void freeTSelector(TSelector s) {
	if(s.id)			free(s.id);
	if(s.className)		free(s.className);
	if(s.type)			free(s.type);
	if(s.parentType)	free(s.parentType);
	if(s.ancestorType)	free(s.ancestorType);
}

/**
 * Checks if a memory allocation failed.
 */
void CheckMemoryError(void *p) {
	if(p == NULL) {
		fprintf(stderr, "Memory error.\n");
		exit(-1);
	}
}