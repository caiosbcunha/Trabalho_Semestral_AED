/* ================================================================
 * jogo.c — Ponto de entrada e loop principal do jogo
 *          "Caminho do Conhecimento"
 *
 * Fluxo geral:
 *  1. Inicialização dos três módulos:
 *       - Jogadores  → Fila (FIFO) de turnos
 *       - Perguntas  → Pilhas por Unidade temática
 *       - Caminho    → Lista Duplamente Encadeada de 45 casas
 *  2. Loop de turnos:
 *       a. Retira o próximo jogador da Fila
 *       b. Rola o dado (1–6)
 *       c. Move o jogador navegando pela Lista Encadeada (->prox)
 *       d. Aplica o efeito da casa (Normal/Sorte/Reves/Pergunta)
 *       e. Verifica condição de vitória (casa 44)
 *       f. Reinsere o jogador no fim da Fila (rodízio)
 *  3. Exibe o vencedor
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Módulos do jogo — cada um encapsula sua estrutura de dados */
#include "jogador.h"   /* Jogador + Fila de turnos             */
#include "perguntas.h" /* Pergunta + Pilhas por Unidade        */
#include "caminho.h"   /* Casa + Lista Duplamente Encadeada    */

/* ----------------------------------------------------------------
 * flush_buffer
 * Descarta todos os caracteres restantes no stdin até o '\n'.
 * Evita que entradas anteriores (scanf) contaminem leituras futuras.
 * ---------------------------------------------------------------- */
void flush_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ================================================================
 * main
 * ================================================================ */
int main() {

    /* Semente para números aleatórios (dado + embaralhamento) */
    srand((unsigned)time(NULL));

    /* ----------------------------------------------------------
     * Variáveis dos três módulos
     * ---------------------------------------------------------- */

    /* Módulo Jogador — Fila FIFO */
    tp_fila fila_jogadores;
    Jogador banco_jogadores[MAX_JOGADORES];
    int     qtd_jogadores;

    /* Módulo Perguntas — uma Pilha por Unidade */
    tp_pilha pilha_u1, pilha_u2, pilha_u3;
    Pergunta banco_perguntas[QTD_PERGUNTAS];

    /* Módulo Caminho — Lista Duplamente Encadeada */
    Casa    banco_casas[TAMANHO_CAMINHO];
    Caminho *caminho;

    /* ----------------------------------------------------------
     * Cabeçalho
     * ---------------------------------------------------------- */
    printf("=========================================\n");
    printf("       CAMINHO DO CONHECIMENTO           \n");
    printf("=========================================\n\n");

    /* ----------------------------------------------------------
     * Inicialização dos módulos
     * ---------------------------------------------------------- */

    /* 1. Cria os jogadores e os coloca na Fila */
    criar_jogadores(&fila_jogadores, banco_jogadores);
    /* Limpa qualquer caractere residual deixado por scanf nas funcoes de cadastro */
    flush_buffer();
    qtd_jogadores = tamanhoFila(&fila_jogadores); /* Salva antes do loop */

    /* 2. Cria as perguntas, embaralha e empilha por Unidade */
    criar_perguntas(&pilha_u1, &pilha_u2, &pilha_u3, banco_perguntas);

    /* 3. Cria o caminho (Lista Duplamente Encadeada com 45 casas) */
    caminho = criar_caminho(banco_casas);

    /* Exibe o estado inicial do tabuleiro */
    imprimir_tabuleiro(caminho, banco_casas, banco_jogadores, qtd_jogadores);

    /* ----------------------------------------------------------
     * Loop principal do jogo
     * ---------------------------------------------------------- */
    int vencedor = -1;   /* ID do jogador que venceu; -1 = nenhum ainda */
    int id_atual;
    int turno = 1;

    while (vencedor == -1) {

        /* Retira o próximo jogador da Fila (FIFO) */
        removeFila(&fila_jogadores, &id_atual);
        Jogador *jogador = &banco_jogadores[id_atual];

        /* Cabeçalho do turno */
        printf("=========================================\n");
        printf(" TURNO %d | %s | Casa atual: %d\n",
               turno, jogador->nome, jogador->posicao_tabuleiro);
        printf("=========================================\n");

        /* Aguarda o jogador confirmar antes de rolar */
        printf("  [Pressione ENTER para rolar o dado...] ");
        fflush(stdout); /* Garante que a mensagem seja exibida antes de esperar o ENTER */
        getchar();     /* Espera o ENTER do usuario */

        /* ------ Rola o dado e move o jogador ------
         * A navegação usa os ponteiros ->prox da lista encadeada,
         * avançando casa a casa a partir da posição atual. */
        int dado    = rolar_dado();
        int pos_ant = jogador->posicao_tabuleiro;
        int nova_pos = mover_na_lista(caminho, pos_ant, dado);

        printf("\n  Dado: %d  |  Movimento: Casa %d -> Casa %d\n",
               dado, pos_ant, nova_pos);

        jogador->posicao_tabuleiro = nova_pos;

        /* ------ Aplica o efeito da casa onde parou ------ */
        aplicar_efeito_casa(jogador, caminho, banco_casas, banco_perguntas,
                            &pilha_u1, &pilha_u2, &pilha_u3);

        printf("\n  Posicao final neste turno: Casa %d\n",
               jogador->posicao_tabuleiro);

        /* ------ Verifica condição de vitória ------ */
        if (jogador->posicao_tabuleiro >= TAMANHO_CAMINHO - 1) {
            vencedor = id_atual;
            printf("\n  *** %s chegou na CHEGADA e VENCEU o jogo! ***\n",
                   jogador->nome);
            break;
        }

        /* Exibe o tabuleiro atualizado */
        imprimir_tabuleiro(caminho, banco_casas, banco_jogadores, qtd_jogadores);

        /* Reinsere o jogador no FIM da Fila — rodízio de turnos */
        insereFila(&fila_jogadores, id_atual);
        turno++;
    }

    /* ----------------------------------------------------------
     * Tela de vitória
     * ---------------------------------------------------------- */
    printf("\n=========================================\n");
    printf("           ** FIM DE JOGO! **            \n");
    printf("  Vencedor: %s\n", banco_jogadores[vencedor].nome);
    printf("  Total de turnos: %d\n", turno);
    printf("=========================================\n");

    return 0;
}