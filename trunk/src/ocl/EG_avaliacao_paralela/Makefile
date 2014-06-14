eg:
	g++  -o eg utils.h utils.c parser.h parser.c gramatica.h gramatica.c operadores_geneticos.c ag.c  eg_opencl.h eg_opencl.c eg.c main.c -lOpenCL -I ./include -I /usr/include -lm -Wno-pointer-arith -Wno-write-strings -D TAMANHO_POPULACAO=500 -D NUMERO_DE_GERACOES=50 -D DIMENSOES_PROBLEMA=128 -D TAMANHO_VALOR=8 -D TAXA_DE_MUTACAO=0.01 -D TAXA_DE_RECOMBINACAO=0.7 -D TAMANHO_TORNEIO=3 -D ELITE=1 -D PROFILING -D gpu 
clean:
	rm -f eg
all:
	clean 
	eg
