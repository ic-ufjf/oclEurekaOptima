/*
 * gramatica.h
 *
 *  Created on: 12/03/2014
 *      Author: igorsr
 */

#ifndef GRAMATICA_H_
#define GRAMATICA_H_

#include "parser.h"
#include <string.h>
#include <stdlib.h>

#define DELIMITADOR_REGRAS "::="
#define DELIMITADOR_ESCOLHAS "|"
#define DELIMITADOR_TERMINAIS " "
#define D_FENOTIPO 30

void LeGramatica(char nomeArquivo[], t_regra  * Gramatica);
void ProcessaEscolha(char * escolha, t_escolha * nova_escolha);

int Decodifica(t_regra * gramatica, int * fenotipo, t_item_programa * programa);

#endif /* GRAMATICA_H_ */
