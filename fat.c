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
	FILE *discoFAT = fopen("lfat.txt", "w+"); 
	
	fwrite(&c, sizeof(char), 1, discoFAT);//indica a estabilidade do arquivo
	fwrite(&tamarq, sizeof(int), 1, discoFAT);//escreve a quantidade de espaço disponível
	
	for(int i=0; i<tamarq; i++)
		fwrite(&c, sizeof(char), 1, discoFAT);//completa os 16384 bytes com valores invalidos

	for(int i=0; i< TAMDSC/TAMBLC; i++)//coloca todos as posição da FAT como vazias
		fat->f[i] = -3;
	fat->tamLista = 0;
	fclose(discoFAT);
}

int insereFAT(FAT *fat, char nome[9]){

	FILE *fp = fopen(nome, "r");
	FILE *disco = fopen("lfat.txt", "r+");

	if(fp == NULL){
		printf("Arquivo de dados não encontrado\n");
		fclose(disco);
		return -1;
	}

	char integridade;
	fread(&integridade, sizeof(char), 1, disco);

	if(integridade != 0){
		printf("Disco corrompido\n");
		fclose(fp);
		fclose(disco);
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

	int tamanhoEmDisco = (int )ceil(((tamanhofp + 0.0)/TAMBLC))* TAMBLC;

	if(tamanhoEmDisco <= tamanhodisco){
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
				while(fat->f[aux] != -3)
					aux++;
				fat->f[i] = aux;
				i=aux;
			}
		}
		free(conteudo);
		tamanhodisco -= tamanhoEmDisco;
		fseek(disco, 1, SEEK_SET);
		fwrite(&tamanhodisco, sizeof(int), 1, disco);

	}
	else{
		printf("Não há espaço disponível\n");
		fseek(disco, 0, SEEK_SET);
		integridade = 0;
		fwrite(&integridade, sizeof(char), 1, disco);
		fclose(disco);
		fclose(fp);
		return -1;
	}

	fseek(disco, 0, SEEK_SET);
	integridade = 0;
	fwrite(&integridade, sizeof(char), 1, disco);

	fclose(disco);
	fclose(fp);
	printf("Inserido com sucesso com FAT\n");
	return tamanhoEmDisco/TAMBLC;

}

void buscaFAT(FAT *fat, char nome[9]){

    FILE *disco = fopen("lfat.txt", "r");
    char integridade;
    fread(&integridade, sizeof(char), 1, disco);
    if(integridade != 0){
		printf("Disco corrompido\n");
		fclose(disco);
		return;
	}
    int i = 0;
    int RRN;
    char * conteudo = (char * ) malloc(sizeof(char)*(TAMBLC+1));
    while(1){
        if((strcmp(fat->l[i].nome, nome))==0){
        	RRN = fat->l[i].inicio;

            do{
                fseek(disco, (RRN*TAMBLC)+5, SEEK_SET);
                fread(conteudo, sizeof(char)*(TAMBLC), 1, disco);
                conteudo[TAMBLC]= '\0';
                printf("%s", conteudo);
                RRN = fat->f[RRN];
            }while(RRN != -1);
            printf("\n");
            break;
        }
        else if(i >= fat->tamLista){
            printf("Arquivo não encontrado\n");
            break;
        }
        else {
            i++;
        }
    }
    free(conteudo);
    fclose(disco);
}

