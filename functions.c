#include "lz.h"
#include "functions.h"
#include <sys/stat.h> 
#include <unistd.h>    

//Operações do diretório e inicializações/liberação

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
    struct info_members_t *new_vector = realloc(dir->vector, (dir->member_qnt + 1) *sizeof(struct info_members_t)); //Realoca memória para o vetor para adicionar um novo membro
    if(new_vector == NULL){
        printf("Erro ao alocar memória no diretório\n");
        return; //retorna se a alocação falhar
    }
    dir->vector = new_vector;  //atualiza o vetor com a nova alocação

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

    if(member_index < target_index){  //mover membros posteriores até o target para esquerda (membro indo para a direita)
        for(int i = member_index; i < target_index; i++){
            dir->vector[i] = dir->vector[i+1];
        }
        dir->vector[target_index] = temp;
    }
    else{   //mover membros anteriores para direita (membro indo para esquerda)
        for(int i = member_index; i > target_index + 1; i--){
            dir->vector[i] = dir->vector[i-1];
        }
        dir->vector[target_index + 1] = temp;
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

    fflush(fp); //garante que o conteúdo foi gravado no disco.
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

void read_directory(FILE *fp, struct directory *dir){
    if(fp == NULL || dir == NULL){
        printf("Arquivo ou diretório inválido. (read_directory)\n");
        return;
    }

    fseek(fp,-sizeof(long int),SEEK_END);                        //Acha o offset do diretório para leitura;
    long int dir_offset;
    if(fread(&dir_offset,sizeof(long int),1,fp) != 1){                          //lê o offset.
        printf("Erro ao ler o offset do diretório. (read_directory)\n");
        return;
    }

    fseek(fp, dir_offset,SEEK_SET); // Vai onde o diretório começa

    // Lê o número de membros
    if(fread(&dir->member_qnt,sizeof(int),1,fp) !=1){
        printf("Erro ao ler o número de membros. (read_directory)\n");
        return;
    }

    // Aloca memória para os membros
    dir->vector = malloc(dir->member_qnt * sizeof(struct info_members_t));
    if (dir->vector == NULL) {
        printf("Erro ao alocar memória para os membros. (read_directory)\n");
        return;
    }

     // Lê os membros
     if (fread(dir->vector, sizeof(struct info_members_t), dir->member_qnt, fp) != dir->member_qnt) {
        printf("Erro ao ler os membros do diretório. (read_directory)\n");
        return;
    }

    printf("Diretório lido com sucesso! Número de membros: %d\n", dir->member_qnt); //Print para depuração
}

//Funções auxiliares

int find_member_index(struct directory *dir,const char *name){  //retorna -1 se não achar membro
    for(int i = 0; i < dir->member_qnt;i++){
        if (strcmp(dir->vector[i].nome, name) == 0){
            return i;
        }
    }
    return -1;
}

//Funções para operações dos membros no archive

void op_ip(const char *archive_name, char **members, int member_count){
    FILE *fp = fopen(archive_name, "r+b");
    struct directory *dir = directory_init();

    if(fp == NULL){     //Arquivo não existe, cria um novo
        fp = fopen(archive_name,"w+b");
        if(fp == NULL){
            printf("Erro ao criar novo arquivo.\n");
            return;
        }
    }else {
        read_directory(fp,dir); //copia o diretorio do archive na memória
        delete_directory(fp); //remove diretorio do archive
    }

    for(int i = 0; i < member_count; i++){
        const char *member_name = members[i];

        int index = find_member_index(dir, member_name);
        if(index != -1){
            //REMOVER MEMBRO DO ARCHIVE E DO DIRETÓRIO (chamar op_r)
        }

        FILE *mf = fopen(member_name,"rb");
        if(mf == NULL){
            printf("Erro ao abrir membro %s.\n", member_name);
            continue;
        }
        fseek(mf,0,SEEK_END); 
        long size = ftell(mf); //salva tamanho do arquivo
        fseek(mf,0,SEEK_SET);

        //onde será adiconado o arquivo
        fseek(fp,0,SEEK_END);
        long int offset = ftell(fp);

        //Aloca Buffer
        char *buffer = malloc(size);
        if(buffer == NULL){
            printf("Erro ao alocar memória para o buffer. (op_ip)\n");
            fclose(mf);
            continue;
        }

        // Lê o conteudo para o buffer;
        size_t bytes = fread(buffer,1,size,mf);
        if (bytes != size) {
            printf("Erro ao ler o conteúdo do membro %s.\n", member_name);
            free(buffer);
            fclose(mf);
            continue;
        }

        //Escreve o conteudo do buffer no archive
        if (fwrite(buffer, 1, bytes, fp) != bytes) {
            printf("Erro ao escrever no archive.\n");
            fclose(mf);
            free(buffer);
            continue;
        }
        fclose(mf);
        free(buffer);

        // Preenche info do membro
        struct info_members_t info;
        struct stat sta;
        if (stat(member_name, &sta) == 0) {  // Preenche a estrutura com informações do arquivo
            strcpy(info.nome, member_name);
            info.offset = offset;
            info.tam_d = size;
            info.tam_o = size;
            info.ordem = dir->member_qnt + 1;
            info.uid = getuid();
            info.mod = sta.st_mtime;  // Atribui o tempo de modificação do arquivo
        
            // Adiciona o membro ao diretório
            add_member(dir, &info);
        } else {
            printf("Erro ao obter informações do arquivo. (op_ip) %s.\n", member_name);
        }
    }
    write_directory(fp, dir);
    fclose(fp);
    destroy_directory(dir);
}



