#ifndef ESTATISTICAS_H
#define ESTATISTICAS_H

/* ============================================================
 * estatisticas.h — Persistencia das estatisticas por casa
 *
 * Atende ao requisito "Registro de Estatisticas":
 * armazena em disco, ao final do jogo, tudo o que aconteceu em
 * cada posicao do tabuleiro (visitas, acertos e erros), lendo a
 * Arvore AVL em ordem crescente de casa.
 *
 * Gera 'estatisticas_casas.csv' (separador ';'), enriquecido com
 * a unidade e o tipo de cada casa:
 *   casa;unidade;tipo;visitas;acertos;erros
 *
 * Depende: arvoreAVL.h (struct NO), caminho.h (Casa, helpers).
 * ============================================================ */

#include <stdio.h>
#include "arvoreAVL.h"
#include "caminho.h"

#define ARQUIVO_ESTATISTICAS_CSV "estatisticas_casas.csv"

/* ------------------------------------------------------------
 * escrever_casas_emordem
 * Percorrimento em-ordem (esq, raiz, dir) da AVL, escrevendo uma
 * linha CSV por casa. Como a chave e o id da casa, sai ordenado.
 * ------------------------------------------------------------ */
void escrever_casas_emordem(struct NO *no, Casa banco_c[], FILE *f) {
    if (no == NULL) return;
    escrever_casas_emordem(no->esq, banco_c, f);

    int id = no->info;
    fprintf(f, "%d;%d;%s;%d;%d;%d\n",
            id,
            unidade_por_id(id),
            tipo_casa_str(banco_c[id].tipo_casa),
            no->contagem,
            no->acertos,
            no->erros);

    escrever_casas_emordem(no->dir, banco_c, f);
}

/* ============================================================
 * salvar_estatisticas_casas
 * Grava o relatorio de estatisticas por casa em disco.
 * ============================================================ */
void salvar_estatisticas_casas(ArvAVL *raiz, Casa banco_c[]) {
    FILE *f = fopen(ARQUIVO_ESTATISTICAS_CSV, "w");
    if (f == NULL) {
        printf("\n[!] Erro ao abrir '%s' para escrita.\n",
               ARQUIVO_ESTATISTICAS_CSV);
        return;
    }

    fprintf(f, "casa;unidade;tipo;visitas;acertos;erros\n");

    if (raiz != NULL && *raiz != NULL)
        escrever_casas_emordem(*raiz, banco_c, f);

    fclose(f);
    printf("-> Estatisticas por casa salvas em '%s'!\n",
           ARQUIVO_ESTATISTICAS_CSV);
}

#endif /* ESTATISTICAS_H */
