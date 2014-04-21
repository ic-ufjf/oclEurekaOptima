#ifndef EG_OPENCL_H_INCLUDED
#define EG_OPENCL_H_INCLUDED

#include "representacao.h"
#include "parser.h"

//OpenCL includes
#include <CL/cl.h>

/*
  Etapa de inicialização do OpenCL:
  1)Descoberta e inicialização da(s) plataforma(s)
  2)Descoberta e inicialização do(s) dispositivo(s)
  3)Criação do contexto de execução
  4)Criação da fila de execução
*/
void openc_init();

void eg_paralela(individuo * pop, t_regra *gramatica, Database *dataBase, int pcores, int kernelAG);

void dispose();

#endif // EG_OPENCL_H_INCLUDED
