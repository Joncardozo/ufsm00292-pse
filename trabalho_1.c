#include <stdio.h>
#include <stdlib.h>

/* macros de testes - baseado em minUnit: www.jera.com/techinfo/jtns/jtn002.html */
#define verifica(mensagem, teste) do { if (!(teste)) return mensagem; } while (0)
#define executa_teste(teste) do { char *mensagem = teste(); testes_executados++; \
if (mensagem) return mensagem; } while (0)
int testes_executados = 0;

static char * executa_testes(void);

typedef struct {
	int * buffer;
	int inicio;
	int fim;
	int tamanho_buffer;
	int buffer_cheio;
} Buffer;

Buffer * cria_buffer(int tamanho_buffer);
int escreve_buffer(Buffer * buffer, int numero);
int le_buffer(Buffer * buffer);
void apaga_buffer(Buffer * buffer);
void print_buffer(Buffer * buffer);

// int main()
// {
// 	const int tamanho_buffer = 10;
//
// 	Buffer * buffer = cria_buffer(tamanho_buffer);
// 	if (buffer == NULL) {
// 		printf("Erro ao criar buffer\n");
// 		return 1;
// 	}
//
// 	int input = 1;
// 	int numero;
// 	while (input) {
// 		printf("Digite uma opcao: (1) escreve no buffer; (2) le o buffer; (3) imprime buffer; (0) sair\n");
// 		scanf("%d", &input);
// 		switch (input) {
// 			case 0:
// 				apaga_buffer(buffer);
// 			break;
// 			case 1:
// 				printf("digite um numero: ");
// 				scanf("%d", &numero);
// 				escreve_buffer(buffer, numero);
// 				break;
// 			case 2:
// 				le_buffer(buffer);
// 				break;
// 			case 3:
// 				print_buffer(buffer);
// 				break;
// 			default:
// 				printf("Opcao invalida!");
// 		}
// 	}
//     return 0;
// }

int main()
{
	char *resultado = executa_testes();
	if (resultado != 0)
	{
		printf("%s\n", resultado);
	}
	else
	{
		printf("TODOS OS TESTES PASSARAM\n");
	}
	printf("Testes executados: %d\n", testes_executados);

	getchar();
	return resultado != 0;
}

/* Teste de exemplo - apenas para demonstração */
/***********************************************/
// static char * teste_retorna1_caso_recebe1(void)
// {
// 	verifica("erro: deveria retornar 1", retornax(1) == 1);
// 	return 0;
// }

static char * teste_cria_buffer(void) {
	Buffer * buffer = cria_buffer(10);
	verifica("erro: deveria retornar buffer", buffer != NULL);
	apaga_buffer(buffer);
	return 0;
}

static char * teste_insere(void) {
	Buffer * buffer = cria_buffer(5);
	const int ret = escreve_buffer(buffer, 10);
	verifica("erro: não foi possível inserir no buffer", ret == 0);
	apaga_buffer(buffer);
	return 0;
}

static char * teste_insere_final(void) {
	Buffer * buffer = cria_buffer(5);
	buffer->inicio = 4;
	buffer->fim = 0;
	buffer->tamanho_buffer = 5;
	buffer->buffer[4] = 1;
	escreve_buffer(buffer, 2);
	verifica("erro: não foi possível escrever ao final", buffer->buffer[0] == 2);
	apaga_buffer(buffer);
	return 0;
}

static char * teste_insere_buffer_cheio(void) {
	Buffer * buffer = cria_buffer(5);
	for (int i = 0; i < 5; i++)
		escreve_buffer(buffer, i);
	int ret = escreve_buffer(buffer, 10);
	verifica("erro: sobrescreveu valores no buffer", ret==1);
	apaga_buffer(buffer);
	return 0;
}

static char * teste_le_buffer(void) {
	Buffer * buffer = cria_buffer(5);
	for (int i = 1; i <= 5; i++)
		escreve_buffer(buffer, i);
	int ret = le_buffer(buffer);
	verifica("erro: não foi possível ler o buffer", ret == 1);
	apaga_buffer(buffer);
	return 0;
}

static char * teste_le_buffer_vazio(void) {
	Buffer * buffer = cria_buffer(5);
	int ret = le_buffer(buffer);
	verifica("erro: leu o buffer vazio", ret == -1);
	apaga_buffer(buffer);
	return 0;
}
/***********************************************/

static char * executa_testes(void)
{
	executa_teste(teste_cria_buffer);
	executa_teste(teste_insere_buffer_cheio);
	executa_teste(teste_insere_final);
	executa_teste(teste_insere);
	executa_teste(teste_le_buffer);
	executa_teste(teste_le_buffer_vazio);
	return 0;
}

Buffer * cria_buffer(const int tamanho_buffer)
{
	Buffer * buffer = malloc(sizeof(Buffer));
	buffer->buffer = malloc(sizeof(int) * tamanho_buffer);
	buffer->inicio = 0;
	buffer->fim = 0;
	buffer->tamanho_buffer = tamanho_buffer;
	buffer->buffer_cheio = 0;
	return buffer;
}


void apaga_buffer(Buffer * buffer) {
	if (buffer == NULL) {
		printf("Erro ao apagar buffer\n");
		return;
	}
	free(buffer->buffer);
	free(buffer);
}


int escreve_buffer(Buffer * buffer, const int numero) {
	if (buffer == NULL) {
		printf("Erro ao escrever buffer\n");
		return 1;
	}
	if (buffer->buffer_cheio) {
		puts("buffer cheio!\n");
		return 1;
	}
	const int end_escrita = buffer->fim;
	buffer->fim = (buffer->fim + 1) % buffer->tamanho_buffer;
	buffer->buffer[end_escrita] = numero;
	if (buffer->fim == buffer->inicio) {
		buffer->buffer_cheio = 1;
	}
	return 0;
}


int le_buffer(Buffer * buffer) {
	if (buffer == NULL) {
		printf("Erro ao ler o buffer\n");
		return -1;
	}
	if (!buffer->buffer_cheio && buffer->inicio == buffer->fim) {
		puts("buffer vazio!\n");
		return -1;
	}
	const int numero = buffer->buffer[buffer->inicio];
	buffer->inicio = (buffer->inicio + 1) % buffer->tamanho_buffer;
	buffer->buffer_cheio = 0;
	return numero;
}


void print_buffer(Buffer * buffer) {
	if (buffer == NULL) {
		printf("Erro ao imprimir buffer\n");
		return;
	}
	int inicio = buffer->inicio;
	const int fim = buffer->fim;
	if (!buffer->buffer_cheio && inicio == fim) {
		puts("buffer vazio!\n");
		return;
	}
	do {
		printf("%d\t", buffer->buffer[inicio]);
		inicio = (inicio + 1) % buffer->tamanho_buffer;
	} while (inicio != fim);
	puts("\n");
}