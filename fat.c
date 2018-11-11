#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define TAMBLC 32
#define TAMDSC 16384

typedef struct Relatorio{
	char nome[9];
	int operacao, espaco;
	double tempo;
}relatorio;

typedef struct {
	char nome[9];
	int inicio;
}ListaArquivos;

typedef struct {
	ListaArquivos l[100];
	int tamLista;
	int f[TAMDSC/TAMBLC];
	
}FAT;


void inicaliza(FAT *fat){
	
	char c = 0;
	int tamarq = TAMDSC;
	FILE *disco = fopen("lista.txt", "w+");//disco = file lista
	FILE *discoFAT = fopen("lfat.txt", "w+"); 
	
	fwrite(&c, sizeof(char), 1, discoFAT);//indica a estabilidade do arquivo
	fwrite(&c, sizeof(char), 1, disco);
	fwrite(&tamarq, sizeof(int), 1, discoFAT);//escreve a quantidade de espaço disponível
	fwrite(&tamarq, sizeof(int), 1, disco);
	
	for(int i=0; i<tamarq; i++){
		fwrite(&c, sizeof(char), 1, discoFAT);//completa os 16384 bytes com valores invalidos
		fwrite(&c, sizeof(char), 1, disco);
	}

	for(int i=0; i< TAMDSC/TAMBLC; i++)//coloca todos as posição da FAT como vazias
		fat->f[i] = -3;
	fat->tamLista = 0;
	fclose(disco);
	fclose(discoFAT);
}

int insereFAT(FAT *fat, char nome[9]){

	FILE *fp = fopen(nome, "r");
	FILE *disco = fopen("lfat.txt", "r+");

	char integridade;
	fread(&integridade, sizeof(char), 1, disco);

	if(integridade != 0){
		printf("Disco corrompido\n");
		return -1;
	}

	fseek(disco, 0, SEEK_SET);
	integridade = 1;
	fwrite(&integridade, sizeof(char), 1, disco);

	fseek(fp, 0, SEEK_END);
	int tamanhofp = ftell(fp);
	fseek(fp, 0, SEEK_SET);	

	int tamanhodisco;
	fread(&tamanhodisco, sizeof(int), 1, disco);

	if(tamanhofp <= tamanhodisco){
		int i=0;
		while(fat->f[i] != -3)
			i++;

		strcpy(fat->l[fat->tamLista].nome, nome);
		fat->l[fat->tamLista].inicio = i;
		fat->tamLista++;
		char *conteudo = (char *) malloc(sizeof(char)*(TAMBLC));

		while(tamanhofp > 0){

			fat->f[i]= -1;
			fread(conteudo, (tamanhofp < TAMBLC)? (tamanhofp*sizeof(char)) : (sizeof(char)*TAMBLC) , 1, fp);
			if(tamanhofp < TAMBLC)
				for(int i = tamanhofp; i < TAMBLC; i++)
					conteudo[i] = 0;


			fseek(disco, (i*TAMBLC)+5, SEEK_SET);
			fwrite(conteudo, sizeof(char)*TAMBLC, 1, disco);
			tamanhofp -= TAMBLC;

			if(tamanhofp > 0){
				int aux = i;
				while(fat->f[i] != -3)
					aux++;
				fat->f[i] = aux;
				i=aux;
			}
		}
		free(conteudo);

	}

}


int main(int argc, char const *argv[]){
	
	relatorio r[1000];
	int op;
	int bloco;
	int i = 0; 
	char nome[9];
	FAT fat;
	inicaliza(&fat);


	while(scanf("%d", &op) != EOF){
	
		if(op != 5){
			scanf("%s", nome);
			if(strlen(nome) != 9){
				printf("Nome invalido\n");
				break;
			}
		}

		clock_t fim;
		
		r[i].operacao = op;
		strcpy(r[i].nome, nome);
		
	switch(op){
			
			case 1:
				fim = clock();
				r[i].espaco = insereFAT(&fat, nome);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;
			break;
/*
			case 2:
				
				fim = clock();
				r[i].espaco = remover(nome);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;
				
			break;

			case 3:

				fim = clock();
				busca(nome);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;
		
			break;

			case 4:

				scanf("%d", &bloco);
				fim = clock();
				buscaBloco(nome, bloco);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;

			break;

			case 5:
				imprimeRelatorio(r, i);			
			break;
		}
		i++;
		//r = (relatorio *) realloc(r, sizeof(relatorio)*(i+3));*/
		}
	}


	return 0;
}