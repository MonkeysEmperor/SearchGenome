// A C program to implement Ukkonen's Suffix Tree Construction 
// And find all locations of a pattern in string 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#define MAX_CHAR 4


struct SuffixTreeNode {
    struct SuffixTreeNode *children[MAX_CHAR];
    char c;
    int count;
};

typedef struct SuffixTreeNode Node;

Node *newNode(char c)
{
	int i;
    Node *node = (Node*) malloc(sizeof(Node));
    node->c = c;
	node->count = 0;
	for (i = 0; i < MAX_CHAR; i++)
		node->children[i] = NULL;

    return node;
}

Node *createNodesFromStr(Node* root, char* str)
{
	int i;
	Node *node = root->children[0];
    for (i = 0; str[i] != '\0'; i++)
	{
		printf("%c\n", str[i]);
		node = newNode(str[i]);
		node = node->children[0];
	}
	return root;
}

Node *createRoot(char* str)
{
	int i;
	Node *node, *root;
	root = newNode('0');
	node = root->children[0];
    for (i = 0; str[i] != '\0'; i++)
	{
		node = newNode(str[i]);
		node = node->children[0];
	}
	return root;
}

int checkForSubString(Node* root, char* str)
{
	Node *node;
	int i, found = 0;

    for (node = root; str != '\0'; str++)
	{
		found = 0;
		for (i = 0; node->children[i] != NULL && i < MAX_CHAR; i++)
			if (node->children[i]->c == *str)
			{
				node = node->children[i];
				found = 1;
			}

		if (!found && i < MAX_CHAR)
		{
			node->children[i] = newNode(*str);
			createNodesFromStr(node->children[i], str+1);
		}
	}
	return 1;	
}

int main()
{
	char text[256], substring[256];
	strcpy(text, "AGTCAGTAC");
	strcpy(substring, "AGTC");
	Node *root  = createRoot(text);
	checkForSubString(root, substring);
	printf("\n");
}