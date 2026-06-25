#ifndef JOGADOR_H
#define JOGADOR_H

#include <stdio.h>
#include "fila.h"

#define MAX_JOGADORES 4
#define MIN_JOGADORES 2

/* Máximo de perguntas que podem ser registradas por jogador.
 * Coincide com QTD_PERGUNTAS de perguntas.h. */
#define MAX_HISTORICO_PERGUNTAS 18

/* ============================================================
 * Estrutura do Jogador
 *
 * CAMPOS NOVOS em relação à versão original:
 *   pontuacao           — pontuação acumulada no jogo
 *                         (+10/+20/+30 por acerto conforme nível)
 *   perguntas_acertadas — IDs das perguntas respondidas corretamente
 *   qtd_acertadas       — quantidade de acertos
 *   perguntas_erradas   — IDs das perguntas respondidas errado
 *   qtd_erradas         — quantidade de erros
 * ============================================================ */
typedef struct {
    int  id;
    char nome[50];
    int  posicao_tabuleiro;

    /* --- Placar e histórico de perguntas --- */
    int  pontuacao;
    int  perguntas_acertadas[MAX_HISTORICO_PERGUNTAS];
    int  qtd_acertadas;
    int  perguntas_erradas[MAX_HISTORICO_PERGUNTAS];
    int  qtd_erradas;
} Jogador;

/* ============================================================
 * criar_jogadores
 * Cadastra os jogadores, inicializa todos os campos e os
 * coloca na fila FIFO de turnos.
 * ============================================================ */
void criar_jogadores(tp_fila *fila_jogadores, Jogador jogadores[]) {
    int qtd;

    do {
        printf("Digite a quantidade de jogadores (%d a %d): ",
               MIN_JOGADORES, MAX_JOGADORES);
        scanf("%d", &qtd);
        if (qtd > MAX_JOGADORES || qtd < MIN_JOGADORES)
            printf("Quantidade invalida! Digite novamente.\n");
    } while (qtd < MIN_JOGADORES || qtd > MAX_JOGADORES);

    inicializaFila(fila_jogadores);

    for (int i = 0; i < qtd; i++) {
        jogadores[i].id                = i;
        printf("Nome do Jogador %d: ", i + 1);
        scanf(" %49[^\n]", jogadores[i].nome);
        jogadores[i].posicao_tabuleiro = 0;

        /* Inicializa campos de placar e histórico */
        jogadores[i].pontuacao     = 0;
        jogadores[i].qtd_acertadas = 0;
        jogadores[i].qtd_erradas   = 0;

        insereFila(fila_jogadores, jogadores[i].id);
    }

    printf("-> %d Jogadores cadastrados e organizados na Fila!\n\n", qtd);
}

#endif /* JOGADOR_H */
