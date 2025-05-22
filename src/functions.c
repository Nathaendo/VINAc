#include "lz.h"
#include "functions.h"
#include "aux.h"
#include <sys/stat.h> 
#include <unistd.h>    

//Funções para operações dos membros no archive

void op_ip(FILE *fp, struct directory *dir, char **members, int member_count) {
    for (int i = 0; i < member_count; i++) {
        char *member_name = members[i];

        int index = find_member_index(dir, member_name);
        int tgt_idx = -2;
        //verifica se o arquivo já existe.
        if (index != -1) {
            tgt_idx = index - 1;
            char *member_array[] = { member_name };
            op_r(fp, dir,member_array, 1);
        }

        // Escreve o novo membro no archive
        struct info_members_t info;
        if (write_member_to_archive(fp, member_name, &info) != 0) {
            printf("Falha ao inserir membro %s.\n", member_name);
            return;
        }

        // Define a ordem e adiciona ao diretório
        info.ordem = dir->member_qnt + 1;
        add_member(dir, &info);

        // Move para a posição correta, se necessário.
        if (tgt_idx >= 0 && dir->member_qnt > 1 && tgt_idx < dir->member_qnt - 2) {
            op_m(fp, dir, member_name, dir->vector[tgt_idx].nome);
        }
        if(tgt_idx == -1 && dir->member_qnt > 1){
            op_m(fp, dir, member_name, NULL);
        }
    }
}

void op_r(FILE *fp,struct directory *dir, char **members, int member_count){
  
    for(int i = 0; i < member_count; i++){
        const char *member_name = members[i];
        int mem_idx = find_member_index(dir, member_name);

        if(mem_idx == -1){
            printf("Membro não encontrado (op_r)\n");
            return;
        }
        int tgt_idx = dir->member_qnt - 1;
        
        //elemento a ser removido vira ultimo membro
        if(mem_idx != tgt_idx){
            op_m(fp,dir,member_name,dir->vector[tgt_idx].nome);
        }

        //acha onde tem que remover
        fseek(fp, dir->vector[tgt_idx].offset, SEEK_SET);
        int result = ftruncate(fileno(fp), ftell(fp));
        if(result == -1){
            printf("Erro ao truncar o arquivo. (op_r)\n");
        }

        del_member(dir, tgt_idx);
    }
}

void op_m(FILE *fp,struct directory *dir, const char *member_name, const char *target_name){
   
    int src_index = find_member_index(dir, member_name);   //acha index no diretorio do membro a ser movido
    if (src_index == -1) {
        printf("Membro não encontrado: %s\n", member_name);
        return;
    }

    int tgt_index = (target_name == NULL) ? -1 : find_member_index(dir, target_name);   //acha index do target no diretorio (NULL = -1)
    if (target_name != NULL && tgt_index == -1) {
        printf("Target não encontrado: %s\n", target_name);
        return;
    }

    struct info_members_t src = dir->vector[src_index]; //copia as infos do membro a ser movido


    long int buffer_size = src.tam_d;
    char *buffer = malloc(buffer_size); //buffer do tamanho do elemento que será movido
    if (buffer == NULL) {
        printf("Erro na alocação de memória (op_m)\n");
        return;
    }

    fseek(fp, src.offset, SEEK_SET);
    if (fread(buffer, 1, buffer_size, fp) != buffer_size) {  //leitura do membro;
        printf("Erro na leitura do membro original (op_m)\n");
        free(buffer);
        return;
    }
    

    if(src_index < tgt_index){ //move para a direita;
        shift_data(fp,dir,tgt_index,src_index + 1,dir->vector[src_index].tam_d);    
        move_member(dir,src_index,tgt_index);
    }
    else if(src_index > tgt_index){ //Move para a esquerda;
        shift_data(fp,dir,tgt_index,src_index - 1,dir->vector[src_index].tam_d);
        move_member(dir,src_index,tgt_index);
    }

    int final_index = (src_index < tgt_index) ? tgt_index : tgt_index + 1;
    fseek(fp, dir->vector[final_index].offset, SEEK_SET);

    if (fwrite(buffer, 1, buffer_size, fp) != buffer_size) {
        printf("Erro na escrita do membro (op_m)\n");
        free(buffer);
        return;
    }
    
    free(buffer);
}

