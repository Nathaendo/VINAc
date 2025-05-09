# Defina o nome do executável
EXEC = vina

# Defina o compilador
CC = gcc

# Defina as flags de compilação
CFLAGS = -Wall -g

# Defina o diretório de origem
SRC_DIR = src

# Defina o diretório de objetos
OBJ_DIR = obj

# Defina os arquivos de origem
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Defina os arquivos de objeto
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Regras de compilação
all: $(EXEC)

# Como compilar o executável
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Como compilar os arquivos .c para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR) # Cria o diretório obj se ele não existir
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza de arquivos gerados
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

# Limpeza de objetos intermediários
mrproper: clean
	rm -f $(SRC_DIR)/*.bak $(SRC_DIR)/*.swp