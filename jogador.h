#ifndef JOGADOR_H
#define JOGADOR_H

#include <stdio.h>
#include "fila.h" // Puxa a estrutura de Fila para os turnos

#define MAX_JOGADORES 4
#define MIN_JOGADORES 2

// Estrutura de dados do Jogador
typedef struct {
    int id;
    char nome[50];
    int posicao_tabuleiro;
} Jogador;

// Funcao que cadastra os jogadores e os coloca na fila
void criar_jogadores(tp_fila *fila_jogadores, Jogador jogadores[]) {
    int qtd;
    
    do {
        printf("Digite a quantidade de jogadores (%d a %d): ", MIN_JOGADORES, MAX_JOGADORES);
        scanf("%d", &qtd);
        if (qtd > MAX_JOGADORES || qtd < MIN_JOGADORES){
            printf("Quantidade invalida! Digite novamente.\n");}
    } while (qtd < MIN_JOGADORES || qtd > MAX_JOGADORES);

    inicializaFila(fila_jogadores);

    for (int i = 0; i < qtd; i++) {
        jogadores[i].id = i;
        printf("Nome do Jogador %d: ", i + 1);
        scanf(" %49[^\n]", jogadores[i].nome);
        jogadores[i].posicao_tabuleiro = 0; // Todos comecam na casa 0

        // Insere o ID do jogador na Fila
        insereFila(fila_jogadores, jogadores[i].id);
    }
    printf("-> %d Jogadores cadastrados e organizados na Fila!\n\n", qtd);
}

#endif
