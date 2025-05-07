#ifndef FUNCTIONS.H
#define FUNCTIONS.H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "lz.h"

struct info_members_t{
    char nome[1024]; //Nome do arquivo
    uid_t uid;       //UID do arquivo
    long int tam_o;  //Tamanho original;
    long int tam_d;  //Tamanho em disco
    time_t  mod;     //Última modificação
    int ordem;       //Ordem do arquivo
    long int loc;    //Localização (Offset a partir do início)
};

struct directory{
    int member_qnt;                         //Quantidade de membros no archiver
    info_members_t *vector[member_qnt];     //Vetor com as infos dos membros
}

void op_ip();    //Insere membro sem compressão

void op_ic();    //Insere membro com compressão

void op_m();     //Move o membro indicado na linha de comando para diretamente após o target (para mover para o início o target tem que ser NULL)

void op_x();     //Extrai os membros indicados, se não indicado, extrai todos

void op_r();     //Remove os membros indicados

void op_c();     //lista o conteúdo do archive, incluindo as propriedades de cada membro

#endif //FUNCTIONS.H