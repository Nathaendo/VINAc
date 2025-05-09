<h1 align="center"><strong><font size="6">VINAc</font></strong></h1>

<h2 align="center"><font size="4">Nathan Endo - GRR: 20243480</font></h2>

# Estrutura do Diretório VINAc

1. **VINAc/**
    1. `README.md`
    2. `Makefile`
    3. `src/`
         1. `aux.c`
         2. `directory.c`
         3. `functions.c`
         4. `lz.c`
         5. `main.c`
         6. `lz.h`
         7. `functions.h`
         8. `directory.h`
         9. `aux.h`
   4. **obj/**
      1. `aux.o`
      2. `directory.o`
      3. `functions.o`
      4. `lz.o`
      5. `main.o`
   5. **bin/**
      1. `vina`


# Bibliotecas Criadas para o Projeto VINAc

## 1. **`aux.c`**

A biblioteca `aux.c` contém funções auxiliares que facilitam a manipulação de dados e operações de baixo nível. Ela inclui operações como leitura e escrita de dados no arquivo, controle de fluxo para verificação de erros, e manipulação de arquivos de maneira geral.

---

## 2. **`directory.c`**

A biblioteca `directory.c` é responsável pela manipulação do diretório do archive e manutenção dos metadados. Ela lida com operações de leitura e escrita de informações sobre os membros do archive, como o nome do arquivo, o tamanho, e o offset no arquivo de dados.

### Funções principais:
- `write_directory()`: Escreve o diretório com a informação dos membros e o offset atual.
- `add_member()`: Adiciona um novo membro ao diretório.
- `del_member()`: Remove um membro do diretório.
- `read_directory()`: Lê o diretório e carrega os membros do arquivo.

---

## 3. **`functions.c`**

A biblioteca `functions.c` contém a implementação das operações principais do programa, como a inserção e remoção de arquivos do archive, além de outras operações como compressão, movimentação e extração de arquivos. Ela interage diretamente com o diretório e com o arquivo de dados.

### Funções principais:
- `op_ip()`: Operação de inserção sem compressão de um arquivo no archive.
- `op_r()`: Operação de remoção de um arquivo do archive.
- `op_ic()`: Operação de inserção com compressão de um arquivo no archive.
- `op_x()`: Operação de extração de um arquivo do archive.

---

## 4. **`lz.c`** (Material Auxiliar fornecido pelos professores)

A biblioteca `lz.c` é responsável pela implementação do algoritmo de compressão LZ (Lempel-Ziv). Ela permite que os arquivos sejam comprimidos antes de serem inseridos no archive e descomprimidos quando extraídos.

### Funções principais:
- `LZ_compress()`: Função para comprimir um arquivo usando o algoritmo LZ.
- `LZ_Uncompress()`: Função para descomprimir um arquivo usando o algoritmo LZ.

---

## 5. **`main.c`**

O `main.c` é o ponto de entrada do programa. Ela contém a lógica principal para a interação do usuário e a execução das operações do programa com base nas opções passadas pela linha de comando.

### Função:
- `main()`: Função principal que processa os argumentos da linha de comando e chama as operações correspondentes (inserção, remoção, movimentação, etc.).

---

# Algoritmos Utilizados no Projeto VINAc

## 1. **Inserção de Arquivos no Archive**
 Os algoritmos de inserção (op_ip e op_ic), seguem a mesma lógica e possuem dois casos: 

- Caso o membro que será inserido já exista no Archive, ele removerá o membro e adicionará no final do Archive, e moverá o membro inserido para sua posição original. Assim como no Archive, ocorre o mesmo processo dos metadados do membro no diretório, que será primeiramente apagado, depois adicionado os novos metadados ao final e movidos para a posição correta. 

- Caso o membro não exista no Archive ele será adicionado ao final, e também é adicionado os metadados do membro no final do diretório.


## 2. **Remoção de Arquivos no Archive**

1. O arquivo é localizado no diretório e o offset é usado para encontrar sua posição no arquivo de dados.

2. O algoritmo de remoção segue uma lógica simples e possui dois casos:

- Caso o membro já esteja no final do Archive, ele apenas será removido do Archive e do Diretório.

- Caso ele não seja o último membro do Archive, ele será movido para a última posição do Archive, e do Diretório, assim sendo removido.

## 3. **Mover Arquivos no Archive**
1. O arquivo é localizado no diretório e o offset é usado para encontrar sua posição no arquivo de dados.

2. Possui dois casos:
- Movimentar para a esquerda, ou seja, o target está à esquerda do membro a ser movido, vale destacar que se target = NULL, moverá o membro para a primeira posição, será feito um shift membro a membro para abrir espaço logo após o target, chama a função para mover o membro no diretório tbm, ajustando os metadados de todos os membros que moverão.

- Movimentar para a direita, ou seja, o target está à direita do membro a ser movido, então será feito um shift até movimentar o target e abrir espaço para o membro ser movido, então chama a função para mover o membro no diretório tbm, ajustando os metadados de todos os membros que moverão.

Obs: existem dois Buffers no movimenta, um para guardar o valor que esta sendo movido, e outro para movimentar os membros um por um, a fim de abrir espaço para o membro ser movido.

## 4. **Extrai Arquivos no Archive**

1. O arquivo é localizado no diretório e o offset é usado para encontrar sua posição no arquivo de dados.
2. Se o arquivo estiver comprimido, ele será descomprimido usando o algoritmo LZ.
3. O arquivo extraído é então gravado no sistema de arquivos.

## 5. **Imprimir Metadados**
1. O diretório é lido do disco para a memória.
2. Os metadados dos membros são imprimidos em ordem, de acordo com o diretório gravado na memória.
## 6. **Funcionamento do Diretório**

1.  O Diretório sempre estará no final do Archive, e terá em seu final seu offset de ínicio.
2.  Após encontrar seu Offset será lido na memória, a fim de manipular os dados do diretório.
3. O Diretório é removido do Archive para não afetar o funcionamento das funções de Arquivos
4. Após o fim da operações de arquivos é gravado no fim do diretório.

- Obs: Há funções de diretório a fim de realizar a manutenção dos metadados enquanto as funções de arquivos são realizadas.
Ex: arrumar os Offsets/Ordem em uma movimentação, ou retirar um membro na exclusão.

# Dificuldades e soluções:

## Shift

- A principal dificuldade envolvendo o trabalho foi fazer uma função para movimentar os membros no archive, visto que é fundamental para a maioria das operações de membros.
- A solução foi utilizar dois Buffers no caso da operação de movimentar membros, que faz a chamada da função Shift (1 Buffer) e passa o deslocamento que todos os membros envolvidos sofrerão e guarda o membro que será movimentado (1 Buffer) após a realização do shift.
- Isso quer dizer que os dados são sobreescritos, porém os buffers garantem que nenhum dado será perdido.

## Nome com 1024B

- Como foi especificado no trabalho, o nome dos arquivos podem ter até 1024 B, ou seja, como a maior parte dos arquivos tem nome pequenos, acabava que muitos bits da string do nome não eram iniciados, apontando problemas no valgrind.
- A solução foi fazer um memset antes de colocar os metadados na struct, a fim de evitar lixo de memória e inicializar todos os bits da struct.