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
	int f[TAMDSC/TAMBLC];
}FAT;

void busca(FAT *fat, char nome[9]){

    FILE *disco = fopen("lfat.txt", "r");
    char integridade;
    fread(&integridade, sizeof(char), 1, disco);
    if(integridade != 0){
        printf("Disco corrompido\n");
        return;
    }
    int i = 0;
    int RRN;
    while(1){
        if((strcmp(fat->l[i].nome, nome))==0){
            RRN = fat->l[i].inicio;
        
            fseek(disco, (RRN*TAMBLC)+15, SEEK_SET);
            char * conteudo = (char * ) malloc(sizeof(char)*(TAMBLC-14));
            fread(conteudo, sizeof(char)*(TAMBLC-15), 1 , disco);
            conteudo[TAMBLC-15]= '\0';
            printf("%s", conteudo);
            RRN = fat->f[RRN];
            while(RRN != -1){
                conteudo = (char *) realloc(conteudo, TAMBLC-5);
                fseek(disco, (RRN*TAMBLC)+7, SEEK_SET);
                fread(&RRN, sizeof(int), 1, disco);
                fread(conteudo, sizeof(char)*(TAMBLC-6), 1, disco);
                conteudo[TAMBLC-6]= '\0';
                printf("%s", conteudo);
                RRN = fat->f[RRN];
            }
            printf("\n");
        }
        else if(i > (TAMDSC/TAMBLC)){
            printf("Arquivo não encontrado\n");
            break;
        }
        else {
            i++;
        }
    }

    
    
}

int main(int argc, char const *argv[]){
	
	//relatorio *r = (relatorio*) malloc(sizeof(relatorio));
	relatorio r[1000];
	int op;
	int bloco;
	int i = 0; 
	char nome[9];


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
    }
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
		}*/
		i++;
    //r = (relatorio *) realloc(r, sizeof(relatorio)*(i+3));
	return 0;
}
