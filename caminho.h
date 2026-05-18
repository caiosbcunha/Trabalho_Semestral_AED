#ifndef CAMINHO_H
#define CAMINHO_H

#define TAMANHO_CAMINHO 45

#include "listade.h"
#include "jogador.h"
#include "perguntas.h"
#include <stdio.h>
#include <stdlib.h>

/* ----------------------------------------------------------
 * Tipos de casa
 * ---------------------------------------------------------- */
#define TIPO_NORMAL    1
#define TIPO_SORTE     2
#define TIPO_REVES     3
#define TIPO_PERGUNTA  4

/* ----------------------------------------------------------
 * Constantes de efeito
 * ---------------------------------------------------------- */
#define CASAS_BONUS_SORTE      3
#define CASAS_PENALIDADE_REVES 2
#define CASAS_BONUS_ACERTO     2
#define CASAS_PENALIDADE_ERRO  1

/* ----------------------------------------------------------
 * Limites de unidade tematica
 * Casa  0-14  -> Unidade 1 (pilha_u1)
 * Casa 15-29  -> Unidade 2 (pilha_u2)
 * Casa 30-44  -> Unidade 3 (pilha_u3)
 * ---------------------------------------------------------- */
#define LIMITE_U1 15
#define LIMITE_U2 30

/* ----------------------------------------------------------
 * Estruturas
 * ---------------------------------------------------------- */
typedef struct {
    int id_casa;
    int tipo_casa; /* 1=Normal | 2=Sorte | 3=Reves | 4=Pergunta */
} Casa;

typedef tp_listad Caminho;

/* ============================================================
 * criar_caminho
 * Inicializa as 45 casas com seus tipos e as insere na lista
 * duplamente encadeada que representa o tabuleiro.
 * ============================================================ */
Caminho *criar_caminho(Casa banco[]) {
    int tipos[TAMANHO_CAMINHO] = {
        /* 0  */ TIPO_NORMAL,
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
        /* 14 */ TIPO_PERGUNTA,  /* Ultima casa da Unidade 1 */
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
        /* 29 */ TIPO_PERGUNTA,  /* Ultima casa da Unidade 2 */
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
        /* 44 */ TIPO_NORMAL    /* Chegada */
    };

    Caminho *caminho = inicializa_listad();

    for (int i = 0; i < TAMANHO_CAMINHO; i++) {
        banco[i].id_casa   = i;
        banco[i].tipo_casa = tipos[i];
        insere_listad_no_fim(caminho, banco[i].id_casa);
    }

    printf("-> Caminho criado com %d casas!\n\n", TAMANHO_CAMINHO);
    return caminho;
}

/* ============================================================
 * encontrar_no_por_id
 * Percorre a lista e retorna o no cujo campo 'info' == id_casa.
 * Retorna NULL se nao encontrado.
 * ============================================================ */
tp_no *encontrar_no_por_id(Caminho *caminho, int id_casa) {
    tp_no *atu = caminho->ini;
    while (atu != NULL) {
        if (atu->info == id_casa)
            return atu;
        atu = atu->prox;
    }
    return NULL;
}

/* ============================================================
 * mover_na_lista
 * Navega pela lista encadeada a partir de 'pos_atual' avancando
 * (passos > 0) ou recuando (passos < 0) casa a casa.
 *
 * - Avanco: usa ->prox; para no ultimo no se chegar a casa 44.
 * - Recuo : usa ->ant; para no primeiro no (casa 0) se necessario.
 *
 * Retorna o id da casa de destino.
 * ============================================================ */
int mover_na_lista(Caminho *caminho, int pos_atual, int passos) {
    tp_no *no = encontrar_no_por_id(caminho, pos_atual);

    if (no == NULL) return pos_atual; /* seguranca: posicao invalida */

    if (passos > 0) {
        for (int i = 0; i < passos && no->prox != NULL; i++)
            no = no->prox;
    } else if (passos < 0) {
        int recuo = -passos;
        for (int i = 0; i < recuo && no->ant != NULL; i++)
            no = no->ant;
    }

    return no->info;
}

/* ============================================================
 * rolar_dado
 * Retorna um valor aleatorio entre 1 e 6.
 * ============================================================ */
int rolar_dado() {
    return (rand() % 6) + 1;
}

/* ============================================================
 * selecionar_pilha_por_posicao
 * Retorna a pilha de perguntas correspondente a unidade tematica
 * da posicao atual do jogador no tabuleiro.
 * ============================================================ */
tp_pilha *selecionar_pilha_por_id(int id,
                                        tp_pilha *pu1,
                                        tp_pilha *pu2,
                                        tp_pilha *pu3) {
    if (id < LIMITE_U1) return pu1;
    if (id < LIMITE_U2) return pu2;
    return pu3;
}

