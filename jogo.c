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
 *        visitada, com acertos e erros, percorrendo em ordem.
 *
 *   2. Persistência em disco e ranking (ranking.h / historico.h /
 *      estatisticas.h)
 *      • Ao final do jogo, salva 'resultados.txt' e os CSVs
 *        (perguntas, histórico de respostas e estatísticas).
 *
 *   3. Interface no terminal organizada (ui.h)
 *      • Acentos corretos (UTF-8), tela limpa a cada turno e
 *        painéis com o tabuleiro na horizontal.
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Módulos do jogo */
#include "ui.h"           /* Interface: tela limpa, divisórias, textos     */
#include "jogador.h"      /* Jogador + Fila de turnos                       */
#include "perguntas.h"    /* Pergunta + Pilhas por Unidade                  */
#include "caminho.h"      /* Casa + Lista Duplamente Encadeada              */
#include "arvoreAVL.h"    /* Árvore AVL para estatísticas de casas          */
#include "ranking.h"      /* Persistência em disco + ranking                */
#include "historico.h"    /* Exportação CSV (modelo do professor)           */
#include "estatisticas.h" /* Estatísticas por casa em disco (CSV)           */

/* ----------------------------------------------------------------
 * flush_buffer — descarta stdin até o '\n' (aguarda o ENTER)
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
     * Tela inicial e inicialização dos módulos
     * ---------------------------------------------------------- */
    limpar_tela();
    imprimir_titulo("CAMINHO DO CONHECIMENTO");
    printf("\n  Bem-vindo(a)! Vamos preparar a partida.\n\n");

    criar_jogadores(&fila_jogadores, banco_jogadores);
    flush_buffer();
    qtd_jogadores = tamanhoFila(&fila_jogadores);

    criar_perguntas(&pilha_u1, &pilha_u2, &pilha_u3, banco_perguntas);

    /* Exporta a tabela de referência das perguntas e inicia o
     * arquivo de histórico (1 linha por resposta) em CSV. */
    salvar_perguntas_csv(banco_perguntas, QTD_PERGUNTAS);
    iniciar_historico_csv();

    caminho = criar_caminho(banco_casas);

    /* ----------------------------------------------------------
     * Loop principal do jogo
     * ---------------------------------------------------------- */
    int vencedor = -1;
    int id_atual;
    int turno = 1;

    while (vencedor == -1) {

        removeFila(&fila_jogadores, &id_atual);
        Jogador *jogador = &banco_jogadores[id_atual];

        /* ------ Tela limpa: tabuleiro no topo + painel do turno ------ */
        limpar_tela();
        imprimir_tabuleiro(caminho, banco_casas, banco_jogadores, qtd_jogadores);

        printf("\n  >> TURNO %d   Jogador: %s   Casa %d   Pontos: %d\n",
               turno, jogador->nome,
               jogador->posicao_tabuleiro, jogador->pontuacao);

        printf("\n  [ENTER para rolar o dado...]");
        fflush(stdout);
        flush_buffer();

        /* ------ Rola o dado e move ------ */
        int dado    = rolar_dado();
        int pos_ant = jogador->posicao_tabuleiro;
        int nova_pos = mover_na_lista(caminho, pos_ant, dado);

        printf("\n  Dado: %d   ->   Casa %d ... %d\n", dado, pos_ant, nova_pos);
        jogador->posicao_tabuleiro = nova_pos;

        /* ------ Registra a visita na Árvore AVL (O(log n)) ------ */
        inserir(avl_casas, nova_pos);

        /* ------ Aplica o efeito da casa ------ */
        ResultadoJogada rj = aplicar_efeito_casa(jogador, caminho, banco_casas,
                                                 banco_perguntas,
                                                 &pilha_u1, &pilha_u2, &pilha_u3);

        /* ------ Se respondeu pergunta: estatísticas e histórico ------ */
        if (rj.respondeu_pergunta) {
            registrar_resultado_casa(avl_casas, rj.casa_pergunta, rj.acertou);
            salvar_historico_resposta(jogador, &banco_perguntas[rj.id_pergunta],
                                      rj.resposta_jogador, rj.acertou,
                                      rj.pontos_ganhos);
        }

        printf("\n  Posicao final: Casa %d   |   Pontos: %d\n",
               jogador->posicao_tabuleiro, jogador->pontuacao);

        /* ------ Verifica vitória ------ */
        if (jogador->posicao_tabuleiro >= TAMANHO_CAMINHO - 1) {
            vencedor = id_atual;
            break;
        }

        insereFila(&fila_jogadores, id_atual);
        turno++;

        printf("\n  [ENTER para o proximo turno...]");
        fflush(stdout);
        flush_buffer();
    }

    /* ----------------------------------------------------------
     * Tela final: tabuleiro + vitória + estatísticas + ranking
     * ---------------------------------------------------------- */
    limpar_tela();
    imprimir_tabuleiro(caminho, banco_casas, banco_jogadores, qtd_jogadores);

    printf("\n");
    imprimir_titulo("FIM DE JOGO");
    printf("\n  *** %s chegou na CHEGADA e VENCEU o jogo! ***\n",
           banco_jogadores[vencedor].nome);
    printf("  Total de turnos: %d\n\n", turno);

    /* ----------------------------------------------------------
     * 1. Estatísticas por casa (Árvore AVL, percurso em ordem)
     * ---------------------------------------------------------- */
    imprimir_titulo("ESTATISTICAS POR CASA (Arvore AVL)");
    emOrd_casas(avl_casas);
    linha_separadora('=', LARGURA_TELA);

    /* Salva o relatório de estatísticas por casa em disco (CSV). */
    salvar_estatisticas_casas(avl_casas, banco_casas);

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
