#ifndef CAMINHO_H
#define CAMINHO_H

#define TAMANHO_CAMINHO 45

#include "listade.h"
#include "jogador.h"
#include "perguntas.h"
#include <stdio.h>
#include <stdlib.h>

/* ----------------------------------------------------------
 * limpar_stdin
 * Consome todos os caracteres que o scanf deixou no buffer
 * (inclusive o '\n' do Enter), impedindo que getchar() leia
 * uma tecla "fantasma" no próximo turno.
 * ---------------------------------------------------------- */
void limpar_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ----------------------------------------------------------
 * Tipos de casa
 * ---------------------------------------------------------- */
#define TIPO_NORMAL    1
#define TIPO_SORTE     2
#define TIPO_REVES     3
#define TIPO_PERGUNTA  4

/* ----------------------------------------------------------
 * Constantes de efeito
 *
 * Observacao: o avanco por ACERTO NAO e fixo — depende do nivel
 * da pergunta (Facil = 1, Medio = 2, Dificil = 3 casas), conforme
 * o requisito "associar cada nivel a uma quantidade diferente de
 * avanco". Por isso nao existe mais uma constante CASAS_BONUS_ACERTO.
 * ---------------------------------------------------------- */
#define CASAS_BONUS_SORTE      3
#define CASAS_PENALIDADE_REVES 2
#define CASAS_PENALIDADE_ERRO  1

/* ----------------------------------------------------------
 * Pontuação por nível de pergunta
 *   Nível 1 (Fácil)  → 10 pts
 *   Nível 2 (Médio)  → 20 pts
 *   Nível 3 (Difícil)→ 30 pts
 * ---------------------------------------------------------- */
#define PONTOS_POR_NIVEL 10

/* ----------------------------------------------------------
 * Limites de unidade temática
 *   Casa  0-14 → Unidade 1
 *   Casa 15-29 → Unidade 2
 *   Casa 30-44 → Unidade 3
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

/* ----------------------------------------------------------
 * ResultadoJogada
 * Resumo do que aconteceu quando o jogador caiu numa casa.
 * Preenchido por aplicar_efeito_casa() e usado pelo main para
 * atualizar as estatisticas (AVL) e gravar o historico em CSV.
 * ---------------------------------------------------------- */
typedef struct {
    int  respondeu_pergunta; /* 1 se a casa era de pergunta e foi respondida */
    int  id_pergunta;        /* id da pergunta respondida                    */
    char resposta_jogador;   /* alternativa escolhida (A/B/C/D)              */
    int  acertou;            /* 1 = acertou | 0 = errou                      */
    int  pontos_ganhos;      /* pontos somados nesta jogada                  */
    int  casa_pergunta;      /* casa onde a pergunta foi respondida          */
} ResultadoJogada;

/* ============================================================
 * criar_caminho
 * ============================================================ */
