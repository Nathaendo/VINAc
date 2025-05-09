#include "lz.h"
#include "functions.h"
#include "directory.h"
#include "aux.h"
#include <sys/stat.h> 
#include <unistd.h> 

//Funções para a manipulação do diretório

struct directory *directory_init(){
    struct directory *dir = malloc(sizeof(struct directory));
    if(dir == NULL){
        printf("Erro ao iniciar diretório\n");
        return NULL;
    }
    dir->member_qnt = 0;
    dir->vector = NULL; //Inicializa como NULL, pois não tem elementos;
    dir->offset = 0;
    return dir;
}

void add_member(struct directory *dir,struct info_members_t *member){
    dir->vector = realloc(dir->vector, (dir->member_qnt + 1) *sizeof(struct info_members_t)); //Realoca memória para o vetor para adicionar um novo membro
    if(dir->vector == NULL){
        printf("Erro ao alocar memória no diretório\n");
        return; //retorna se a alocação falhar
    }
   
    member->ordem = dir->member_qnt;
    dir->vector[dir->member_qnt] = *member; //adiciona o novo membro (copia os dados)
    dir->member_qnt++;  //incrementa qnt de membros
}

void del_member(struct directory *dir, int index){ //remove pelo index (achado dentro da função de remover membro no disco);
    if(dir == NULL || index < 0 || index >= dir->member_qnt){
        printf("Diretório ou index inválido. (del_member)\n");
        return;
    }

    //Mover todos os membros após o índice para a posição anterior
    for(int i = index; i < dir->member_qnt - 1; i++){
        dir->vector[i] = dir->vector[i+1];
        dir->vector[i].ordem -= 1;
    }

    // Reduzir quantidade de membros;
    dir->member_qnt--;

     // Realocar memória para o vetor, se necessário;
    dir->vector = realloc(dir->vector, dir->member_qnt * sizeof(struct info_members_t));
    if (dir->vector == NULL && dir->member_qnt > 0) {
        printf("Erro ao realocar memória após remoção do membro. (del_member)\n");
        return;
    }
}

void move_member(struct directory *dir, int member_index, int target_index){
    struct info_members_t temp = dir->vector[member_index]; //auxiliar
    time_t current_time;
    time(&current_time);

    if(member_index < target_index){  //mover membros posteriores até o target para esquerda (membro indo para a direita)
        for(int i = member_index; i < target_index; i++){
            dir->vector[i] = dir->vector[i+1];
            dir->vector[i].offset = dir->vector[i+1].offset - temp.tam_d; //ajusta offset;
            dir->vector[i].ordem -= 1;
        }
        dir->vector[target_index] = temp;
        dir->vector[target_index].offset = dir->vector[target_index - 1].offset + dir->vector[target_index - 1].tam_d;
        dir->vector[target_index].mod = current_time;
        dir->vector[target_index].ordem = dir->vector[target_index-1].ordem + 1;
    }
    else if((member_index != target_index) && (member_index - target_index > 1)){   //mover membros anteriores para direita (membro indo para esquerda)
        for(int i = member_index; i > target_index + 1; i--){
            dir->vector[i] = dir->vector[i-1];
            dir->vector[i].offset = dir->vector[i-1].offset + temp.tam_d;
            dir->vector[i].ordem += 1;
        }
        dir->vector[target_index + 1] = temp;
        if(target_index == -1){
            dir->vector[target_index+1].offset = 0;
            dir->vector[target_index+1].ordem = 0;
        }
        else{
            dir->vector[target_index+1].offset = dir->vector[target_index].offset + dir->vector[target_index].tam_d;
            dir->vector[target_index+1].ordem = dir->vector[target_index].ordem + 1;
        }
        dir->vector[target_index+1].mod = current_time;
    }
}

void destroy_directory(struct directory *dir){
    if(dir == NULL){
        printf("Nada para ser destruído\n");
        return;
    }

    free(dir->vector);        //libera aloc do vetor;
    dir->vector = NULL;
    free(dir);                //libera aloc do diretorio;
    dir = NULL;
    return;     
}

void write_directory(FILE *fp, struct directory *dir){  //necessário remoção do antigo diretório antes, caso ele exista.
    if(fp == NULL || dir == NULL){
        printf("Arquivo ou diretório inválido. (write_directory)\n");
        return;
    }

    fseek(fp,0,SEEK_END);               //aponta para o final do arquivo.
    long int dir_offset = ftell(fp);    //obtém offset do diretório
    if (dir_offset == -1){
        printf("Erro ao obter o offset do arquivo. (write_directory)\n");
        return;
    }
    dir->offset = dir_offset;

    if(dir->member_qnt > 0){
        // Escreve o número de membros (member_qnt)
        if (fwrite(&dir->member_qnt, sizeof(int), 1, fp) != 1) {
            printf("Erro ao escrever o número de membros no arquivo. (write_directory)\n");
            return;
        }

        // Escreve todos os membros
        if (fwrite(dir->vector, sizeof(struct info_members_t), dir->member_qnt, fp) != dir->member_qnt) {
            printf("Erro ao escrever os membros no arquivo. (write_directory)\n");
            return;
        }

        // Escreve o offset do diretório no final do arquivo
        if (fwrite(&dir_offset, sizeof(long int), 1, fp) != 1) {
            printf("Erro ao escrever o offset do diretório no final do arquivo. (write_directory)\n");
            return;
        }

        if(fflush(fp) != 0){ //garante que o conteúdo foi gravado no disco.
            printf("Erro ao garantir escrita no disco. (write_directory)\n");
            return;
        } 
    }
}

void delete_directory(FILE *fp){
    if(fp == NULL){
        printf("Arquivo inválido. (delete_directory)\n");
        return;
    };

    fseek(fp,-sizeof(long int),SEEK_END);   //acha o endereço para ler o offset do diretório.

    long int dir_offset;
    if(fread(&dir_offset,sizeof(long int),1,fp) != 1){                              //lê o offset
        printf("Erro ao ler o offset do diretório. (delete_directory)\n");
        return;
    };

    if (ftruncate(fileno(fp), dir_offset) != 0) {
        printf("Erro ao truncar o arquivo. (delete_directory)\n");
        return;
    }
}

int read_directory(FILE *fp, struct directory *dir){
    if(fp == NULL || dir == NULL){
        printf("Arquivo ou diretório inválido. (read_directory)\n");
        return 0;
    }

    fseek(fp,-sizeof(long int),SEEK_END);                        //Acha o offset do diretório para leitura;
    long int dir_offset;
    if(fread(&dir_offset,sizeof(long int),1,fp) != 1){                          //lê o offset.
        printf("Erro ao ler o offset do diretório. (read_directory)\n");
        return 0;
    }

    fseek(fp, dir_offset,SEEK_SET); // Vai onde o diretório começa

    // Lê o número de membros
    if(fread(&dir->member_qnt,sizeof(int),1,fp) !=1){
        printf("Erro ao ler o número de membros. (read_directory)\n");
        return 0;
    }

    // Aloca memória para os membros
    dir->vector = malloc(dir->member_qnt * sizeof(struct info_members_t));
    if (dir->vector == NULL) {
        printf("Erro ao alocar memória para os membros. (read_directory)\n");
        return 0;
    }

     // Lê os membros
     if (fread(dir->vector, sizeof(struct info_members_t), dir->member_qnt, fp) != dir->member_qnt) {
        printf("Erro ao ler os membros do diretório. (read_directory)\n");
        return 0;
    }

    return dir->member_qnt; //Retorna
}