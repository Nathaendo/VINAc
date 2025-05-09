#include "lz.h"
#include "functions.h"
#include "aux.h"
#include "directory.h"
#include <time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h> 
#include <sys/types.h>

//Funções auxiliares

int find_member_index(struct directory *dir,const char *name){  //retorna -1 se não achar membro
    for(int i = 0; i < dir->member_qnt;i++){
        if (strcmp(dir->vector[i].nome, name) == 0){
            return i;
        }
    }
    return -1;
}

void shift_data(FILE *fp, struct directory *dir,int target, int ini, long int desloc){
    if(ini > target){ //shift para a direita
        for (int i = ini; i > target; i--) {

            size_t buffer_size = dir->vector[i].tam_d;
            char *buffer = malloc(buffer_size);
            if(buffer == NULL){
                printf("Erro alocação buffer (leitura) (shift - dir)\n");
                return;
            }
            if (fseek(fp, dir->vector[i].offset, SEEK_SET) != 0){
                printf("Erro no fseek (shift - dir)\n");
                free(buffer);
                return;
            }
            if (fread(buffer, 1, buffer_size, fp) != buffer_size) {
                printf("Erro na leitura do membro (shift - dir)\n");
                free(buffer);
                return;
            }


            if (fseek(fp, dir->vector[i].offset + desloc, SEEK_SET) != 0) {
                printf("Erro no fseek (escrita) (shift - dir)\n");
                free(buffer);
                return;
            }
            if (fwrite(buffer, 1, buffer_size, fp) != buffer_size) {
                printf("Erro na escrita do membro (shift - dir)\n");
                free(buffer);
                return;
            }
            free(buffer);
        }
    }
    else{  //Shift para a esquerda
        for (int i = ini; i <= target; i++) {
        size_t buffer_size = dir->vector[i].tam_d;
        char *buffer = malloc(buffer_size);
        if (buffer == NULL) {
            printf("Erro alocação buffer (leitura) (shift - esq)\n");
            return;
        }
        if (fseek(fp, dir->vector[i].offset, SEEK_SET) != 0) {
            printf("Erro no fseek leitura (shift - esq)\n");
            free(buffer);
            return;
        }
        if (fread(buffer, 1, buffer_size, fp) != buffer_size) {
            printf("Erro na leitura do membro (shift - esq)\n");
            free(buffer);
            return;
        }
        if (fseek(fp, dir->vector[i].offset - desloc, SEEK_SET) != 0) {
            printf("Erro no fseek (escrita) (shift - esq)\n");
            free(buffer);
            return;
        }
        if (fwrite(buffer, 1, buffer_size, fp) != buffer_size) {
            printf("Erro na escrita do membro (shift - esq)\n");
            free(buffer);
            return;
        }
        free(buffer);
        }
    }
}

FILE *open_archive(const char *archive_name, struct directory *dir,int create_if_missing){
    FILE *fp = fopen(archive_name,"rb+");

    if(fp == NULL){
        if(create_if_missing){
            fp = fopen(archive_name,"wb+");
            if(fp == NULL){
                printf("Erro ao criar archive\n");
                return NULL;
            }
        }
        else{
            printf("Archive não existe\n");
            return NULL;
        }
    }
    else{
        // Verifica se o arquivo tem conteúdo
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        rewind(fp); // Volta para o início se for continuar lendo
        if (size > 0) {
            if (read_directory(fp, dir) == 0) {
                printf("Erro ao ler diretório\n");
                return NULL;
            }
        }
    }
    return fp;
}

int write_member_to_archive(FILE *fp, const char *member_name, struct info_members_t *info_out) {
    // Abre o arquivo do membro
    FILE *mf = fopen(member_name, "rb");
    if (mf == NULL) {
        printf("Erro ao abrir membro %s.\n", member_name);
        return -1;
    }

    // Descobre o tamanho do arquivo
    fseek(mf, 0, SEEK_END);
    long size = ftell(mf);
    fseek(mf, 0, SEEK_SET);

    // Move o ponteiro do archive para o final
    fseek(fp, 0, SEEK_END);
    long offset = ftell(fp);

    // Aloca buffer e lê o conteúdo do membro
    char *buffer = malloc(size);
    if (buffer == NULL) {
        printf("Erro ao alocar memória para %s.\n", member_name);
        fclose(mf);
        return -1;
    }

    if (fread(buffer, 1, size, mf) != size) {
        printf("Erro ao ler o conteúdo do membro %s.\n", member_name);
        free(buffer);
        fclose(mf);
        return -1;
    }

    // Escreve o conteúdo no archive
    if (fwrite(buffer, 1, size, fp) != size) {
        printf("Erro ao escrever no archive o membro %s.\n", member_name);
        free(buffer);
        fclose(mf);
        return -1;
    }

    free(buffer);
    fclose(mf);

    // Preenche a estrutura info_members_t
    struct stat sta;
    if (stat(member_name, &sta) != 0) {
        printf("Erro ao obter informações do arquivo %s.\n", member_name);
        return -1;
    }
    memset(info_out, 0, sizeof(struct info_members_t));

    strcpy(info_out->nome, member_name);
    info_out->offset = offset;
    info_out->tam_d = size;
    info_out->tam_o = size;
    info_out->uid = getuid();
    info_out->mod = sta.st_mtime;

    return 0; // sucesso
}


int write_compressed_member_to_archive(FILE *fp, const char *member_name, struct info_members_t *info_out) {
    FILE *mf = fopen(member_name, "rb");
    if (!mf) {
        printf("Erro ao abrir membro %s.\n", member_name);
        return -1;
    }

    fseek(mf, 0, SEEK_END);
    long original_size = ftell(mf);
    fseek(mf, 0, SEEK_SET);

    if (original_size <= 0) {
        printf("Arquivo %s vazio ou erro ao obter tamanho.\n", member_name);
        fclose(mf);
        return -1;
    }

    unsigned char *original_data = malloc(original_size);
    if (!original_data) {
        printf("Erro ao alocar memória para leitura.\n");
        fclose(mf);
        return -1;
    }

    if (fread(original_data, 1, original_size, mf) != original_size) {
        printf("Erro ao ler conteúdo de %s.\n", member_name);
        free(original_data);
        fclose(mf);
        return -1;
    }
    fclose(mf);

    // Buffer de saída (pior caso: insize + margem pequena de segurança)
    unsigned char *compressed_data = malloc(original_size + 64);
    if (!compressed_data) {
        printf("Erro ao alocar memória para compressão.\n");
        free(original_data);
        return -1;
    }

    int compressed_size = LZ_Compress(original_data, compressed_data, original_size);
    free(original_data);

    if (compressed_size <= 0) {
        printf("Erro na compressão de %s.\n", member_name);
        free(compressed_data);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long offset = ftell(fp);

    if (fwrite(compressed_data, 1, compressed_size, fp) != (size_t)compressed_size) {
        printf("Erro ao escrever dados comprimidos no archive.\n");
        free(compressed_data);
        return -1;
    }
    free(compressed_data);

    // Preenche info do membro
    struct stat sta;
    if (stat(member_name, &sta) == -1) {
        printf("Erro ao obter metadados de %s.\n", member_name);
        return -1;
    }

    memset(info_out, 0, sizeof(struct info_members_t));

    strcpy(info_out->nome, member_name);
    info_out->offset = offset;
    info_out->tam_o = original_size;
    info_out->tam_d = compressed_size;
    info_out->uid = getuid();
    info_out->mod = sta.st_mtime;

    return 0;
}
