#include "functions.h"
#include "lz.h"
#include "aux.h"
#include "directory.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: ./vinac <opção> <arquivo> [membro1 membro2 ...]\n");
        return 1;
    }

    const char *option = argv[1];
    const char *archive_name = argv[2];

    struct directory *dir = directory_init();

    //verifica se poderá ser necessário criar arquivo.
    int create_if_missing = (strcmp(option, "-ip") == 0 || strcmp(option, "-p") == 0 || strcmp(option, "-ic") == 0 || strcmp(option, "-i") == 0);

    FILE *fp = open_archive(archive_name, dir, create_if_missing);
    if (fp == NULL) {
        fprintf(stderr, "Erro ao abrir ou criar archive: %s\n", archive_name);
        destroy_directory(dir);
        return 1;
    }
    else if(dir->member_qnt > 0){
        delete_directory(fp); // recria o arquivo
    }

    if (strcmp(option, "-ip") == 0 || strcmp(option, "-p") == 0) {
        op_ip(fp, dir, &argv[3], argc - 3);
    } 
    else if (strcmp(option, "-ic") == 0 || strcmp(option, "-i") == 0) {
        op_ic(fp, dir, &argv[3], argc - 3);
    } 
    else if (strcmp(option, "-x") == 0) {
        op_x(fp, dir, &argv[3], argc - 3);
    } 
    else if (strcmp(option, "-r") == 0) {
        op_r(fp, dir, argv + 3, argc - 3);
    } 
    else if (strcmp(option, "-m") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Uso: ./vinac -m <arquivo> <origem> <destino>\n");
            write_directory(fp, dir);
            destroy_directory(dir);
            fclose(fp);
            return 1;
        }
        op_m(fp, dir, argv[3], argv[4]);
    } 
    else if (strcmp(option, "-c") == 0) {
        op_c(fp, dir);
    } 
    else {
        fprintf(stderr, "Opção desconhecida: %s\n", option);
    }

    write_directory(fp, dir);
    destroy_directory(dir);
    fclose(fp);
    return 0;
}