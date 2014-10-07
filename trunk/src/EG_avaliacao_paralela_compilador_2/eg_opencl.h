#ifndef EG_OPENCL_H_INCLUDED
#define EG_OPENCL_H_INCLUDED

#include "representacao.h"
#include "parser.h"
#include "gramatica.h"

void opencl_init(Database *dataBase);

void avaliacao_init(t_regra *gramatica, Database *dataBase);

void avaliacao_paralela(individuo *pop, t_prog * programas, int geracao, t_regra *gramatica);

void opencl_dispose();

#endif // EG_OPENCL_H_INCLUDED
