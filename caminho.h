/* ================================================================
 * caminho.h — Módulo do Tabuleiro (Caminho do Conhecimento)
 *
 * Representa as 45 casas do tabuleiro usando uma Lista Duplamente
 * Encadeada (listade.h). Cada nó da lista armazena o ID (int) de
 * uma casa; os dados completos ficam em banco_casas[].
 * Os ponteiros ->prox e ->ant da lista permitem navegar para
 * frente e para trás sem custo de busca linear após localizar o nó.
 *
 * Depende: listade.h, jogador.h, perguntas.h
 * ================================================================ */

#ifndef CAMINHO_H
#define CAMINHO_H

/* Número total de casas no tabuleiro */
#define TAMANHO_CAMINHO 45

#include "listade.h"
#include "jogador.h"
#include "perguntas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------
 * Rótulos dos tipos de casa
 * ---------------------------------------------------------------- */
#define TIPO_NORMAL    "Normal"
#define TIPO_SORTE     "Sorte"
#define TIPO_REVES     "Reves"
#define TIPO_PERGUNTA  "Pergunta"

/* ----------------------------------------------------------------
 * Parâmetros de bonificacao e penalidade
 * ---------------------------------------------------------------- */
#define CASAS_BONUS_SORTE      3  /* Avança ao cair em casa Sorte         */
#define CASAS_PENALIDADE_REVES 2  /* Volta ao cair em casa Reves          */
#define CASAS_BONUS_ACERTO     2  /* Avança ao acertar pergunta           */
#define CASAS_PENALIDADE_ERRO  1  /* Volta ao errar pergunta              */

/* ----------------------------------------------------------------
 * Divisão do caminho por Unidade temática
 * ---------------------------------------------------------------- */
#define LIMITE_U1 15  /* Casas  0–14  → perguntas Unidade 1 */
#define LIMITE_U2 30  /* Casas 15–29  → perguntas Unidade 2 */
                      /* Casas 30–44  → perguntas Unidade 3 */

/* ================================================================
 * Estrutura Casa
 * Contém o identificador e o tipo de cada casa do tabuleiro.
 * ================================================================ */
typedef struct {
    int  id_casa;
    char tipo_casa[20];
} Casa;

/* O Caminho é um alias para a lista duplamente encadeada */
typedef tp_listad Caminho;

/* ================================================================
 * criar_caminho
 * Inicializa o banco de casas e a lista duplamente encadeada.
 * Cada nó da lista armazena o ID (int) da casa correspondente.
 *
 *   banco[] — array de saída com os dados de todas as casas
 *   retorna — ponteiro para o Caminho (lista encadeada) alocado
 * ================================================================ */
Caminho *criar_caminho(Casa banco[]) {

    /* Layout fixo das 45 casas — editável conforme o design do jogo */
    const char *tipos[TAMANHO_CAMINHO] = {
        /* 0  */ TIPO_NORMAL,    /* Início                        */
        /* 1  */ TIPO_NORMAL,
        /* 2  */ TIPO_PERGUNTA,
        /* 3  */ TIPO_SORTE,
        /* 4  */ TIPO_NORMAL,
        /* 5  */ TIPO_REVES,
        /* 6  */ TIPO_PERGUNTA,
        /* 7  */ TIPO_NORMAL,
        /* 8  */ TIPO_SORTE,
        /* 9  */ TIPO_PERGUNTA,
        /* 10 */ TIPO_NORMAL,
        /* 11 */ TIPO_REVES,
        /* 12 */ TIPO_PERGUNTA,
        /* 13 */ TIPO_NORMAL,
        /* 14 */ TIPO_PERGUNTA, /* Última casa da Unidade 1      */
        /* 15 */ TIPO_NORMAL,
        /* 16 */ TIPO_SORTE,
        /* 17 */ TIPO_PERGUNTA,
        /* 18 */ TIPO_NORMAL,
        /* 19 */ TIPO_REVES,
        /* 20 */ TIPO_PERGUNTA,
        /* 21 */ TIPO_NORMAL,
        /* 22 */ TIPO_SORTE,
        /* 23 */ TIPO_PERGUNTA,
        /* 24 */ TIPO_REVES,
        /* 25 */ TIPO_NORMAL,
        /* 26 */ TIPO_PERGUNTA,
        /* 27 */ TIPO_NORMAL,
        /* 28 */ TIPO_SORTE,
        /* 29 */ TIPO_PERGUNTA, /* Última casa da Unidade 2      */
        /* 30 */ TIPO_NORMAL,
        /* 31 */ TIPO_REVES,
        /* 32 */ TIPO_PERGUNTA,
        /* 33 */ TIPO_NORMAL,
        /* 34 */ TIPO_SORTE,
        /* 35 */ TIPO_PERGUNTA,
        /* 36 */ TIPO_NORMAL,
        /* 37 */ TIPO_REVES,
        /* 38 */ TIPO_PERGUNTA,
        /* 39 */ TIPO_NORMAL,
        /* 40 */ TIPO_SORTE,
        /* 41 */ TIPO_PERGUNTA,
        /* 42 */ TIPO_NORMAL,
        /* 43 */ TIPO_REVES,
        /* 44 */ TIPO_NORMAL    /* Chegada (casa final)          */
    };

    Caminho *caminho = inicializa_listad();

    for (int i = 0; i < TAMANHO_CAMINHO; i++) {
        banco[i].id_casa = i;
        strncpy(banco[i].tipo_casa, tipos[i], sizeof(banco[i].tipo_casa) - 1);
        banco[i].tipo_casa[sizeof(banco[i].tipo_casa) - 1] = '\0';

        /* Insere o ID da casa no fim da lista encadeada */
        insere_listad_no_fim(caminho, i);
    }

    printf("-> Caminho criado: %d casas inseridas na Lista Duplamente Encadeada!\n\n",
           TAMANHO_CAMINHO);
    return caminho;
}

