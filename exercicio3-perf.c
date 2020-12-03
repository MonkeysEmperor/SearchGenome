#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mede_time.h"
#include <omp.h>

#define N_SEQ_ARQ1 6
#define SIZE 100 
#define NOME_ARQ_SIZE 50
#define NTHREADS 4


int *binarySearch(char*, char [][SIZE], int*, int, int, int);
int findNextLineFeed(char*, int);
int getLine(char**, int, char*);

int main(int argc, char *argv[])
{
	char arq1[NOME_ARQ_SIZE],
		arq2[NOME_ARQ_SIZE],
		file1Lines[N_SEQ_ARQ1][SIZE],
		*file2Lines,
		buff[SIZE],
		*aux;
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

	// Leitura do arquivo 1 linha por linha e busca pela sequencia lida
	// Leitura do arquivo 2 linha por linha ao buscar a sequencia lida do arquivo 1
	
	TIMER_START;

	printf("INICIO\n");
	
	fseek(f2, 0L, SEEK_END);
	arq2Size = ftell(f2);
	fseek(f2, 0L, SEEK_SET);
	
	for(i = 0; !feof(f1); i++)
		fgets(file1Lines[i], SIZE, f1);
		
	file2Lines = (char*) malloc(SIZE * arq2Size * sizeof(char));
	
	for (aux = file2Lines; !feof(f2) && fgets(buff, SIZE, f2); aux = aux + i)
	{
		i = strlen(buff);
		memcpy(aux, buff, i);
		aux[i + 1] = ' ';
	}

	fclose(f1);
	fclose(f2);

	#pragma omp parallel num_threads(NTHREADS)
  	{ 
		#pragma omp single
		ocorrencias = binarySearch(file2Lines, file1Lines, calloc(N_SEQ_ARQ1, sizeof(int)), 0, arq2Size, 0);
	}
	printf("=======================================\n");
	for(i = 0; i < N_SEQ_ARQ1; i++) 
		printf("Total de ocorrencias[%d] = %d\n", i, ocorrencias[i]);
	printf("=======================================\n");

	free(file2Lines);

	TIMER_STOP;
	printf("Tempo: %f \n", TIMER_ELAPSED);
	
	return 0;   
}


int *binarySearch(char *pointer, char lines[][SIZE], int *matches, int start, int end, int depth)
{
	int *aux, *aux2, half, i, counter, size;
	char *s, *current;

	if (depth < 3 && end - start > SIZE)
	{
		half = start + (end - start)/2;
		#pragma omp task shared (aux)
			aux = binarySearch(pointer, lines, matches, start, half, depth + 1);
		#pragma omp task shared (aux2)
			aux2 = binarySearch(pointer, lines, matches, half + 1, end, depth + 1);
		#pragma omp taskwait
			for (i = 0; i < N_SEQ_ARQ1; i++)
				aux[i] += aux2[i];
	}
	else
	{
		aux = malloc(sizeof(int) * N_SEQ_ARQ1);
		memcpy(aux, matches, sizeof(int) * N_SEQ_ARQ1);
		start = findNextLineFeed(pointer, start);
		end = findNextLineFeed(pointer, end);
		current = (pointer + start);
		while (current - pointer < end)
		{
			size = getLine(&s, SIZE, current);
			for (i = 0; i < N_SEQ_ARQ1; i++)
				aux[i] += !memcmp(lines[i], s, size);
			current = (current + size);
		}
	}
	return aux;
}

int findNextLineFeed(char *f, int position)
{
	char s[SIZE], *aux;
	if (!position) return position;

	for (aux = f + position; *aux != '\n' && *aux != 0; aux++);

	return aux - f + (*aux != 0);
}

int getLine(char **s, int size, char *f)
{
	int i;
	char *aux = (char*) malloc(size * sizeof(char));
	for (i = 0; *f != '\n'; i++)
		aux[i] = *(f++);
	aux[i++] = '\n';
	aux[i] = '\0';
	*s = aux;
	return i;
}