void op_ic(FILE *fp, struct directory *dir, char **members, int member_count) {
    for (int i = 0; i < member_count; i++) {
        char *member_name = members[i];

        int index = find_member_index(dir, member_name);
        int tgt_idx = -2;
        if (index != -1) {
            tgt_idx = index - 1;
            char *member_array[] = { member_name };
            op_r(fp, dir,member_array, 1);
        }

        // Escreve o novo membro comprimido no archive
        struct info_members_t info;
        if (write_compressed_member_to_archive(fp, member_name, &info) != 0) {
            printf("Falha ao inserir membro comprimido %s.\n", member_name);
            return;
        }

        // Define a ordem e adiciona ao diretório
        info.ordem = dir->member_qnt + 1;
        add_member(dir, &info);

        // Move para a posição correta, se necessário.
        if (tgt_idx >= 0 && dir->member_qnt > 1 && tgt_idx < dir->member_qnt - 2) {
            op_m(fp, dir, member_name, dir->vector[tgt_idx].nome);
        }
        if(tgt_idx == -1 && dir->member_qnt > 1){
            op_m(fp, dir, member_name, NULL);
        }
    }
}

void op_x(FILE *fp, struct directory *dir, char **members, int member_count) {
    if (!dir || !fp) return;

    for (int i = 0; i < dir->member_qnt; i++) {
        struct info_members_t *m = &dir->vector[i];

        // Se membros foram especificados, verifica se este está na lista
        if (member_count > 0) {
            int found = 0;
            for (int j = 0; j < member_count; j++) {
                if (strcmp(m->nome, members[j]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) continue;
        }

        fseek(fp, m->offset, SEEK_SET);

        // Lê os dados armazenados no archive
        unsigned char *buffer = malloc(m->tam_d);
        if (!buffer) {
            fprintf(stderr, "Erro ao alocar memória para %s\n", m->nome);
            continue;
        }
        fread(buffer, 1, m->tam_d, fp);

        // Decide se descomprime
        unsigned char *output_data = buffer;
        int is_compressed = m->tam_o != m->tam_d;

        if (is_compressed) {
            output_data = malloc(m->tam_o);
            if (!output_data) {
                fprintf(stderr, "Erro ao alocar memória para descompressão de %s\n", m->nome);
                free(buffer);
                continue;
            }

            LZ_Uncompress(buffer, output_data, m->tam_d);
            free(buffer);
        }

        // Grava no disco
        FILE *out = fopen(m->nome, "wb");
        if (!out) {
            fprintf(stderr, "Erro ao criar arquivo %s\n", m->nome);
            if (is_compressed) free(output_data);
            continue;
        }

        fwrite(output_data, 1, m->tam_o, out);
        fclose(out);

        if (is_compressed) free(output_data);

        printf("Extraído: %s\n", m->nome);
        
        if(!is_compressed){
            free(buffer);        
        }
    }
}

void op_c(FILE *fp, struct directory *dir) {
    if (!dir || !fp) {
        printf("Diretório ou arquivo inválido.\n");
        return;
    }

    printf("Conteúdo do archive:\n");
    printf("------------------------------------------------------\n");

    for (int i = 0; i < dir->member_qnt; i++) {
        struct info_members_t *m = &dir->vector[i];

        // Exibe as propriedades de cada membro
        printf("Membro %d:\n", i + 1);
        printf("  Nome: %s\n", m->nome);
        printf("  Tamanho Original: %ld bytes\n", m->tam_o);
        printf("  Tamanho em Disco: %ld bytes\n", m->tam_d);
        printf("  UID: %u\n", m->uid);
        
        // Formatação da data de última modificação
        char mod_time_str[100];
        struct tm *tm_info = localtime(&m->mod);
        strftime(mod_time_str, sizeof(mod_time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        printf("  Última Modificação: %s\n", mod_time_str);

        printf("  Ordem: %d\n", m->ordem);
        printf("  Offset: %ld\n", m->offset);

        printf("------------------------------------------------------\n");
    }

    printf("Total de membros: %d\n", dir->member_qnt);
}



