#ifndef EG_OPENCL_H_INCLUDED
#define EG_OPENCL_H_INCLUDED

#include "representacao.h"
#include "parser.h"

void opencl_init(Database *dataBase);

void avaliacao_init(t_regra *gramatica, Database *dataBase);

void avaliacao_paralela(individuo * pop);

void opencl_dispose();

#endif // EG_OPENCL_H_INCLUDED
