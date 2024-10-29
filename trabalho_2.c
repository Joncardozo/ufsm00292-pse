//
// Created by Jonathan on 13/10/24.
//

#include <stdio.h>
#include <stdlib.h>

/* macros de testes - baseado em minUnit: www.jera.com/techinfo/jtns/jtn002.html */
#define verifica(mensagem, teste) do { if (!(teste)) return mensagem; } while (0)
#define executa_teste(teste) do { char *mensagem = teste(); testes_executados++; \
if (mensagem) return mensagem; } while (0)
int testes_executados = 0;

static char * executa_testes(void);

typedef struct mensagem {
    u_int8_t STX;
    u_int8_t QTD_DADOS;
    u_int8_t * DADOS;
    u_int8_t CHK;
    u_int8_t ETX;
} Mensagem;

typedef enum {
    WAITING_MSG,
    GETTING_SIZE,
    READING_DATA,
    GETTING_CHK,
    END_RX_MSG
} rx_state;

typedef enum {
    START_TX,
    SEND_SIZE,
    SEND_DATA,
    SEND_CHK,
    END_TX_MSG
} tx_state;

rx_state receptor(const u_int8_t data, Mensagem * mensagem, rx_state state);
tx_state transmissor(u_int8_t dado, Mensagem * mensagem, tx_state state);
int recebe_msg(u_int8_t ** dados_recebidos, const int exemplo);
int envia_msg(u_int8_t * data, int n_dados);
u_int8_t calc_checksum(const u_int8_t * data, const u_int8_t size);
u_int8_t * mock_data(int example);


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

rx_state receptor(const u_int8_t data, Mensagem *mensagem, rx_state state) {

    if (mensagem == NULL)
        return WAITING_MSG;

    static int index = 0;

    switch (state) {
        case WAITING_MSG: {
            if (data == 0x02)
                state = GETTING_SIZE;
            return state;
        }
        case GETTING_SIZE: {
            const u_int8_t size = data;
            state = READING_DATA;
            mensagem->QTD_DADOS = size;
            mensagem->DADOS = malloc(size * sizeof(u_int8_t));
            return state;
        }
        case READING_DATA: {
            mensagem->DADOS[index] = data;
            index++;
            if (index == mensagem->QTD_DADOS)
                state = GETTING_CHK;
            return state;
        }
        case GETTING_CHK: {
            const u_int8_t chk = data;
            mensagem->CHK = chk;
            const u_int8_t chk_rx = calc_checksum(mensagem->DADOS, mensagem->QTD_DADOS);
            if (chk_rx != mensagem->CHK) {
                free(mensagem->DADOS);
                free(mensagem);
                return WAITING_MSG;
            }
            state = END_RX_MSG;
            return state;
        }
        case END_TX_MSG: {
            if (data == 0x03) {
                return WAITING_MSG;
            }
            else {
                free(mensagem->DADOS);
                free(mensagem);
                return WAITING_MSG;
            }
        }
    }
    return WAITING_MSG;
}

int recebe_msg(u_int8_t ** dados_recebidos, const int exemplo) {

    Mensagem * mensagem = malloc(sizeof(Mensagem));

    rx_state state = WAITING_MSG;

    const u_int8_t * data = mock_data(exemplo);
    const int n_dados = data[0];

    for (int i = 1; i <= n_dados; i++) {
        state = receptor(data[i], mensagem, state);
    }

    free((void*) data);

    if (mensagem != NULL && state == WAITING_MSG) {
        int msg_size = 0;
        msg_size = mensagem->QTD_DADOS;
        *dados_recebidos = mensagem->DADOS;
        free(mensagem);
        return msg_size;
    }
    return -1;
}

tx_state transmissor(u_int8_t dado, Mensagem * mensagem, tx_state state) {

    static int index = 0;
    static int n_dados = 0;

    switch (state) {
        case START_TX : {
            if (dado == 0x02) {
                mensagem->STX = 0x02;
                return SEND_SIZE;
            }
            else return START_TX;
        }
        case SEND_SIZE: {
            mensagem->QTD_DADOS = dado;
            n_dados = dado;
            return SEND_DATA;
        }
        case SEND_DATA: {
            if (index <  n_dados) {
                mensagem->DADOS[index++] = dado;
                return SEND_DATA;
            }
            return SEND_CHK;
        }
        case SEND_CHK: {
            mensagem->CHK = calc_checksum(mensagem->DADOS, mensagem->QTD_DADOS);
            return END_TX_MSG;
        }
        case END_TX_MSG: {
            mensagem->ETX = 0x03;
            return START_TX;
        }
    }
}

int envia_msg(u_int8_t * data, int n_dados) {

    Mensagem *mensagem = malloc(sizeof(Mensagem));
    mensagem->DADOS = malloc(data[2] * sizeof(u_int8_t));

    tx_state state = START_TX;
    for (int i = 0; i <= n_dados; i++) {
        state = transmissor(data[i + 1], mensagem, state);
    }

    free(mensagem->DADOS);
    free(mensagem);

    if (state == START_TX)
        return 0;
    return -1;
}

u_int8_t calc_checksum(const u_int8_t * data, const u_int8_t size) {
    u_int8_t sum = 0;
    for (u_int8_t i = 0; i < size; i++)
        sum += data[i];
    return sum;
}

u_int8_t * mock_data(const int example) {
    switch (example) {
        case 1 : {
            u_int8_t * data = malloc(7 * sizeof(u_int8_t));
            data[0] = 7;
            data[1] = 0x02;
            data[2] = 3;
            data[3] = 4;
            data[4] = 2;
            data[5] = 6;
            data[6] = 12;
            data[7] = 0x03;
            return data;
        }
        case 2 : {
            u_int8_t * data = malloc(7 * sizeof(u_int8_t));
            data[0] = 3;
            data[1] = 0x02;
            data[2] = 3;
            data[3] = 4;
            return data;
        }
        default: return NULL;
    }
}

/* Implementa funções de teste */

static char * teste_envia_mensagem(void) {
    u_int8_t * data = mock_data(1);
    int n_dados = data[0];
    const int status = envia_msg(data, n_dados);
    verifica("erro: nao enviado", status == 0);
    free(data);
    return NULL;
}

static char * teste_recebe_mensagem(void) {
    u_int8_t * dados = NULL;
    const int status = recebe_msg(&dados, 1);
    verifica("erro no receptor: nao recebido", status != -1);
    verifica("erro na mensagem recebida", dados[0] == 4 && dados[1] == 2 && dados[2] == 6);
    return NULL;
}

static char * teste_recebe_msg_incompleta(void) {
    u_int8_t * dados = NULL;
    const int status = recebe_msg(&dados, 2);
    verifica("erro no receptor: nao recebido", status == -1);
    return NULL;
}

static char * teste_recebe_msg_checksum_failed(void) {
    verifica("erro: nao implementado", 1==0);
}

static char * executa_testes(void)
{
    // executa_teste(teste_recebe_msg_checksum_failed);
    executa_teste(teste_envia_mensagem);
    executa_teste(teste_recebe_mensagem);
    executa_teste(teste_recebe_msg_incompleta);


    return 0;
}