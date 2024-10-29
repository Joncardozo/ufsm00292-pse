#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* número máximo de bytes do buffer de dados */
#define STX     0x02
#define ETX     0x03
#define MAX_BUFFER 32

/* macros de testes */
#define verifica(mensagem, teste) do { if (!(teste)) return mensagem; } while (0)
#define executa_teste(teste) do { char *mensagem = teste(); testes_executados++; \
                                if (mensagem) return mensagem; } while (0)

/* protótipo para threads */
#include "pt.h"

/* Variáveis globais */
static uint8_t buffer[MAX_BUFFER];
static uint8_t ACK = 0;
static uint8_t PKT_RXED = 0;
static uint8_t teste_erro = 0;

/* Contador de testes */
int testes_executados = 0;

/***********************************************
 * Funções de Comunicação e Teste
 ***********************************************/

void envia_pacote(void)
{
    uint8_t dados_sem_erro[] = { STX, 5, 11, 22, 33, 44, 55, 39, ETX };
    uint8_t dados_com_erro[] = { STX, 5, 11, 22, 33, 44, 66, 39, ETX };
    uint8_t *dados = dados_sem_erro;

    if (teste_erro) {
        dados = dados_com_erro;
        teste_erro = 0;
    }

    ACK = 0;
    memcpy(buffer, dados, sizeof(dados_sem_erro));
    PKT_RXED = 1;
}

uint8_t processa_pacote(uint8_t *dados_recebidos, uint8_t *buffer)
{
    uint8_t i = 0, chk = 0, cnt = 0;
    if (buffer[0] == STX) {
        cnt = buffer[1];
        for (i = 0; i < cnt; i++) {
            dados_recebidos[i] = buffer[2 + i];
            chk ^= dados_recebidos[i];
        }
        if (chk == buffer[2 + cnt] && buffer[2 + cnt + 1] == ETX) {
            return 1;
        }
    }
    return 0;
}

/***********************************************
 * Funções Protothread (sender e receiver)
 ***********************************************/

PT_THREAD(sender(struct pt *pt))
{
    PT_BEGIN(pt);

    do {
        envia_pacote();
        PT_WAIT_UNTIL(pt, ACK == 1 || PKT_RXED == 0);
    } while (ACK == 0);

    PT_END(pt);
}

PT_THREAD(receiver(struct pt *pt))
{
    static uint8_t resultado_esperado[] = {11, 22, 33, 44, 55};
    static uint8_t dados_recebidos[MAX_BUFFER];

    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, PKT_RXED == 1);
    ACK = processa_pacote(dados_recebidos, buffer);
    PKT_RXED = 0;

    if (ACK == 1) {
        if (memcmp(resultado_esperado, dados_recebidos, sizeof(resultado_esperado)) == 0) {
            printf("teste: PASSOU\n");
        } else {
            printf("teste: FALHOU\n");
        }
    }
    PT_END(pt);
}

/***********************************************
 * Funções de Teste
 ***********************************************/

static char *teste_pacote_correto(void)
{
    teste_erro = 0;
    envia_pacote();
    verifica("erro: pacote não foi enviado", PKT_RXED == 1);
    ACK = processa_pacote(buffer, buffer);
    verifica("erro: confirmação não recebida", ACK == 1);
    return 0;
}

static char *teste_pacote_com_erro(void)
{
    teste_erro = 1;
    envia_pacote();
    verifica("erro: pacote com erro foi enviado", PKT_RXED == 1);
    ACK = processa_pacote(buffer, buffer);
    verifica("erro: confirmação foi enviada incorretamente", ACK == 0);
    return 0;
}

static char *executa_testes(void)
{
    executa_teste(teste_pacote_correto);
    executa_teste(teste_pacote_com_erro);
    return 0;
}

/***********************************************
 * Função Principal
 ***********************************************/

int main()
{
    char *resultado = executa_testes();
    if (resultado != 0) {
        printf("%s\n", resultado);
    } else {
        printf("TODOS OS TESTES PASSARAM\n");
    }
    printf("Testes executados: %d\n", testes_executados);

    return resultado != 0;
}