Caminho *criar_caminho(Casa banco[]) {
    int tipos[TAMANHO_CAMINHO] = {
        TIPO_NORMAL,   TIPO_NORMAL,   TIPO_PERGUNTA, TIPO_SORTE,
        TIPO_NORMAL,   TIPO_REVES,    TIPO_PERGUNTA, TIPO_NORMAL,
        TIPO_SORTE,    TIPO_PERGUNTA, TIPO_NORMAL,   TIPO_REVES,
        TIPO_PERGUNTA, TIPO_NORMAL,   TIPO_PERGUNTA,
        TIPO_NORMAL,   TIPO_SORTE,    TIPO_PERGUNTA, TIPO_NORMAL,
        TIPO_REVES,    TIPO_PERGUNTA, TIPO_NORMAL,   TIPO_SORTE,
        TIPO_PERGUNTA, TIPO_REVES,    TIPO_NORMAL,   TIPO_PERGUNTA,
        TIPO_NORMAL,   TIPO_SORTE,    TIPO_PERGUNTA,
        TIPO_NORMAL,   TIPO_REVES,    TIPO_PERGUNTA, TIPO_NORMAL,
        TIPO_SORTE,    TIPO_PERGUNTA, TIPO_NORMAL,   TIPO_REVES,
        TIPO_PERGUNTA, TIPO_NORMAL,   TIPO_SORTE,    TIPO_PERGUNTA,
        TIPO_NORMAL,   TIPO_REVES,    TIPO_NORMAL
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
 * ============================================================ */
tp_no *encontrar_no_por_id(Caminho *caminho, int id_casa) {
    tp_no *atu = caminho->ini;
    while (atu != NULL) {
        if (atu->info == id_casa) return atu;
        atu = atu->prox;
    }
    return NULL;
}

/* ============================================================
 * mover_na_lista
 * ============================================================ */
int mover_na_lista(Caminho *caminho, int pos_atual, int passos) {
    tp_no *no = encontrar_no_por_id(caminho, pos_atual);
    if (no == NULL) return pos_atual;

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
 * ============================================================ */
int rolar_dado() {
    return (rand() % 6) + 1;
}

/* ============================================================
 * selecionar_pilha_por_id
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
 * unidade_por_id
 * Retorna a unidade (1, 2 ou 3) correspondente a uma casa.
 * ============================================================ */
int unidade_por_id(int id) {
    if (id < LIMITE_U1) return 1;
    if (id < LIMITE_U2) return 2;
    return 3;
}

/* ============================================================
 * tipo_casa_str
 * Rotulo textual (sem acento) do tipo de casa, para relatorios.
 * ============================================================ */
const char *tipo_casa_str(int tipo) {
    switch (tipo) {
        case TIPO_NORMAL:   return "Normal";
        case TIPO_SORTE:    return "Sorte";
        case TIPO_REVES:    return "Reves";
        case TIPO_PERGUNTA: return "Pergunta";
        default:            return "Desconhecido";
    }
}

/* ============================================================
 * aplicar_efeito_casa
 *
 * Aplica o efeito da casa onde o jogador parou e devolve um
 * ResultadoJogada descrevendo o que aconteceu (usado pelo main
 * para atualizar a AVL de estatisticas e gravar o historico).
 *
 * Casa de PERGUNTA:
 *   • O nivel da pergunta e sorteado de forma aleatoria, pois a
 *     pilha da unidade foi previamente embaralhada com os tres
 *     niveis misturados; retira-se sempre o elemento do topo (LIFO).
 *   • Se a pilha esgotar, ela e reembaralhada e reconstituida
 *     antes de retirar a proxima pergunta.
 *   • Acerto → +nivel*PONTOS_POR_NIVEL e avanca 'nivel' casas
 *              (Facil=1, Medio=2, Dificil=3).
 *   • Erro   → volta CASAS_PENALIDADE_ERRO casa(s).
 * ============================================================ */
ResultadoJogada aplicar_efeito_casa(Jogador *jogador, Caminho *caminho,
                        Casa banco_c[], Pergunta banco_p[],
                        tp_pilha *pu1, tp_pilha *pu2, tp_pilha *pu3) {

    int   pos  = jogador->posicao_tabuleiro;
    Casa *casa = &banco_c[pos];

    /* Resultado "vazio" por padrao (casas que nao sao de pergunta) */
    ResultadoJogada r = { 0, -1, ' ', 0, 0, pos };

    /* ------ Casa Normal ------ */
    if (casa->tipo_casa == TIPO_NORMAL) {
        printf("  Casa %d [NORMAL]   -> sem efeito.\n", pos);
        return r;
    }

    /* ------ Casa Sorte ------ */
    if (casa->tipo_casa == TIPO_SORTE) {
        int nova_pos = mover_na_lista(caminho, pos, CASAS_BONUS_SORTE);
        printf("  Casa %d [SORTE]    -> SORTE! Avanca %d casas: %d -> %d\n",
               pos, CASAS_BONUS_SORTE, pos, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;
        return r;
    }

    /* ------ Casa Reves ------ */
    if (casa->tipo_casa == TIPO_REVES) {
        int nova_pos = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_REVES);
        printf("  Casa %d [REVES]    -> REVES! Volta %d casas: %d -> %d\n",
               pos, CASAS_PENALIDADE_REVES, pos, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;
        return r;
    }

    /* ------ Casa Pergunta ------ */
    if (casa->tipo_casa == TIPO_PERGUNTA) {
        tp_pilha *pilha = selecionar_pilha_por_id(pos, pu1, pu2, pu3);

        /* Conjunto esgotado: reembaralha e reconstitui a unidade */
        if (pilha_vazia(pilha)) {
            int unidade = unidade_por_id(pos);
            printf("  [i] Perguntas da Unidade %d esgotadas -> reembaralhando...\n",
                   unidade);
            preencher_pilha_unidade(pilha, banco_p, unidade);
        }

        int id_pergunta;
        pop(pilha, &id_pergunta);
        Pergunta *p = &banco_p[id_pergunta];

        printf("  Casa %d [PERGUNTA]  (Unid.%d | %s | %s)\n",
               pos, p->unidade, p->tema, nivel_para_str(p->nivel));
        exibir_pergunta(*p);

        char resposta;
        printf("\n  Resposta (A/B/C/D): ");
        scanf(" %c", &resposta);
        limpar_stdin(); /* remove o '\n' (e qualquer outro char) que scanf deixou no buffer */

        /* Normaliza a resposta para maiuscula no registro */
        char resp_norm = (resposta >= 'a' && resposta <= 'z')
                         ? (char)(resposta - 32) : resposta;

        r.respondeu_pergunta = 1;
        r.id_pergunta        = id_pergunta;
        r.resposta_jogador   = resp_norm;
        r.casa_pergunta      = pos;

        if (verificar_resposta(*p, resposta)) {

            /* --- Acerto: pontua, registra e avanca 'nivel' casas --- */
            int pontos  = p->nivel * PONTOS_POR_NIVEL;
            int avanco  = p->nivel; /* Facil=1, Medio=2, Dificil=3 */
            jogador->pontuacao += pontos;

            if (jogador->qtd_acertadas < MAX_HISTORICO_PERGUNTAS)
                jogador->perguntas_acertadas[jogador->qtd_acertadas++] = id_pergunta;

            int nova_pos = mover_na_lista(caminho, pos, avanco);
            printf("  [ACERTOU] +%d pts. Avanca %d casa(s): %d -> %d\n",
                   pontos, avanco, pos, nova_pos);
            jogador->posicao_tabuleiro = nova_pos;

            r.acertou       = 1;
            r.pontos_ganhos = pontos;
            return r;

        } else {

            /* --- Erro: apenas registra (sem perda de pontos) --- */
            if (jogador->qtd_erradas < MAX_HISTORICO_PERGUNTAS)
                jogador->perguntas_erradas[jogador->qtd_erradas++] = id_pergunta;

            int nova_pos = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_ERRO);
            printf("  [ERROU] Resposta certa: %c. Volta %d casa(s): %d -> %d\n",
                   p->resposta_correta, CASAS_PENALIDADE_ERRO, pos, nova_pos);
            jogador->posicao_tabuleiro = nova_pos;

            r.acertou       = 0;
            r.pontos_ganhos = 0;
            return r;
        }
    }

    return r;
}

/* ----------------------------------------------------------
 * Quantas casas sao exibidas por linha do tabuleiro.
 * 15 = 1 unidade (o tabuleiro tem 45 casas = 3 unidades).
 * ---------------------------------------------------------- */
#define CASAS_POR_LINHA 15

/* ============================================================
 * imprimir_tabuleiro
 * Mostra o tabuleiro na HORIZONTAL, em trilhas por unidade
 * (15 casas por linha). Cada casa e uma caixinha [NN T] e os
 * jogadores aparecem pelo numero, centralizados sob a casa.
 *
 *   T: I=Inicio  C=Chegada  .=Normal  +=Sorte  -=Reves  ?=Pergunta
 * ============================================================ */
void imprimir_tabuleiro(Caminho *caminho, Casa banco_c[],
                        Jogador banco_j[], int qtd_jogadores) {
    (void)caminho; /* o layout usa o banco de casas indexado por id */

    imprimir_titulo("CAMINHO DO CONHECIMENTO");
    printf("  Legenda: [.]Normal  [+]Sorte  [-]Reves  [?]Pergunta   (I=Inicio  C=Chegada)\n");

    for (int base = 0; base < TAMANHO_CAMINHO; base += CASAS_POR_LINHA) {
        int fim = base + CASAS_POR_LINHA;
        if (fim > TAMANHO_CAMINHO) fim = TAMANHO_CAMINHO;

        printf("\n  Unidade %d\n  ", unidade_por_id(base));

        /* ---- Linha das casas ---- */
        for (int id = base; id < fim; id++) {
            char t;
            switch (banco_c[id].tipo_casa) {
                case TIPO_SORTE:    t = '+'; break;
                case TIPO_REVES:    t = '-'; break;
                case TIPO_PERGUNTA: t = '?'; break;
                default:            t = '.'; break;
            }
            if (id == 0)                   t = 'I';
            if (id == TAMANHO_CAMINHO - 1) t = 'C';
            printf("[%02d%c]", id, t);
        }
        printf("\n  ");

        /* ---- Linha dos jogadores (numero centralizado sob a casa) ---- */
        for (int id = base; id < fim; id++) {
            char celula[6] = "     "; /* 5 espacos = largura de [NN T] */
            int n = 0;

            for (int i = 0; i < qtd_jogadores && n < 4; i++)
                if (banco_j[i].posicao_tabuleiro == id)
                    n++; /* conta quantos jogadores estao nesta casa */

            int ini = (5 - n) / 2; /* posicao inicial para centralizar */
            int col = 0;
            for (int i = 0; i < qtd_jogadores && col < n; i++)
                if (banco_j[i].posicao_tabuleiro == id)
                    celula[ini + col++] = (char)('1' + i);

            celula[5] = '\0';
            printf("%s", celula);
        }
        printf("\n");
    }

    /* ---- Legenda com nome e posicao de cada jogador ---- */
    printf("\n  Jogadores: ");
    for (int i = 0; i < qtd_jogadores; i++) {
        printf("%d=%s (casa %d)", i + 1, banco_j[i].nome,
               banco_j[i].posicao_tabuleiro);
        if (i < qtd_jogadores - 1) printf("  |  ");
    }
    printf("\n");
    linha_separadora('=', LARGURA_TELA);
}

#endif /* CAMINHO_H */
