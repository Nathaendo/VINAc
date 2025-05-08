#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>

struct info_members_t{
    char nome[1024]; //Nome do arquivo
    uid_t uid;       //UID do arquivo
    long int tam_o;  //Tamanho original;
    long int tam_d;  //Tamanho em disco
    time_t  mod;     //Última modificação
    int ordem;       //Ordem do arquivo
    long int offset;    //Localização (Offset a partir do início)
};

struct directory{
    int member_qnt;                        //Quantidade de membros no archiver
    struct info_members_t *vector;     //Vetor com as infos dos membros
    long int offset;
};


//Operações para o diretório

struct directory *directory_init();  //Inicia diretório

void destroy_directory(struct directory *dir);  //Destrói diretório

void add_member(struct directory *dir,struct info_members_t *member); //adiciona um mebro ao diretório

void del_member(struct directory *dir, int index); //remove o membro removido do archive do diretório.

void move_member(struct directory *dir, int member_index, int target_index); //Move o membro para depois do target (target = NULL, move para o ínicio);

void write_directory(FILE *fp, struct directory *dir); //Grava o diretório no final do arquivo.vc

void delete_directory(FILE *fp); //Deleta o diretório do arquivo.vc

void read_directory(FILE *fp,struct directory *dir); //Lê o diretório no final do arquivo

// Operações principais do VINAc

void op_ip(const char *archive_name, char **members, int member_count);    //Insere membro sem compressão

void op_ic();    //Insere membro com compressão

void op_m();     //Move o membro indicado na linha de comando para diretamente após o target (para mover para o início o target tem que ser NULL)

void op_x();     //Extrai os membros indicados, se não indicado, extrai todos

void op_r();     //Remove os membros indicados

void op_c();     //lista o conteúdo do archive, incluindo as propriedades de cada membro

#endif //FUNCTIONS.H