eg:
	g++ -g -o eg utils.h utils.c parser.h parser.c gramatica.h gramatica.c eg_opencl.h eg_opencl.c main.c -lOpenCL -I ./include -I /usr/include -lm -Wno-pointer-arith -Wno-write-strings -D TAMANHO_POPULACAO=64 -D NUMERO_DE_GERACOES=1000 -DDIMENSOES_PROBLEMA=128 -D TAMANHO_VALOR=8 -D TAXA_DE_MUTACAO=0.05 -D TAXA_DE_RECOMBINACAO=0.9 -D TAMANHO_TORNEIO=3 -DELITE=1 -D PROFILING -D gpu 
clean:
	rm -f eg
all:
	clean
	eg
