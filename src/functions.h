#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include "directory.h"

// Operações principais do VINAc

void op_ip(FILE *fp,struct directory *dir, char **members, int member_count);    //Insere membro sem compressão

void op_ic(FILE *fp,struct directory *dir, char **members, int member_count);    //Insere membro com compressão

void op_m(FILE *fp,struct directory *dir, const char *member_name, const char *target_name);     //Move o membro indicado na linha de comando para diretamente após o target (para mover para o início o target tem que ser NULL)

void op_x(FILE *fp, struct directory *dir, char **members, int member_count);     //Extrai os membros indicados, se não indicado, extrai todos

void op_r(FILE *fp,struct directory *dir, char **members, int member_count);     //Remove os membros indicados

void op_c();     //lista o conteúdo do archive, incluindo as propriedades de cada membro

#endif //FUNCTIONS.H