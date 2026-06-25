#ifndef RANKING_H
#define RANKING_H

/* ============================================================
 * ranking.h — Persistência em Disco e Exibição do Ranking
 *
 * Formato do arquivo (resultados.txt):
 *   NOME:<nome>
 *   PONTUACAO:<valor>
 *   ACERTOS:<id1>,<id2>,...   (ou "-" se nenhum)
 *   ERROS:<id1>,<id2>,...     (ou "-" se nenhum)
 *   ###
 *   ... (um bloco por jogador)
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jogador.h"
#include "perguntas.h"

#define ARQUIVO_RESULTADOS "resultados.txt"

/* ----------------------------------------------------------
 * EntradaRanking
 * Representa os dados de um jogador lidos do arquivo em disco.
 * ---------------------------------------------------------- */
typedef struct {
    char nome[50];
    int  pontuacao;
    int  acertos[QTD_PERGUNTAS];
    int  qtd_acertos;
    int  erros[QTD_PERGUNTAS];
    int  qtd_erros;
} EntradaRanking;

/* ============================================================
 * salvar_resultados
 * Grava nome, pontuação e IDs das perguntas certas/erradas
 * de cada jogador em ARQUIVO_RESULTADOS.
 * ============================================================ */
void salvar_resultados(Jogador jogadores[], int qtd) {

    FILE *f = fopen(ARQUIVO_RESULTADOS, "w");
    if (f == NULL) {
        printf("\n[!] Erro ao abrir '%s' para escrita.\n", ARQUIVO_RESULTADOS);
        return;
    }

    for (int i = 0; i < qtd; i++) {
        fprintf(f, "NOME:%s\n",      jogadores[i].nome);
        fprintf(f, "PONTUACAO:%d\n", jogadores[i].pontuacao);

        /* --- Acertos --- */
        if (jogadores[i].qtd_acertadas == 0) {
            fprintf(f, "ACERTOS:-\n");
        } else {
            fprintf(f, "ACERTOS:");
            for (int j = 0; j < jogadores[i].qtd_acertadas; j++) {
                if (j > 0) fprintf(f, ",");
                fprintf(f, "%d", jogadores[i].perguntas_acertadas[j]);
            }
            fprintf(f, "\n");
        }

        /* --- Erros --- */
        if (jogadores[i].qtd_erradas == 0) {
            fprintf(f, "ERROS:-\n");
        } else {
            fprintf(f, "ERROS:");
            for (int j = 0; j < jogadores[i].qtd_erradas; j++) {
                if (j > 0) fprintf(f, ",");
                fprintf(f, "%d", jogadores[i].perguntas_erradas[j]);
            }
            fprintf(f, "\n");
        }

        fprintf(f, "###\n");
    }

    fclose(f);
    printf("\n-> Resultados salvos em '%s'!\n", ARQUIVO_RESULTADOS);
}

/* ============================================================
 * exibir_ranking_do_disco
 * Lê ARQUIVO_RESULTADOS, ordena por pontuação (maior → menor)
 * e exibe o ranking com detalhes de cada pergunta.
 * ============================================================ */
