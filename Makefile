# Nome do compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -O2

# Nome do executável
TARGET = hamming

# Regra padrão
all: $(TARGET)

# Regra de compilação
$(TARGET): hamming.c
	$(CC) $(CFLAGS) -o $(TARGET) hamming.c

# Regra para limpeza dos arquivos gerados
clean:
	rm -f $(TARGET) *.o