/* ============================================================
 * aplicar_efeito_casa
 * Le o tipo da casa onde o jogador parou e executa a acao:
 *
 *   NORMAL   -> nenhum efeito
 *   SORTE    -> avanca CASAS_BONUS_SORTE casas
 *   REVES    -> recua  CASAS_PENALIDADE_REVES casas
 *   PERGUNTA -> retira pergunta da pilha da unidade correta,
 *               exibe ao jogador e processa a resposta:
 *                 acerto -> avanca CASAS_BONUS_ACERTO casas
 *                 erro   -> recua  CASAS_PENALIDADE_ERRO casas
 *
 * Retorna 1 em acerto/efeito neutro, 0 em erro/penalidade.
 * ============================================================ */
int aplicar_efeito_casa(Jogador *jogador, Caminho *caminho,
                        Casa banco_c[], Pergunta banco_p[],
                        tp_pilha *pu1, tp_pilha *pu2, tp_pilha *pu3) {

    int   pos  = jogador->posicao_tabuleiro;
    Casa *casa = &banco_c[pos];

    const char *nomes_tipo[] = { "", "Normal", "Sorte", "Reves", "Pergunta" };
    printf("\n  [Casa %2d | %s]\n", pos, nomes_tipo[casa->tipo_casa]);

    /* ------ Casa Normal ------ */
    if (casa->tipo_casa == TIPO_NORMAL) {
        printf("  Casa normal. Nenhum efeito adicional.\n");
        return 1;
    }

    /* ------ Casa Sorte ------ */
    if (casa->tipo_casa == TIPO_SORTE) {
        int nova_pos = mover_na_lista(caminho, pos, CASAS_BONUS_SORTE);
        printf("  ** SORTE! %s avanca %d casas: %d -> %d **\n",
               jogador->nome, CASAS_BONUS_SORTE, pos, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;
        return 1;
    }

    /* ------ Casa Reves ------ */
    if (casa->tipo_casa == TIPO_REVES) {
        int nova_pos = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_REVES);
        printf("  ** REVES! %s volta %d casas: %d -> %d **\n",
               jogador->nome, CASAS_PENALIDADE_REVES, pos, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;
        return 0;
    }

    /* ------ Casa Pergunta ------ */
    if (casa->tipo_casa == TIPO_PERGUNTA) {
        tp_pilha *pilha = selecionar_pilha_por_id(pos, pu1, pu2, pu3);

        if (pilha_vazia(pilha)) {
            printf("  [!] Sem perguntas disponiveis para esta unidade.\n");
            printf("      Casa tratada como Normal.\n");
            return 1;
        }

        int id_pergunta;
        pop(pilha, &id_pergunta);
        Pergunta *p = &banco_p[id_pergunta];

        exibir_pergunta(*p);

        char resposta;
        printf("\n  Sua resposta (A/B/C/D): ");
        scanf(" %c", &resposta);

        if (verificar_resposta(*p, resposta)) {
            int nova_pos = mover_na_lista(caminho, pos, CASAS_BONUS_ACERTO);
            printf("\n  ** ACERTO! %s avanca %d casas: %d -> %d **\n",
                   jogador->nome, CASAS_BONUS_ACERTO, pos, nova_pos);
            jogador->posicao_tabuleiro = nova_pos;
            return 1;
        } else {
            int nova_pos = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_ERRO);
            printf("\n  ** ERROU! Resposta correta: %c. %s volta %d casa(s): %d -> %d **\n",
                   p->resposta_correta, jogador->nome,
                   CASAS_PENALIDADE_ERRO, pos, nova_pos);
            jogador->posicao_tabuleiro = nova_pos;
            return 0;
        }
    }

    return 1; /* tipo desconhecido: nenhuma acao */
}

/* ============================================================
 * imprimir_tabuleiro
 * Exibe o estado atual do tabuleiro percorrendo a lista
 * encadeada e indicando onde cada jogador esta posicionado.
 *
 * Legenda de tipos:
 *   [.] Normal   [+] Sorte   [-] Reves   [?] Pergunta
 * ============================================================ */
void imprimir_tabuleiro(Caminho *caminho, Casa banco_c[],
                        Jogador banco_j[], int qtd_jogadores) {
    printf("\n=== TABULEIRO ===\n");

    tp_no *atu = caminho->ini;
    while (atu != NULL) {
        int id = atu->info;

        char tipo_label;
        switch (banco_c[id].tipo_casa) {
            case TIPO_NORMAL:   tipo_label = '.'; break;
            case TIPO_SORTE:    tipo_label = '+'; break;
            case TIPO_REVES:    tipo_label = '-'; break;
            case TIPO_PERGUNTA: tipo_label = '?'; break;
            default:            tipo_label = ' '; break;
        }

        printf("  Casa %2d [%c]", id, tipo_label);

        for (int i = 0; i < qtd_jogadores; i++) {
            if (banco_j[i].posicao_tabuleiro == id)
                printf(" <%s>", banco_j[i].nome);
        }

        if (id == 0)                   printf(" <- INICIO");
        if (id == TAMANHO_CAMINHO - 1) printf(" <- CHEGADA");

        printf("\n");
        atu = atu->prox;
    }

    printf("\n  Legenda: [.] Normal  [+] Sorte  [-] Reves  [?] Pergunta\n\n");
}

#endif /* CAMINHO_H */