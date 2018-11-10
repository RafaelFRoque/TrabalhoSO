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
	ListaArquivos *l;
	int fat[TAMDSC/TAMBLC];	
}FAT;


void inicaliza(){
	
	char c = 0;
	int tamarq = TAMDSC;
	
	FILE *disco = fopen("lista.txt", "w+");//disco = file lista
	FILE *tabelaFat = fopen("lfat.txt", "w+"); 
	
	fwrite(&c, sizeof(char), 1, tabelaFat);//indica a estabilidade do arquivo
	fwrite(&c, sizeof(char), 1, disco);
	fwrite(&tamarq, sizeof(int), 1, tabelaFat);//escreve a quantidade de espaço disponível
	fwrite(&tamarq, sizeof(int), 1, disco);
	
	for(int i=0; i<tamarq; i++){
		fwrite(&c, sizeof(char), 1, tabelaFat);//completa os 16384 bytes com valores invalidos
		fwrite(&c, sizeof(char), 1, disco);
	}
	
	fclose(disco);
	fclose(tabelaFat);
}



int main(int argc, char const *argv[]){
	
	//relatorio *r = (relatorio*) malloc(sizeof(relatorio));
	relatorio r[1000];
	int op;
	int bloco;
	int i = 0; 
	char nome[9];
	inicaliza();


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
		
		/*switch(op){
			
			case 1:
				fim = clock();
				r[i].espaco = insere(nome);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;
			break;

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


	return 0;
}