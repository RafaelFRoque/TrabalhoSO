#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define TAMBLC 32
#define TAMDSC 16384

typedef struct Relatorio{
	char nome[9];
	int operacao, espaco1, espaco2;
	double tempo1, tempo2;
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

int insere(char nome[9]){

	/*FALTA FAZER: Paridade do arquivo, vericicar nome válido, verificar retornos de FOPEN()*/

	FILE *fp = fopen(nome, "r");
	FILE *disco = fopen("lista.txt", "r+");
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

	int tamanhodisco;
	fread(&tamanhodisco, sizeof(int), 1, disco);

	
	int tamanhoEmDisco = (int )ceil((tamanhofp + 9.0)/(TAMBLC - 6.0))* TAMBLC;

	if(tamanhoEmDisco <= tamanhodisco ){ //se houver espaço para a inserção
		
		char estaVazio;
		fread(&estaVazio, sizeof(char), 1, disco);
		
		while(estaVazio != 0){//procura o primeiro bloco que está vazio
			fseek(disco, TAMBLC-1, SEEK_CUR);
			fread(&estaVazio, sizeof(char), 1, disco);
		}

		char c = 1;
		int prox = -1; //fim do arquivo

		fseek(disco, -1, SEEK_CUR);
		fwrite(&c, sizeof(char), 1, disco);//indica que não esta removido e que é cabeçalho
		fwrite(&c, sizeof(char), 1, disco);//indica que não esta removido e que é cabeçalho
		
		int ponteiroProx = ftell(disco);//salva a posição do disco para atualizar

		fwrite(&prox, sizeof(int), 1, disco);//indica fim de arquivo tempo1rariamente.
		fwrite(nome, sizeof(char), strlen(nome), disco);//salva o nome do arquivo
		char *conteudo = (char *) malloc(sizeof(char)*(TAMBLC-15));

		fseek(fp, 0, SEEK_SET); 
		fread(conteudo, sizeof(char)*(TAMBLC-15), 1, fp);//usa o valor restante do bloco para armazenar o arquivo
		
		fwrite(conteudo, sizeof(char)*(TAMBLC-15), 1, disco);
		
		tamanhofp -= TAMBLC-15; //atualiza do arquivo que falta escrever
		conteudo = (char *)realloc(conteudo, TAMBLC-6);
		
		while(tamanhofp > 0){

			fread(&estaVazio, sizeof(char), 1, disco);
			while(estaVazio != 0){//procura o proximo bloco estaVazio
				fseek(disco, TAMBLC-1, SEEK_CUR);
				fread(&estaVazio, sizeof(char), 1, disco);
			}
			fseek(disco, -1, SEEK_CUR);
			
			int atual = ftell(disco);
			prox =  (atual-5)/ TAMBLC;
			
			fseek(disco, ponteiroProx, SEEK_SET);
			fwrite(&prox, sizeof(int), 1, disco);//indica fim de arquivo tempo1rariamente.
			
			fseek(disco, atual, SEEK_SET);
			c = 1;
			fwrite(&c, sizeof(char), 1, disco);//indica que não esta removido
			c = 0;
			fwrite(&c, sizeof(char), 1, disco);//indica que não é cabeçalho
			
			ponteiroProx = ftell(disco);//salva a posição do disco para atualizar
			prox = -1;
			fwrite(&prox, sizeof(int),  1, disco);

			//usa o valor restante do bloco para armazenar o arquivo
			fread(conteudo, (tamanhofp < (TAMBLC-6))? sizeof(char)*tamanhofp : sizeof(char)* (TAMBLC-6), 1, fp);
			
			if(tamanhofp < TAMBLC-6)
				for(int i = tamanhofp; i < (TAMBLC-6); i++)
					conteudo[i] = 0;

			fwrite(conteudo, sizeof(char)* (TAMBLC-6), 1, disco);
			tamanhofp -= TAMBLC-6;			
		}

		tamanhodisco -= tamanhoEmDisco;
		fseek(disco, 1, SEEK_SET);
		fwrite(&tamanhodisco, sizeof(int), 1, disco);
		
		free(conteudo);
	}
	else
		printf("Não há espaço disponível\n");
	
	fseek(disco, 0, SEEK_SET);
	integridade = 0;
	fwrite(&integridade, sizeof(char), 1, disco);

	fclose(disco);
	fclose(fp);

	printf("Inserido com sucesso sem FAT\n");
	return tamanhoEmDisco/TAMBLC;
}

int retornaRRN(FILE *disco, char nome[9]){
	
	int RRN;
	fseek(disco, 5, SEEK_SET);

	int tamanhodisco = TAMDSC;	
	
	while(tamanhodisco > 0){
		
		char removido;
		fread(&removido, sizeof(char), 1, disco);
		if(removido != 0){
			char cabecalho;
			fread(&cabecalho, sizeof(char), 1, disco);
			if(cabecalho == 1){
				char nomeArquivo[9];
				fseek(disco, 4, SEEK_CUR);
				fread(nomeArquivo, sizeof(char)*9, 1, disco);
				if(strcmp(nome, nomeArquivo) == 0){
					fseek(disco, -15, SEEK_CUR);
					RRN = (ftell(disco)-5)/TAMBLC;
					return RRN;
				}
				fseek(disco, TAMBLC-15, SEEK_CUR);
			}
			else
				fseek(disco, TAMBLC-2, SEEK_CUR);
		}
		else
			fseek(disco, TAMBLC-1, SEEK_CUR);
		
		tamanhodisco-=TAMBLC;
	}
	return -1;
}

void busca(char nome[9]){

	FILE *disco = fopen("lista.txt", "r");
	char integridade;
	fread(&integridade, sizeof(char), 1, disco);

	if(integridade != 0){
		printf("Disco corrompido\n");
		fclose(disco);
		return;
	}
	fseek(disco, 0, SEEK_SET);
	int RRN = retornaRRN(disco, nome);
	
	if(RRN == -1){
		printf("Arquivo não encontrado\n");
		return;
	}
	
	int p;
	fseek(disco, (RRN*TAMBLC)+7, SEEK_SET);
	fread(&p, sizeof(int), 1, disco);
	fseek(disco, 9, SEEK_CUR);

	char * conteudo = (char * ) malloc(sizeof(char)*(TAMBLC-14));
	fread(conteudo, sizeof(char)*(TAMBLC-15), 1 , disco);
	conteudo[TAMBLC-15]= '\0';
	printf("%s", conteudo);

	while(p != -1){
		conteudo = (char *) realloc(conteudo, TAMBLC-5);
		fseek(disco, (p*TAMBLC)+7, SEEK_SET);
		fread(&p, sizeof(int), 1, disco);
		fread(conteudo, sizeof(char)*(TAMBLC-6), 1, disco);
		conteudo[TAMBLC-6]= '\0';
		printf("%s", conteudo);
	}
	printf("\n");
	
	fclose(disco);
}

void buscaBloco(char nome[9], int numeroBloco){

	FILE *disco = fopen("lista.txt", "r");
	char integridade;
	fread(&integridade, sizeof(char), 1, disco);
	if(integridade != 0){
		printf("Disco corrompido\n");
		fclose(disco);
		return;
	}
	fseek(disco, 0, SEEK_SET);
	
	int RRN = retornaRRN(disco, nome);
	
	if(RRN == -1){
		printf("Arquivo não encontrado\n");
		return;
	}
	
	int p;
	fseek(disco, (RRN*TAMBLC)+7, SEEK_SET);
	fread(&p, sizeof(int), 1, disco);
	fseek(disco, 9, SEEK_CUR);

	char * conteudo = (char * ) malloc(sizeof(char)*(TAMBLC-5));
	if(numeroBloco == 0){
		fread(conteudo, sizeof(char)*(TAMBLC-15), 1 , disco);
		conteudo[TAMBLC-15]= '\0';
		printf("%s\n", conteudo);
	}
	else{
		int i = 0;
		while(p != -1){
			fseek(disco, (p*TAMBLC)+7, SEEK_SET);
			fread(&p, sizeof(int), 1, disco);
			fread(conteudo, sizeof(char)*(TAMBLC-6), 1, disco);
			conteudo[TAMBLC-6]= '\0';
			i++;
			if(i == numeroBloco)
				break;
		}

		if(i == numeroBloco)
			printf("%s\n", conteudo);
		else
			printf("Bloco não encontrado\n");
	}
	
	fclose(disco);
}

int remover(char nome[9]){

	FILE *disco = fopen("lista.txt", "r+");
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
	int RRN = retornaRRN(disco, nome);

	if(RRN == -1){
		printf("Arquivo não encontrado\n");
		return -1;
	}

	fseek(disco, (RRN*TAMBLC)+5, SEEK_SET);
	char a = 0;
	fwrite(&a, sizeof(char), 1, disco);
	fseek(disco, (RRN*TAMBLC)+7, SEEK_SET);
	fread(&RRN, sizeof(int), 1, disco);
	
	int count = 1;
	while(RRN != -1){
		fseek(disco, (RRN*TAMBLC)+5, SEEK_SET);
		fwrite(&a, sizeof(char), 1, disco);
		fseek(disco, (RRN*TAMBLC)+7, SEEK_SET);
		fread(&RRN, sizeof(int), 1, disco);
		count++;
	}

	fseek(disco, 1, SEEK_SET);
	int tamanhodisco;
	fread(&tamanhodisco, sizeof(int), 1, disco);

	tamanhodisco += count*TAMBLC; 
	fseek(disco, 1, SEEK_SET);
	fwrite(&tamanhodisco, sizeof(int), 1, disco);

	fseek(disco, 0, SEEK_SET);
	integridade = 0;
	fwrite(&integridade, sizeof(char), 1, disco);
	
	fclose(disco);
	printf("Removido com sucesso sem FAT\n");

	return count;
}

void imprimeRelatorio(relatorio *r, int n){

	printf("=======================INSERÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 1){
			if(r[i].espaco1 != -1 || r[i].espaco2 != -1){	
				printf("Arquivo '%s':\n", r[i].nome);
				printf("Duração com FAT: %lf segundos\n", r[i].tempo1);
				printf("Duração sem FAT: %lf segundos\n", r[i].tempo2);
				printf("Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco1);
				printf("Espaço ocupado pelo arquivo sem FAT: %d blocos\n\n", r[i].espaco2);
			}
		}
	}

	printf("========================REMOÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 2){
			if(r[i].espaco1 != -1 || r[i].espaco2 != -1){		
				printf("Arquivo '%s':\n", r[i].nome);
				printf("Duração com FAT: %lf segundos\n", r[i].tempo1);
				printf("Duração sem FAT: %lf segundos\n", r[i].tempo2);
				printf("Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco1);
				printf("Espaço ocupado pelo arquivo sem FAT: %d blocos\n\n", r[i].espaco2);
			}
		}
	}	
	printf("=========================BUSCA=======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 3){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração com FAT: %lf segundos\n", r[i].tempo1);
			printf("Duração sem FAT: %lf segundos\n", r[i].tempo2);
		}
	}	
	
	printf("===================BUSCA DE BLOCOS===================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 4){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração com FAT: %lf segundos\n", r[i].tempo1);
			printf("Duração sem FAT: %lf segundos\n", r[i].tempo2);
		}
	}

	FILE *rel = fopen("relatorio.txt", "w+");


	fprintf(rel, "=======================INSERÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 1){
			if(r[i].espaco1 != -1 || r[i].espaco2 != -1){
				fprintf(rel, "Arquivo '%s':\n", r[i].nome);
				fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo1);
				fprintf(rel, "Duração sem FAT: %lf segundos\n", r[i].tempo2);
				fprintf(rel, "Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco1);
				fprintf(rel, "Espaço ocupado pelo arquivo sem FAT: %d blocos\n\n", r[i].espaco2);
			}
		}
	}

	fprintf(rel, "========================REMOÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 2){
			if(r[i].espaco1 != -1 || r[i].espaco2 != -1){
				fprintf(rel, "Arquivo '%s':\n", r[i].nome);
				fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo1);
				fprintf(rel, "Duração sem FAT: %lf segundos\n", r[i].tempo2);
				fprintf(rel, "Espaço ocupado pelo arquivo com FAT: %d blocos\n\n", r[i].espaco1);
				fprintf(rel, "Espaço ocupado pelo arquivo sem FAT: %d blocos\n\n", r[i].espaco2);
			}
		}
	}	
	fprintf(rel, "=========================BUSCA=======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 3){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo1);
			fprintf(rel, "Duração sem FAT: %lf segundos\n", r[i].tempo2);
		}
	}	
	
	fprintf(rel, "===================BUSCA DE BLOCOS===================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 4){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração com FAT: %lf segundos\n", r[i].tempo1);
			fprintf(rel, "Duração sem FAT: %lf segundos\n", r[i].tempo2);
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
				r[i].espaco2 = insere(nome);
				fim = clock() - fim;
				r[i].tempo2 = (float)fim/CLOCKS_PER_SEC;
				fim = clock();
				r[i].espaco1 = insereFAT(&fat, nome);
				fim = clock() - fim;
				r[i].tempo1 = (float)fim/CLOCKS_PER_SEC;

			break;

			case 2:
				fim = clock();
				r[i].espaco2 = remover(nome);
				fim = clock() - fim;
				r[i].tempo2 = (float)fim/CLOCKS_PER_SEC;
				fim = clock();
				r[i].espaco1 = removerFAT(&fat, nome);
				fim = clock() - fim;
				r[i].tempo1 = (float)fim/CLOCKS_PER_SEC;
				
			break;


			case 3:
				fim = clock();
				busca(nome);
				fim = clock() - fim;
				r[i].tempo2 = (float)fim/CLOCKS_PER_SEC;
				fim = clock();
				buscaFAT(&fat, nome);
				fim = clock() - fim;
				r[i].tempo1 = (float)fim/CLOCKS_PER_SEC;
		
			break;

			case 4:

				scanf("%d", &bloco);
				fim = clock();
				buscaBloco(nome, bloco);
				fim = clock() - fim;
				r[i].tempo2 = (float)fim/CLOCKS_PER_SEC;
				fim = clock();
				buscaBlocoFAT(&fat, nome, bloco);
				fim = clock() - fim;
				r[i].tempo1 = (float)fim/CLOCKS_PER_SEC;


			break;

			case 5:
				imprimeRelatorio(r, i);			
			break;
		}
		i++;
	}


	return 0;
}