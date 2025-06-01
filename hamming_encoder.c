#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 26
#define CODED_SIZE 31



int is_parity_position(int pos) {
    return pos == 1 || pos == 2 || pos == 4 || pos == 8 || pos == 16;
}



void hamming_31_26_decode(uint32_t *code, uint32_t *data){
    int s = 0;

    uint32_t copy_code = (*code);

    for (int i = 0; i < 5; i++)
    {                     // vai até a potência de 2⁴ = 16!
        int pi = 1 << i;  //1 2 4 8 16
        int count = 0;

        for (int j = pi; j < 32; j += 2 * pi)   //blocos de 2pi, pq alterna ligado a cada pi!
            for (int k = j; k < j + pi && k < 32; k++)  //vai de j ate j + pi - 1 no bloco (todas as possibilidades com o bit de interesse ligado!)
                count ^= (copy_code >> (31 - k)) & 1;   //paridade dos bits do bloco
                                                    //xor com o bit na posicao de interesse!
        if (count)  //se a qtd for impar
            s |= pi;    //liga o bit correspondente ao bloco no laco!!!
    }                   //com cada bloco correspondente ao valor binario, ao montar tudo, gera o indice especifico!

    if (s != 0 && s < 32){
        copy_code ^= 1u << (31 - s);   //corrige o erro na posicao de syndrome
        printf("Erro corrigido, bit na posição %d\n", s);
    }
    else
        printf("Nenhum erro ou erro excessivo detectado!\n");

    uint32_t copy_data = 0;
    int di = 0;
    for (uint32_t pos = 1; pos < 32; pos++){    
        if (!is_parity_position(pos)){   //se nao for potencia de 2
            uint32_t bit = (copy_code >> (31 - pos)) & 1;   //operacao para isolar 1 bit
            copy_data = (copy_data << 1) | bit; //seta os bits da msg original 1 de cada vez
            di++;   //atualiza o bit de referencia
        }
    }

    (*code) = copy_code;
    (*data) = copy_data;
}

uint32_t hamming_31_26_encode(uint32_t data) {
    int hamming[CODED_SIZE] = {0};
    int data_bit_index = 0;

    // Insere bits de dados nas posições que não são de paridade
    for (int i = 1; i <= CODED_SIZE; i++) {
        if (!is_parity_position(i)) {
            // Pega o bit correspondente dos 26 menos significativos
            int bit = (data >> (BLOCK_SIZE - 1 - data_bit_index)) & 1;
            hamming[i - 1] = bit;
            data_bit_index++;
        }
    }

    // Calcula os bits de paridade
    for (int p = 0; p < 5; p++) {
        int parity_pos = 1 << p; // 1, 2, 4, 8, 16
        int parity = 0;
        for (int i = 1; i <= CODED_SIZE; i++) {
            if ((i & parity_pos) && i != parity_pos) {
                parity ^= hamming[i - 1];
            }
        }
        hamming[parity_pos - 1] = parity;
    }

    // Constrói o valor codificado em um uint32_t
    uint32_t encoded = 0;
    for (int i = 0; i < CODED_SIZE; i++) {
        encoded <<= 1;
        encoded |= hamming[i];
    }

    return encoded;
}

void print_binary(uint32_t value, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
}

