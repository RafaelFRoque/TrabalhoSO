#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAMBLC 32

void inicaliza(){
	char c = 0;
	int tamarq = 16384;
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

void insere(char nome[9]){

	FILE *fp = fopen(nome, "r");
	FILE *disco = fopen("lista.txt", "r+");

	fseek(fp, 0, SEEK_END);
	int tamanhofp = ftell(fp);	
	//printf("tamanhofp = %d\n", tamanhofp);

	fseek(disco, 1, SEEK_SET);
	int tamanhodisco;

	fread(&tamanhodisco, sizeof(int), 1, disco);
	//printf("tamanhodisco = %d\n", tamanhodisco);
	
	if(tamanhofp <= tamanhodisco){ //se houver espaço para a inserção
		
		char estaVazio;
		fread(&estaVazio, sizeof(char), 1, disco);
		
		while(estaVazio != 0){//procura o primeiro bloco que está vazio
			fseek(disco, TAMBLC-1, SEEK_CUR);
			printf("POSIÇÂO1 == %ld\n", ftell(disco)-5);
			fread(&estaVazio, sizeof(char), 1, disco);
		}

		char c = 1;
		int prox = -1; //fim do arquivo

		fseek(disco, -1, SEEK_CUR);
		fwrite(&c, sizeof(char), 1, disco);//indica que não esta removido e que é cabeçalho
		fwrite(&c, sizeof(char), 1, disco);//indica que não esta removido e que é cabeçalho
		//printf("achou cabeçalho na posição %ld\n", ftell(disco)-2);

		int ponteiroProx = ftell(disco);//salva a posição do disco para atualizar

		//printf("Salva a posição == %ld\n", ftell(fprox));

		fwrite(&prox, sizeof(int), 1, disco);//indica fim de arquivo temporariamente.
		fwrite(nome, sizeof(char), strlen(nome), disco);//salva o nome do arquivo
		char *conteudo = (char *) malloc(sizeof(char)*(TAMBLC-15));

		fseek(fp, 0, SEEK_SET); 
		fread(conteudo, sizeof(char)*(TAMBLC-15), 1, fp);//usa o valor restante do bloco para armazenar o arquivo
		//printf("leu e salvou %s\n", conteudo);
		fwrite(conteudo, sizeof(char)*(TAMBLC-15), 1, disco);
		
		tamanhofp -= TAMBLC-15; //atualiza do arquivo que falta escrever
		conteudo = (char *)realloc(conteudo, TAMBLC-6);
		
		//printf("disco == %ld\n", ftell(disco));

		
		while(tamanhofp > 0){
			printf("entrou aqui\n");

			fread(&estaVazio, sizeof(char), 1, disco);
			while(estaVazio != 0){//procura o proximo bloco estaVazio
				fseek(disco, TAMBLC-1, SEEK_CUR);
				printf("POSIÇÂO2 == %ld\n", ftell(disco)-5);
				fread(&estaVazio, sizeof(char), 1, disco);
			}
			fseek(disco, -1, SEEK_CUR);

			
			int atual = ftell(disco);
			prox =  (atual-5)/ TAMBLC;

			//printf("Escreve na posição == %ld\n", ftell(fprox));
			printf("prox == %d\n", prox);
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

			fread(conteudo, (tamanhofp < (TAMBLC-6))? sizeof(char)*tamanhofp : sizeof(char)* (TAMBLC-6), 1, fp);//usa o valor restante do bloco para armazenar o arquivo
			int a = fwrite(conteudo, sizeof(char)* (TAMBLC-6), 1, disco);
			printf("AAAAAAAAAA = %d\n", a);
			tamanhofp -= TAMBLC-6;			
			printf("disco == %ld\n", ftell(disco));
		}

		fread(&c,  sizeof(char), 1, disco);
		free(conteudo);
	}
}

int retornaRRN(FILE *disco, char nome[9]){
	int RRN;
	fseek(disco, 5, SEEK_SET);
	int tamanhodisco = 16384;	
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
		tamanhodisco-=32;
	}
	return -1;
}

void busca(FILE *disco, char nome[9]){
	int RRN = retornaRRN(disco, nome);
	fseek(disco, (RRN*TAMBLC)+7, SEEK_SET);

	int p;
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
		conteudo[TAMBLC-7]= '\0';
		printf("%s", conteudo);
	}
	printf("\n");
}



int main(int argc, char const *argv[]){
	int op;
	inicaliza();
	while(scanf("%d ", &op) != EOF){
		char nome[9];
		switch(op){
			case 1:
				scanf("%s", nome);
				insere(nome);
			break;
/*
			case 2:
				scanf("%s ", nome);
				remove(nome);
			break; */

			case 3:
				scanf("%s", nome);
				FILE *disco = fopen("lista.txt", "r");
				busca(disco, nome);
				//printf("%d",retornaRRN(disco, nome));
			break;
/*
			case 4:
				int bloco;
				scanf("%s %d", nome, bloco);
				buscaBloco(nome, bloco);
			break;

			case 5:
				imprimeRelatorio();
			break;*/
		}
	}


	return 0;
}
