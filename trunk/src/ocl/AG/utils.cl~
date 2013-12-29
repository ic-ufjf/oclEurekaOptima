#include <vsmc/opencl/urng.h>

/*
   Gera uma distribuição uniforme entre 0 e 1 (inclusive)
*/
float u_rand(cburng4x32 *rng){

    return u01_closed_closed_32_24(cburng4x32_rand(rng));
}


int rand(cburng4x32 *rng){
    return abs((int)cburng4x32_rand(rng));
}

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
