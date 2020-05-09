/** Stiuca Roxana **/

#include "lib.h"

/**
 * Parses commands and calls the respective function for each.
 * Input: root - the root of the tree on which the commands are applied;
 *		  commandsInput - the name of the input file;
 *		  commandsOutput - the name of the output file.
 * Output: N\A.
 */
void ResolveCommands(TArb root, char *commandsInput, char *commandsOutput) {
	FILE *fin = fopen(commandsInput, "rt");
	FILE *fout = fopen(commandsOutput, "wt");

	if (!fin || !fout) {
		fprintf(stderr, "Cannot open files for commands.\n");
		if (fin) fclose(fin);
		if (fout) fclose(fout);
		return;
	}

	int i, commandsCount, tokens;
	tokens = fscanf(fin, "%d", &commandsCount);
	if (tokens != 1) {
		fprintf(stderr, "First line must contain a number.\n");
		if (fin) fclose(fin);
		if (fout) fclose(fout);
		return;
	}

	for (i = 0; i < commandsCount; i++) {
		char command[50];
		fscanf(fin, "%s", command);

		if (strcmp(command, "format") == 0) {
			formatCode(fout, root, 0);
		}
		else if (strcmp(command, "add") == 0) {
			addTagCommand(fin, fout, root);
		}
		else if (strcmp(command, "deleteRecursively") == 0) {
			deleteRecursivelyCommand(fin, fout, root);
		}
		else if (strcmp(command, "overrideStyle") == 0) {
			changeStyleCommand(fin, fout, root, 1);
		}
		else if (strcmp(command, "appendStyle") == 0) {
			changeStyleCommand(fin, fout, root, 0);
		}
	}

	fclose(fin);
	fclose(fout);
}

int main(int argc, char* argv[]) {
	if(argc != 4) {
		fprintf(stderr, "Invalid number of arguments.\n");
		return -1;
	}

	TArb root = ParseInput(argv[1]);
	ResolveCommands(root, argv[2], argv[3]);
	freeTArb(root);

	return 0;	
}

/**
 * //////////////DEBUG///////////////////////
 * Unused.
 * Left for further debugging, if necessary.
 */
void printArb(TArb arb) {
	if(arb == NULL) {
		return;
	}

	printf("#%s# ", arb->info->type);
	printf("id = #%s#\n", arb->info->id);

	printf("\tstyle attr:\n");
	TAttr l = arb->info->style;
	while(l != NULL) {
		printf("\t-> name:#%s# value=#%s#\n", l->name, l->value);
		l = l->next;
	}

	printf("\tother attr:\n");
	l = arb->info->otherAttributes;
	while(l != NULL) {
		printf("\t-> name:#%s# value=#%s#\n", l->name, l->value);
		l = l->next;
	}

	if(arb->info->isSelfClosing) {
		printf("\tis self closing\n");
	}

	if(arb->info->contents != NULL) {
		printf("\tcontents:#%s#\n", arb->info->contents);
	}

	printArb(arb->firstChild);
	printArb(arb->nextSibling);
}