/* ================================================================
 * obter_no_por_id
 * Percorre a lista e retorna o nó cujo campo info == id_casa.
 * Retorna NULL se o ID não for encontrado.
 * ================================================================ */
tp_no *obter_no_por_id(Caminho *caminho, int id_casa) {
    tp_no *atu = caminho->ini;
    while (atu != NULL) {
        if (atu->info == id_casa) return atu;
        atu = atu->prox;
    }
    return NULL;
}

/* ================================================================
 * mover_na_lista
 * Navega pela lista duplamente encadeada a partir do nó atual.
 *   passos > 0 → avança usando ->prox
 *   passos < 0 → recua  usando ->ant
 * Respeita os limites (casa 0 e casa 44).
 * Retorna o ID da nova casa.
 * ================================================================ */
int mover_na_lista(Caminho *caminho, int pos_atual, int passos) {
    tp_no *no = obter_no_por_id(caminho, pos_atual);
    if (no == NULL) return pos_atual; /* Segurança: posição inválida */

    if (passos > 0) {
        /* Avança: percorre ->prox até esgotar passos ou chegar ao fim */
        for (int i = 0; i < passos; i++) {
            if (no->prox == NULL) break;
            no = no->prox;
        }
    } else {
        /* Recua: percorre ->ant até esgotar passos ou chegar ao início */
        for (int i = 0; i > passos; i--) {
            if (no->ant == NULL) break;
            no = no->ant;
        }
    }
    return no->info;
}

/* ================================================================
 * rolar_dado
 * Retorna um número aleatório entre 1 e 6 (requer srand anterior).
 * ================================================================ */
int rolar_dado() {
    return (rand() % 6) + 1;
}

/* ================================================================
 * selecionar_pilha_por_posicao
 * Escolhe qual pilha de perguntas usar conforme a posição do jogador.
 *   0–14  → pilha Unidade 1
 *   15–29 → pilha Unidade 2
 *   30–44 → pilha Unidade 3
 * ================================================================ */
tp_pilha *selecionar_pilha_por_posicao(int pos,
                                        tp_pilha *pu1,
                                        tp_pilha *pu2,
                                        tp_pilha *pu3) {
    if (pos < LIMITE_U1) return pu1;
    if (pos < LIMITE_U2) return pu2;
    return pu3;
}

/* ================================================================
 * aplicar_efeito_casa
 * Aplica o efeito da casa onde o jogador parou:
 *   Normal   → sem ação
 *   Sorte    → avança CASAS_BONUS_SORTE casas (via ->prox na lista)
 *   Reves    → volta CASAS_PENALIDADE_REVES casas (via ->ant na lista)
 *   Pergunta → retira do topo da pilha (pop), exibe e verifica resposta
 *              acerto → avança CASAS_BONUS_ACERTO
 *              erro   → volta CASAS_PENALIDADE_ERRO
 *
 * Atualiza jogador->posicao_tabuleiro e retorna a nova posição.
 * ================================================================ */
