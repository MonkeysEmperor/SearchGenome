#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mede_time.h"

#define N_SEQ_ARQ1 6
#define SIZE 100 
#define NOME_ARQ_SIZE 50


struct linkedList
{
	struct linkedList* next;
	char s[SIZE];
	int size;
};

typedef struct linkedList node;

node *createNode();
int *binarySearch(char*, node*, int*, int, int, int);
int findNextLineFeed(FILE*, int);

int main(int argc, char *argv[])
{
	char arq1[NOME_ARQ_SIZE],
		arq2[NOME_ARQ_SIZE];
	node *file1Lines, *aux, *next;
	int *ocorrencias;
	int i, arq2Size;
	FILE *f1, *f2;

	TIMER_CLEAR;

	// Abrir o arquivo e pegar a palavra
	if (argc == 3)
	{
		strcpy(arq1, argv[1]);
		strcpy(arq2, argv[2]);
	}
	else
	{
		printf("Digite o nome do arquivo 1 (seqs a serem buscadas): ");
		scanf("%s", arq1);
		printf("Digite o nome do arquivo 2 (em que as seqs serao buscadas): ");
		scanf("%s", arq2);
	}
	f1 = fopen(arq1, "r");
	f2 = fopen(arq2, "r");

	// Verificacao do nome do arquivo
	if(!f1 || !f2)
	{
		printf("Arquivo %d nao encontrado\n", !f1 ? 1 : 2);
		exit(1);
	}

	TIMER_START;

	printf("INICIO\n");
	
	fseek(f2, 0L, SEEK_END);
	arq2Size = ftell(f2);
	fclose(f2);
	
	file1Lines = createNode();
	fgets(file1Lines->s, SIZE, f1);
	file1Lines->size = strlen(file1Lines->s);
	for (next = file1Lines; !feof(f1); next = next->next)
	{
		aux = createNode();
		fgets(aux->s, SIZE, f1);
		aux->size = strlen(aux->s);
		next->next = aux;
	}
	fclose(f1);
   
	ocorrencias = binarySearch(arq2, file1Lines, calloc(N_SEQ_ARQ1, sizeof(int)), 0, arq2Size, 0);
	printf("%d", arq2Size);
	printf("=======================================\n");
	for(i = 0; i < N_SEQ_ARQ1; i++) 
		printf("Total de ocorrencias[%d] = %d\n", i, ocorrencias[i]);
	printf("=======================================\n");

	aux = file1Lines;
	while (aux)
	{
		file1Lines = aux;
		aux = aux->next;
		free(file1Lines);
	}

	TIMER_STOP;
	printf("Tempo: %f \n", TIMER_ELAPSED);
	
	return 0;   
}

node *createNode()
{
	node *n = (node*) malloc(sizeof(node));
	n->s[0] = '\0';
	n->size = 0;
	n->next = NULL;
	return n;
}

int *binarySearch(char *fileName, node *lines, int *matches, int start, int end, int depth)
{
	int *aux, *aux2, half, i, size;
	char s[SIZE];
	node *next;
	FILE *f;

	if (depth < 3 && end - start > SIZE)
	{
		half = start + (end - start)/2;
		aux = binarySearch(fileName, lines, matches, start, half, depth + 1);
		aux2 = binarySearch(fileName, lines, matches, half + 1, end, depth + 1);
		for (i = 0; i < N_SEQ_ARQ1; ++i)
			aux[i] += aux2[i];
	}
	else
	{
		aux = malloc(sizeof(int) * N_SEQ_ARQ1);
		memcpy(aux, matches, sizeof(int) * N_SEQ_ARQ1);
		f = fopen(fileName, "r");
		end = findNextLineFeed(f, end);
		start = findNextLineFeed(f, start);
		while (start < end && fgets(s, SIZE, f))
		{
			size = strlen(s);
			for (next = lines, i = 0; next; next = next->next, ++i)
			{
				if (next->size != size) continue;
				aux[i] += !strcmp(next->s, s);
			}
			start += size;
		}
	}

	return aux;
}

int findNextLineFeed(FILE *f, int position)
{
	char s[SIZE];
	fseek(f, (float)position, SEEK_SET);
	if (!position) return position;
	
	fgets(s, SIZE, f);
	return position + strlen(s); 
}
