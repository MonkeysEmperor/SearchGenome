#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
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
		*f2;
	int *ocorrencias;
	int i, fd, arq2Size;
	FILE *f1;
	struct stat sb;

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
	fd = open(arq2, O_RDONLY, S_IRUSR | S_IWUSR);

	// Verificacao do nome do arquivo
	if(!f1 || fstat(fd, &sb) == -1)
	{
		printf("Arquivo %d nao encontrado\n", !f1 ? 1 : 2);
		exit(1);
	}

	// Leitura do arquivo 1 linha por linha e busca pela sequencia lida
	// Leitura do arquivo 2 linha por linha ao buscar a sequencia lida do arquivo 1
	
	TIMER_START;

	printf("INICIO\n");
	
	f2 = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	// fseek(f2, 0L, SEEK_END);
	// arq2Size = ftell(f2);
	// fseek(f2, 0L, SEEK_SET);
	for(i = 0; !feof(f1); i++)
		fgets(file1Lines[i], SIZE, f1);

	fclose(f1);
   
	#pragma omp parallel num_threads(NTHREADS)
	{
		#pragma omp single
		ocorrencias = binarySearch(f2, file1Lines, calloc(N_SEQ_ARQ1, sizeof(int)), 0, sb.st_size, 0);
	}

	printf("=======================================\n");
	for(i = 0; i < N_SEQ_ARQ1; i++) 
		printf("Total de ocorrencias[%d] = %d\n", i, ocorrencias[i]);
	printf("=======================================\n");

	close(fd);

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
		half = (start + end)/2;
		if (depth < NTHREADS / 2)
		{
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
			aux = binarySearch(pointer, lines, matches, start, half, depth + 1);
			aux2 = binarySearch(pointer, lines, matches, half + 1, end, depth + 1);
			for (i = 0; i < N_SEQ_ARQ1; i++)
				aux[i] += aux2[i];
		}
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
				aux[i] += !strcmp(lines[i], s);
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
