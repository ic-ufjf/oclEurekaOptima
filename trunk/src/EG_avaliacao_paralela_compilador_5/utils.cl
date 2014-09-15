
int binario_para_decimal(short binario[], int inicio, int fim){

    int i,n=1; int valorNumerico=0;

    for(i=fim-1; i>=inicio; i--, n=n<<1){
        valorNumerico += n*((int)binario[i]);
    }

    return valorNumerico;
}

void gray_para_binario(short gray[], short binarios[]){

        int i,j;

        for(i=0; i< TAMANHO_INDIVIDUO; i++){
            binarios[i] = gray[i];
        }

        int start;
        int end = 0;
        for (j = 0; j < DIMENSOES_PROBLEMA; j++) {
            start = end;
            end += TAMANHO_VALOR;
            for (i = start + 1; i < end; i++) {
                binarios[i] = binarios[i - 1] ^ binarios[i];
            }
        }
}

void obtem_fenotipo_individuo(individuo p, short fenotipo[]){
	
    int i, j=0;

    short genotipo_binario[TAMANHO_INDIVIDUO];

    gray_para_binario(p.genotipo, genotipo_binario);
   
    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){
       fenotipo[i] = binario_para_decimal(genotipo_binario, j, j+TAMANHO_VALOR);
    }
}
