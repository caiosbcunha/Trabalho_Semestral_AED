/* ================================================================
 * jogo.c — Ponto de entrada e loop principal do jogo
 *          "Caminho do Conhecimento"
 *
 * NOVAS FUNCIONALIDADES em relação à versão original:
 *
 *   1. Árvore AVL de estatísticas (arvoreAVL.h)
 *      • Cada vez que um jogador cai em uma casa (após o dado),
 *        o id dessa casa é inserido na AVL.
 *      • A árvore garante busca/inserção em O(log n) no pior caso.
 *      • Ao final do jogo, exibe quantas vezes cada casa foi
 *        visitada, percorrendo a árvore em ordem crescente (emOrd).
 *
 *   2. Persistência em disco e ranking (ranking.h)
 *      • Ao final do jogo, salva em 'resultados.txt':
 *          nome, pontuação, IDs das perguntas certas e erradas.
 *      • Em seguida, lê esse arquivo e exibe o ranking ordenado
 *        da maior para a menor pontuação, com detalhes de cada
 *        pergunta acertada/errada.
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Módulos do jogo */
#include "jogador.h"    /* Jogador + Fila de turnos              */
#include "perguntas.h"  /* Pergunta + Pilhas por Unidade         */
#include "caminho.h"    /* Casa + Lista Duplamente Encadeada     */
#include "arvoreAVL.h"  /* Árvore AVL para estatísticas de casas */
#include "ranking.h"    /* Persistência em disco + ranking       */

/* ----------------------------------------------------------------
 * flush_buffer — descarta stdin até o '\n'
 * ---------------------------------------------------------------- */
void flush_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ================================================================
 * main
 * ================================================================ */
int main() {

    srand((unsigned)time(NULL));

    /* ----------------------------------------------------------
     * Módulo Jogador — Fila FIFO
     * ---------------------------------------------------------- */
    tp_fila fila_jogadores;
    Jogador banco_jogadores[MAX_JOGADORES];
    int     qtd_jogadores;

    /* ----------------------------------------------------------
     * Módulo Perguntas — uma Pilha por Unidade
     * ---------------------------------------------------------- */
    tp_pilha pilha_u1, pilha_u2, pilha_u3;
    Pergunta banco_perguntas[QTD_PERGUNTAS];

    /* ----------------------------------------------------------
     * Módulo Caminho — Lista Duplamente Encadeada
     * ---------------------------------------------------------- */
    Casa    banco_casas[TAMANHO_CAMINHO];
    Caminho *caminho;

    /* ----------------------------------------------------------
     * Árvore AVL — estatísticas de visitas por casa
     * ---------------------------------------------------------- */
    ArvAVL *avl_casas = criarAVL();

    /* ----------------------------------------------------------
     * Cabeçalho
     * ---------------------------------------------------------- */
    printf("=========================================\n");
    printf("       CAMINHO DO CONHECIMENTO           \n");
    printf("=========================================\n\n");

    /* ----------------------------------------------------------
     * Inicialização dos módulos
     * ---------------------------------------------------------- */
    criar_jogadores(&fila_jogadores, banco_jogadores);
    flush_buffer();
    qtd_jogadores = tamanhoFila(&fila_jogadores);

    criar_perguntas(&pilha_u1, &pilha_u2, &pilha_u3, banco_perguntas);

    caminho = criar_caminho(banco_casas);

    imprimir_tabuleiro(caminho, banco_casas, banco_jogadores, qtd_jogadores);

    /* ----------------------------------------------------------
     * Loop principal do jogo
     * ---------------------------------------------------------- */
    int vencedor = -1;
    int id_atual;
    int turno = 1;

    while (vencedor == -1) {

        removeFila(&fila_jogadores, &id_atual);
        Jogador *jogador = &banco_jogadores[id_atual];

        printf("=========================================\n");
        printf(" TURNO %d | %s | Casa atual: %d | Pontos: %d\n",
               turno, jogador->nome,
               jogador->posicao_tabuleiro,
               jogador->pontuacao);
        printf("=========================================\n");

        printf("  [Pressione ENTER para rolar o dado...] ");
        fflush(stdout);
        flush_buffer(); /* lê até '\n': aguarda o Enter e descarta chars extras */

        /* ------ Rola o dado e move ------ */
        int dado    = rolar_dado();
        int pos_ant = jogador->posicao_tabuleiro;
        int nova_pos = mover_na_lista(caminho, pos_ant, dado);

        printf("\n  Dado: %d  |  Movimento: Casa %d -> Casa %d\n",
               dado, pos_ant, nova_pos);

        jogador->posicao_tabuleiro = nova_pos;

        /* ------ Registra a visita na Árvore AVL ------ *
         * O jogador "caiu" em nova_pos; inserir() incrementa
         * o contador se a casa já estava na árvore (O(log n)).  */
        inserir(avl_casas, nova_pos);

        /* ------ Aplica o efeito da casa ------ */
        aplicar_efeito_casa(jogador, caminho, banco_casas, banco_perguntas,
                            &pilha_u1, &pilha_u2, &pilha_u3);

        printf("\n  Posicao final neste turno: Casa %d  | Pontos: %d\n",
               jogador->posicao_tabuleiro,
               jogador->pontuacao);

        /* ------ Verifica vitória ------ */
        if (jogador->posicao_tabuleiro >= TAMANHO_CAMINHO - 1) {
            vencedor = id_atual;
            printf("\n  *** %s chegou na CHEGADA e VENCEU o jogo! ***\n",
                   jogador->nome);
            break;
        }

        imprimir_tabuleiro(caminho, banco_casas, banco_jogadores, qtd_jogadores);

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

    /* ----------------------------------------------------------
     * 1. Estatísticas de visitas por casa (Árvore AVL)
     *    Percorre em ordem crescente de id — O(n)
     * ---------------------------------------------------------- */
    printf("\n=========================================\n");
    printf("   ESTATISTICAS DE VISITAS POR CASA      \n");
    printf("   (Arvore AVL — percurso em ordem)       \n");
    printf("=========================================\n");
    emOrd_casas(avl_casas);
    printf("=========================================\n");

    /* ----------------------------------------------------------
     * 2. Salva resultados em disco
     * ---------------------------------------------------------- */
    salvar_resultados(banco_jogadores, qtd_jogadores);

    /* ----------------------------------------------------------
     * 3. Lê do disco e exibe ranking
     * ---------------------------------------------------------- */
    exibir_ranking_do_disco(banco_perguntas);

    /* ----------------------------------------------------------
     * Libera memória
     * ---------------------------------------------------------- */
    liberaABB(avl_casas);

    return 0;
}
