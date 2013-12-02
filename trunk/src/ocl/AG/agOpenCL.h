#ifndef AGOPENCL_H_INCLUDED
#define AGOPENCL_H_INCLUDED

#include "representacao.h"

//OpenCL includes
#include <CL/cl.h>

/*
  Etapa de inicialização do OpenCL:
  1)Descoberta e inicialização da(s) plataforma(s)
  2)Descoberta e inicialização do(s) dispositivo(s)
  3)Criação do contexto de execução
  4)Criação da fila de execução
*/
void initializeOpenCL();

/*
  Executa o kernel de avaliação
*/
void avaliacao(individuo * pop);

#endif // AGOPENCL_H_INCLUDED