void exibir_ranking_do_disco(Pergunta banco_perguntas[]) {

    FILE *f = fopen(ARQUIVO_RESULTADOS, "r");
    if (f == NULL) {
        printf("\n[!] Arquivo '%s' nao encontrado.\n", ARQUIVO_RESULTADOS);
        return;
    }

    EntradaRanking entradas[MAX_JOGADORES];
    int qtd = 0;
    char linha[600];
    char tmp[512];
    char *tok;
    int  count;

    /* Inicializa a primeira entrada */
    entradas[0].qtd_acertos = 0;
    entradas[0].qtd_erros   = 0;

    while (fgets(linha, sizeof(linha), f) && qtd < MAX_JOGADORES) {

        linha[strcspn(linha, "\n")] = '\0'; /* remove '\n' do final */

        if (strncmp(linha, "NOME:", 5) == 0) {
            strncpy(entradas[qtd].nome, linha + 5, 49);
            entradas[qtd].nome[49]    = '\0';
            entradas[qtd].pontuacao   = 0;
            entradas[qtd].qtd_acertos = 0;
            entradas[qtd].qtd_erros   = 0;

        } else if (strncmp(linha, "PONTUACAO:", 10) == 0) {
            entradas[qtd].pontuacao = atoi(linha + 10);

        } else if (strncmp(linha, "ACERTOS:", 8) == 0) {
            strncpy(tmp, linha + 8, 511);
            tmp[511] = '\0';
            count = 0;
            if (strcmp(tmp, "-") != 0) {
                tok = strtok(tmp, ",");
                while (tok != NULL && count < QTD_PERGUNTAS) {
                    entradas[qtd].acertos[count++] = atoi(tok);
                    tok = strtok(NULL, ",");
                }
            }
            entradas[qtd].qtd_acertos = count;

        } else if (strncmp(linha, "ERROS:", 6) == 0) {
            strncpy(tmp, linha + 6, 511);
            tmp[511] = '\0';
            count = 0;
            if (strcmp(tmp, "-") != 0) {
                tok = strtok(tmp, ",");
                while (tok != NULL && count < QTD_PERGUNTAS) {
                    entradas[qtd].erros[count++] = atoi(tok);
                    tok = strtok(NULL, ",");
                }
            }
            entradas[qtd].qtd_erros = count;

        } else if (strcmp(linha, "###") == 0) {
            qtd++;
            if (qtd < MAX_JOGADORES) {
                entradas[qtd].qtd_acertos = 0;
                entradas[qtd].qtd_erros   = 0;
            }
        }
    }
    fclose(f);

    if (qtd == 0) {
        printf("\n[!] Nenhum jogador encontrado no arquivo.\n");
        return;
    }

    /* --- Ordenação por pontuação: Bubble Sort (max 4 jogadores) --- */
    EntradaRanking tmp_swap;
    for (int i = 0; i < qtd - 1; i++) {
        for (int j = 0; j < qtd - 1 - i; j++) {
            if (entradas[j].pontuacao < entradas[j + 1].pontuacao) {
                tmp_swap      = entradas[j];
                entradas[j]   = entradas[j + 1];
                entradas[j+1] = tmp_swap;
            }
        }
    }

    /* --- Exibição --- */
    printf("\n=========================================\n");
    printf("           RANKING FINAL                 \n");
    printf("   (dados lidos de '%s')\n", ARQUIVO_RESULTADOS);
    printf("=========================================\n");

    for (int i = 0; i < qtd; i++) {
        printf("\n  %d.  %s\n", i + 1, entradas[i].nome);
        printf("      Pontuacao: %d pt(s)\n", entradas[i].pontuacao);

        if (entradas[i].qtd_acertos == 0) {
            printf("      Acertos  : nenhum\n");
        } else {
            printf("      Acertos (%d):\n", entradas[i].qtd_acertos);
            for (int j = 0; j < entradas[i].qtd_acertos; j++) {
                int id = entradas[i].acertos[j];
                char texto_curto[64];
                strncpy(texto_curto, banco_perguntas[id].texto, 60);
                texto_curto[60] = '\0';
                if (strlen(banco_perguntas[id].texto) > 60)
                    strcat(texto_curto, "...");
                printf("        [OK] (id %2d) %s\n", id, texto_curto);
            }
        }

        if (entradas[i].qtd_erros == 0) {
            printf("      Erros    : nenhum\n");
        } else {
            printf("      Erros   (%d):\n", entradas[i].qtd_erros);
            for (int j = 0; j < entradas[i].qtd_erros; j++) {
                int id = entradas[i].erros[j];
                char texto_curto[64];
                strncpy(texto_curto, banco_perguntas[id].texto, 60);
                texto_curto[60] = '\0';
                if (strlen(banco_perguntas[id].texto) > 60)
                    strcat(texto_curto, "...");
                printf("        [ X] (id %2d) %s  (Correta: %c)\n",
                       id, texto_curto, banco_perguntas[id].resposta_correta);
            }
        }
    }

    printf("\n=========================================\n");
}

#endif /* RANKING_H */