void print_encoded_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir arquivo codificado");
        exit(1);
    }

    uint32_t value;
    int count = 0;

    printf("Conteúdo do arquivo codificado:\n");

    while (fread(&value, sizeof(uint32_t), 1, file) == 1) {
        printf("[%d] Decimal: %u | Binário: ", count, value);
        print_binary(value, 31);
        printf("\n");
        count++;
    }

    fclose(file);
}
// Lê bytes do arquivo, agrupa em blocos de 26 bits, codifica e grava no novo arquivo
void encode_file(const char *input_filename, const char *output_filename) {

    FILE *arq_entrada = fopen(input_filename, "rb");
    if (!arq_entrada) {
        perror("Erro ao abrir arquivo de entrada");
        exit(1);
    }

    FILE *arq_saida = fopen(output_filename, "wb");
    if (!arq_saida) {
        perror("Erro ao criar arquivo de saída");
        fclose(arq_entrada);
        exit(1);
    }

    uint8_t buffer[4];                                                                   // Buffer para armazenar os bytes lidos
    size_t read;                                                                         // Número de bytes lidos    
    uint32_t bit_buffer = 0;                                                             // Buffer para armazenar os bits lidos
    int bit_count = 0;                                                                   // Contador de bits em bit_buffer

    while ((read = fread(buffer, 1, sizeof(buffer), arq_entrada)) > 0) {                 // Lê até 4 bytes do arquivo
        for (size_t i = 0; i < read; i++) {                                              // Para cada byte lido
            bit_buffer = (bit_buffer << 8) | buffer[i];                                  // Adiciona o byte ao buffer de bits
            bit_count += 8;                                                              // Incrementa o contador de bits                        

            while (bit_count >= 26) {
                uint32_t data26 = bit_buffer >> (bit_count - 26);                        // Extrai os 26 bits mais significativos

                printf("bit_count antes do encode: %d\n", bit_count);
                printf("data26: ");
                print_binary(data26, 26);
                printf("\n");

                uint32_t encoded = hamming_31_26_encode(data26);                       // Codifica os 26 bits

                printf("encoded: ");
                print_binary(encoded, 31);
                printf("\n");

                uint32_t data = 0;
                uint32_t copy = encoded;
                hamming_31_26_decode(&encoded, &data);
                printf("DECODIFICADO FICA:");
                print_binary(data, 26);
                printf("\n");

                encoded = copy;
                fwrite(&encoded, sizeof(uint32_t), 1, arq_saida);
                bit_count -= 26;
                bit_buffer &= (1 << bit_count) - 1;                                      // Limpa os bits processados   
            }
        }
    }

    if (bit_count > 0) {
        // Preenche os bits restantes à esquerda com zeros para formar 26 bits
        uint32_t data26 = bit_buffer << (26 - bit_count);

        printf("Final data26 (padded): ");
        print_binary(data26, 26);
        printf("\n");

        uint32_t encoded = hamming_31_26_encode(data26);
        fwrite(&encoded, sizeof(uint32_t), 1, arq_saida);

        printf("Final encoded: ");
        print_binary(encoded, 31);
        printf("\n");
    }

    fclose(arq_entrada);
    fclose(arq_saida);
    printf("Codificação concluída com sucesso!\n");
}


void decode_file (const char *input_filename, const char *output_filename){
    FILE *arq_entrada = fopen(input_filename, "rb");
    if (!arq_entrada) {
        perror("Erro ao abrir arquivo de entrada");
        exit(1);
    }

    FILE *arq_saida = fopen(output_filename, "w");
    if (!arq_saida) {
        perror("Erro ao criar arquivo de saída");
        fclose(arq_entrada);
        exit(1);
    }

    uint8_t buffer[4];                                                                   // Buffer para armazenar os bytes lidos
    size_t read;                                                                         // Número de bytes lidos    
    uint32_t bit_buffer = 0;                                                             // Buffer para armazenar os bits lidos
    int bit_count = 0;                                                                   // Contador de bits em bit_buffer

    uint32_t data;

    while ((read = fread(buffer, 1, sizeof(buffer), arq_entrada)) > 0) {                 // Lê até 4 bytes do arquivo
        for (size_t i = 0; i < read; i++) {                                              
            bit_buffer = (bit_buffer << 8) | buffer[i];                                  
            bit_count += 8;                                                                                      

            while (bit_count >= 31) {
                uint32_t data31 = bit_buffer >> (bit_count - 31);                        

                printf("bit_count antes do dencode: %d\n", bit_count);
                printf("data31: ");
                print_binary(data31, 31);
                printf("\n");

                hamming_31_26_decode(&data31, &data);                       

                printf("decoded: ");
                print_binary(data, 26);
                printf("\n");


                fprintf(arq_saida, "%u ", data);
                bit_count -= 31;
                bit_buffer &= (1 << bit_count) - 1;                                    
            }
        }
    }

    //VERIFICO SE SOBRA NESSE CASO?

    // if (bit_count > 0) {
    //     // Preenche os bits restantes à esquerda com zeros para formar 26 bits
    //     uint32_t data26 = bit_buffer << (26 - bit_count);

    //     printf("Final data26 (padded): ");
    //     print_binary(data26, 26);
    //     printf("\n");

    //     uint32_t encoded = hamming_31_26_encode(data26);
    //     fprintf(arq_saida, "%u ", encoded);

    //     printf("Final encoded: ");
    //     print_binary(encoded, 31);
    //     printf("\n");
    // }

    fclose(arq_entrada);
    fclose(arq_saida);
    printf("Decodificação concluída com sucesso!\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <-e|-d> <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-e") == 0) {
        encode_file(argv[2], argv[3]);
        print_encoded_file(argv[3]);
    } else if (strcmp(argv[1], "-d") == 0) {
        decode_file(argv[2], argv[3]);
    }
    return 0;
}

