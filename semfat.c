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

int insere(char nome[9]){

	/*FALTA FAZER: Paridade do arquivo, vericicar nome válido, verificar retornos de FOPEN()*/

	FILE *fp = fopen(nome, "r");
	FILE *disco = fopen("lista.txt", "r+");

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

		fwrite(&prox, sizeof(int), 1, disco);//indica fim de arquivo temporariamente.
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
			fwrite(&prox, sizeof(int), 1, disco);//indica fim de arquivo temporariamente.
			
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
	printf("Removido com sucesso\n");

	return count;
}

void imprimeRelatorio(relatorio *r, int n){

	printf("=======================INSERÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 1){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração: %lf segundos\n", r[i].tempo);
			printf("Espaço ocupado pelo arquivo: %d blocos\n\n", r[i].espaco);
		}
	}

	printf("========================REMOÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 2){
				printf("Arquivo '%s':\n", r[i].nome);
				printf("Duração: %lf segundos\n", r[i].tempo);
				printf("Espaço ocupado pelo arquivo: %d blocos\n\n", r[i].espaco);
		}
	}	
	printf("=========================BUSCA=======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 3){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração: %lf segundos\n\n", r[i].tempo);
		}
	}	
	
	printf("===================BUSCA DE BLOCOS===================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 4){
			printf("Arquivo '%s':\n", r[i].nome);
			printf("Duração: %lf segundos\n\n", r[i].tempo);
		}
	}

	FILE *rel = fopen("relatorio.txt", "w+");


	fprintf(rel, "=======================INSERÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 1){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração: %lf segundos\n", r[i].tempo);
			fprintf(rel, "Espaço ocupado pelo arquivo: %d blocos\n\n", r[i].espaco);
		}
	}

	fprintf(rel, "========================REMOÇÃO======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 2){
				fprintf(rel, "Arquivo '%s':\n", r[i].nome);
				fprintf(rel, "Duração: %lf segundos\n", r[i].tempo);
				fprintf(rel, "Espaço ocupado pelo arquivo: %d blocos\n\n", r[i].espaco);
		}
	}	
	fprintf(rel, "=========================BUSCA=======================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 3){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração: %lf segundos\n\n", r[i].tempo);
		}
	}	
	
	fprintf(rel, "===================BUSCA DE BLOCOS===================\n");
	for(int i = 0; i < n; i++){
		if(r[i].operacao == 4){
			fprintf(rel, "Arquivo '%s':\n", r[i].nome);
			fprintf(rel, "Duração: %lf segundos\n\n", r[i].tempo);
		}
	}

	fclose(rel);		
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
		
		switch(op){
			
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
		//r = (relatorio *) realloc(r, sizeof(relatorio)*(i+3));
	}


	return 0;
}