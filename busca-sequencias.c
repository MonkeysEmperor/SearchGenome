#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "mede_time.h"

#define N_SEQ_ARQ1 6
#define SIZE 100 
#define NOME_ARQ_SIZE 50
#define TRUE 1 
#define FALSE 0

int main(int argc, char *argv[])
{
    char buff[SIZE];
    char sequencia[SIZE];
    char arq1[NOME_ARQ_SIZE];
    char arq2[NOME_ARQ_SIZE];
    int ocorrencias[N_SEQ_ARQ1];
    int i, j, i_seq;
    int achou;
    FILE *f1;
    FILE *f2;
    int arq2_size;
    int i_seq_busca;

    TIMER_CLEAR;

    // Abrir o arquivo e pegar a palavra
    if (argc == 3){
    	f1 = fopen(argv[1], "r");
    	f2 = fopen(argv[2], "r");
    }

    else{
    	printf("Digite o nome do arquivo 1 (seqs a serem buscadas): ");
    	scanf("%s",&arq1);
    	printf("Digite o nome do arquivo 2 (em que as seqs serao buscadas): ");
    	scanf("%s",&arq2);
    	f1 = fopen(arq1, "r");
    	f2 = fopen(arq2, "r");
    }

    // Verificacao do nome do arquivo
    if(f1 == NULL){
    	printf("Arquivo 1 nao encontrado\n");
    	exit(1);
    }
    if(f2 == NULL){
    	printf("Arquivo 2 nao encontrado\n");
    	exit(1);
    }


    // Leitura do arquivo 1 linha por linha e busca pela sequencia lida
    // Leitura do arquivo 2 linha por linha ao buscar a sequencia lida do arquivo 1
    
    TIMER_START;

    printf("INICIO\n");
    
    fseek(f2,0L,SEEK_END);
    arq2_size = ftell(f2);
    fseek(f2,0L,SEEK_SET);

    i_seq = 0; 
    fgets(sequencia,SIZE,f1);
    
    while(!feof(f1)){
        ocorrencias[i_seq] = 0;
        achou = FALSE;
        i_seq_busca = 0;
    	
        while(i_seq_busca < arq2_size){
    		
            // Leitura de uma linha
    		fgets(buff, SIZE, f2);
	    	
            // Comparacao da sequencia na linha em arq2 com a seq buscada
	    	i_seq_busca += strlen(buff);
            
            if (strlen(sequencia) == strlen(buff)){
                achou=TRUE; 

                for(j = 0; j < strlen(sequencia); j++){
	    			
                    if(sequencia[j] != buff[j]){
                        achou = FALSE;
	    			    break;
	    			}
	    		}

	    		if (achou == TRUE)
                    ocorrencias[i_seq]++;
                
            } 

        }

        i_seq++;
        fgets(sequencia,SIZE,f1);
        fseek(f2, 0, SEEK_SET );
    }

    printf("=======================================\n");
    for(i = 0;i < N_SEQ_ARQ1; i++) 
        printf("Total de ocorrencias[%d] = %d\n",i,ocorrencias[i]);
    printf("=======================================\n");

    // Fecha os arquivos
    fclose(f1);
    fclose(f2);

    TIMER_STOP;
    printf("Tempo: %f \n", TIMER_ELAPSED);
    
    return 0;   
}