int aplicar_efeito_casa(Jogador *jogador, Caminho *caminho,
                         Casa banco_c[], Pergunta banco_p[],
                         tp_pilha *pu1, tp_pilha *pu2, tp_pilha *pu3) {

    int   pos  = jogador->posicao_tabuleiro;
    Casa *casa = &banco_c[pos];

    printf("\n  [Casa %2d | Tipo: %s]\n", pos, casa->tipo_casa);

    /* ------ NORMAL: sem efeito ------ */
    if (strcmp(casa->tipo_casa, TIPO_NORMAL) == 0) {
        printf("  Casa normal. Nenhum efeito adicional.\n");
    }

    /* ------ SORTE: avança usando ->prox ------ */
    else if (strcmp(casa->tipo_casa, TIPO_SORTE) == 0) {
        int nova_pos = mover_na_lista(caminho, pos, CASAS_BONUS_SORTE);
        printf("  ** SORTE! %s avanca %d casas: %d -> %d **\n",
               jogador->nome, CASAS_BONUS_SORTE, pos, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;
    }

    /* ------ REVES: recua usando ->ant ------ */
    else if (strcmp(casa->tipo_casa, TIPO_REVES) == 0) {
        int nova_pos = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_REVES);
        printf("  ** REVES! %s volta %d casas: %d -> %d **\n",
               jogador->nome, CASAS_PENALIDADE_REVES, pos, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;
    }

    /* ------ PERGUNTA: movimenta a pilha (pop) e verifica resposta ------ */
    else if (strcmp(casa->tipo_casa, TIPO_PERGUNTA) == 0) {
        tp_pilha *pilha = selecionar_pilha_por_posicao(pos, pu1, pu2, pu3);
        int id_pergunta;

        if (pilha_vazia(pilha)) {
            printf("  Sem perguntas disponiveis para esta unidade.\n");
            printf("  Casa tratada como Normal.\n");
        } else {
            /* Pop: retira a pergunta do topo (movimenta a estrutura) */
            pop(pilha, &id_pergunta);
            Pergunta *pergunta = &banco_p[id_pergunta];

            exibir_pergunta(*pergunta);

            char resposta;
            printf("  Sua resposta (A/B/C/D): ");
            scanf(" %c", &resposta);
            /* Limpa o restante da linha (enter) deixado pelo scanf */
            int _ch;
            while ((_ch = getchar()) != '\n' && _ch != EOF);

            if (verificar_resposta(*pergunta, resposta)) {
                int nova_pos = mover_na_lista(caminho, pos, CASAS_BONUS_ACERTO);
                printf("  CORRETO! %s avanca %d casas bonus: %d -> %d\n",
                       jogador->nome, CASAS_BONUS_ACERTO, pos, nova_pos);
                jogador->posicao_tabuleiro = nova_pos;
            } else {
                int nova_pos = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_ERRO);
                printf("  ERRADO! Resposta certa: %c. %s volta %d casa(s): %d -> %d\n",
                       pergunta->resposta_correta, jogador->nome,
                       CASAS_PENALIDADE_ERRO, pos, nova_pos);
                jogador->posicao_tabuleiro = nova_pos;
            }
        }
    }

    return jogador->posicao_tabuleiro;
}

/* ================================================================
 * imprimir_tabuleiro
 * Percorre a lista encadeada (->prox) e exibe cada casa com seu tipo
 * e os jogadores que estão nela.
 * ================================================================ */
void imprimir_tabuleiro(Caminho *caminho, Casa banco_c[],
                         Jogador banco_j[], int qtd_jogadores) {
    printf("\n=== TABULEIRO (Lista Duplamente Encadeada) ===\n");

    tp_no *atu = caminho->ini;
    while (atu != NULL) {
        int id = atu->info;

        /* Destaque para a casa inicial e final */
        if (id == 0)
            printf("  [INICIO");
        else if (id == TAMANHO_CAMINHO - 1)
            printf("  [CHEGADA");
        else
            printf("  [%2d|%-8s", id, banco_c[id].tipo_casa);

        printf("]");

        /* Marca os jogadores presentes nesta casa */
        for (int j = 0; j < qtd_jogadores; j++) {
            if (banco_j[j].posicao_tabuleiro == id)
                printf(" <-- %s", banco_j[j].nome);
        }
        printf("\n");
        atu = atu->prox;
    }
    printf("==============================================\n\n");
}

#endif /* CAMINHO_H */