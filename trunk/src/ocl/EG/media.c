#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** arvgv){

	FILE *arq = fopen(arvgv[1], "r");

	char linha[20];

	float media = 0;
	int linhas = 0; 

	while(fgets(linha,20,arq) != NULL){
		media+=atof(linha);
		linhas++;
	}
	media/=(float)linhas;
	
	printf("%.2f",media);

}
