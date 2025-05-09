#ifndef AUX_H
#define AUX_H

#include <time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include "directory.h"


int find_member_index(struct directory *dir,const char *name); //retorna -1 se não achar membro;

void shift_data(FILE *fp, struct directory *dir,int target, int ini, long int desloc); // Shift_data para mover membros no disco (1 por 1);

FILE *open_archive(const char *archive_name, struct directory *dir,int create_if_missing); //Abre o arquivo, cria se necessário, retorna o ponteiro para o arquivo aberto e o diretório por referência;

int write_member_to_archive(FILE *fp, const char *member_name, struct info_members_t *info_out);

int write_compressed_member_to_archive(FILE *fp, const char *member_name, struct info_members_t *info_out);

#endif //aux.h