void buscaBlocoFAT(FAT *fat, char nome[9], int numeroBloco){
    
    FILE *disco = fopen("lfat.txt", "r");
    char integridade;
    fread(&integridade, sizeof(char), 1, disco);
 	if(integridade != 0){
		printf("Disco corrompido\n");
		fclose(disco);
		return;
	}
    
    int i = 0;
    int RRN = 0;
	char * conteudo = (char * ) malloc(sizeof(char)*(TAMBLC+1));
    while(1){
        if((strcmp(fat->l[i].nome, nome))==0){
            RRN = fat->l[i].inicio;

            for(int j = 0; j < numeroBloco; j++){
                RRN = fat->f[RRN];
                if(RRN == -1){
                	printf("Bloco invalido\n");
                	return;
                }
            }
            	fseek(disco, (RRN*TAMBLC)+5, SEEK_SET);
                fread(conteudo, sizeof(char)*(TAMBLC), 1, disco);
                conteudo[TAMBLC]= '\0';
                printf("%s\n", conteudo);
                break;               
        }
        else if(i >= fat->tamLista){
            printf("Arquivo não encontrado\n");
            break;
        }
        else {
            i++;
        }
    }
	free(conteudo);
    fclose(disco);
}
int removerFAT(FAT *fat, char nome[9]){

	FILE *disco = fopen("lfat.txt", "r+");
	char integridade;
	fread(&integridade, sizeof(char), 1, disco);

	if(integridade != 0){
		printf("Disco corrompido\n");
		fclose(disco);
		return -1;
	}

	fseek(disco, 0, SEEK_SET);
	integridade = 1;
	fwrite(&integridade, sizeof(char), 1, disco);
	int i=0;
	int RRN;
	int count=0;
	while(1){
        if((strcmp(fat->l[i].nome, nome))==0){
            RRN = fat->l[i].inicio;
            while(RRN != -1){
            	count++;
            	int aux = fat->f[RRN];
            	fat->f[RRN] = -3;
            	RRN = aux;
            }
            fat->l[i] = fat->l[fat->tamLista-1];
            strcpy(fat->l[fat->tamLista - 1].nome, "");
            fat->l[fat->tamLista - 1].inicio = -3;
            fat->tamLista-=1;
            break;
        }
        else if(i >= fat->tamLista){
            printf("Arquivo não encontrado\n");
            break;
        }
        else {
            i++;
        }
    }

    fseek(disco, 0, SEEK_SET);
	integridade = 0;
	fwrite(&integridade, sizeof(char), 1, disco);
    fclose(disco);
    printf("Removido com sucesso com FAT\n");
    return count;
}

void imprimeRelatorio(relatorio *r, int n){

	printf("=======================INSERÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 1){
			if(r[i].espaco != -1){	
				printf("Arquivo '%s':\n", r[i].nome);
				printf("Duração com FAT: %lf segundos\n", r[i].tempo);
				printf("Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco);
			}
		}
	}

	printf("========================REMOÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 2){
			if(r[i].espaco != -1){		
				printf("Arquivo '%s':\n", r[i].nome);
				printf("Duração com FAT: %lf segundos\n", r[i].tempo);
				printf("Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco);
			}
		}
	}	
	printf("=========================BUSCA=======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 3){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração com FAT: %lf segundos\n", r[i].tempo);
		}
	}	
	
	printf("===================BUSCA DE BLOCOS===================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 4){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração com FAT: %lf segundos\n", r[i].tempo);
		}
	}

	FILE *rel = fopen("RelatorioComFat.txt", "w+");


	fprintf(rel, "=======================INSERÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 1){
			if(r[i].espaco != -1){
				fprintf(rel, "Arquivo '%s':\n", r[i].nome);
				fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo);
				fprintf(rel, "Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco);
			}
		}
	}

	fprintf(rel, "========================REMOÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 2){
			if(r[i].espaco != -1){
				fprintf(rel, "Arquivo '%s':\n", r[i].nome);
				fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo);
				fprintf(rel, "Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco);
			}
		}
	}	
	fprintf(rel, "=========================BUSCA=======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 3){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo);
		}
	}	
	
	fprintf(rel, "===================BUSCA DE BLOCOS===================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 4){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo);
		}
	}
	fclose(rel);		
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

			case 2:
				fim = clock();
				r[i].espaco = removerFAT(&fat, nome);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;				
			break;

			case 3:
				fim = clock();
				buscaFAT(&fat, nome);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;
			break;

			case 4:
				scanf("%d", &bloco);
				fim = clock();
				buscaBlocoFAT(&fat, nome, bloco);
				fim = clock() - fim;
				r[i].tempo = (float)fim/CLOCKS_PER_SEC;
			break;

			case 5:
				imprimeRelatorio(r, i);			
			break;
		}
		i++;
	}
	return 0;
}