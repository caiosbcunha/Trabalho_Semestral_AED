#ifndef HISTORICO_H
#define HISTORICO_H

/* ============================================================
 * historico.h — Exportacao pedagogica em CSV
 *
 * Gera dois arquivos, ambos com ';' como separador e cabecalho
 * na primeira linha (prontos para Excel / Google Sheets / Power BI):
 *
 *   1) perguntas.csv          — tabela de referencio a das perguntas
 *      Cabecalho: id_pergunta;unidade;tema;dificuldade;enunciado
 *
 *   2) historico_respostas.csv — UMA linha por pergunta respondida
 *      Cabecalho:
 *      turma;id_jogador;id_pergunta;unidade;tema;dificuldade;
 *      resposta_jogador;resposta_correta;resultado;pontos
 *
 * Depende: perguntas.h (Pergunta, nivel_para_str), jogador.h.
 * ============================================================ */

#include <stdio.h>
#include <string.h>
#include "perguntas.h"
#include "jogador.h"

/* Turma/semestre — ajuste conforme a sua turma. */
#define TURMA                  "AED-2026.1"

#define ARQUIVO_PERGUNTAS_CSV  "perguntas.csv"
#define ARQUIVO_HISTORICO_CSV  "historico_respostas.csv"

/* ============================================================
 * csv_sanitizar
 * Copia 'src' para 'dst' (limite n) trocando ';' por ',' e
 * quebras de linha por espaco, para nao corromper as colunas
 * do CSV (alguns enunciados contem ';').
 * ============================================================ */
void csv_sanitizar(const char *src, char *dst, size_t n) {
    size_t i = 0;
    for (; src[i] != '\0' && i < n - 1; i++) {
        char c = src[i];
        if (c == ';')                 dst[i] = ',';
        else if (c == '\n' || c == '\r') dst[i] = ' ';
        else                          dst[i] = c;
    }
    dst[i] = '\0';
}

/* ============================================================
 * salvar_perguntas_csv
 * Gera a tabela de referencia com todas as perguntas do banco.
 * O id_pergunta segue o padrao "P001", "P002", ... (id + 1).
 * ============================================================ */
void salvar_perguntas_csv(Pergunta banco[], int qtd) {
    FILE *f = fopen(ARQUIVO_PERGUNTAS_CSV, "w");
    if (f == NULL) {
        printf("\n[!] Erro ao abrir '%s' para escrita.\n", ARQUIVO_PERGUNTAS_CSV);
        return;
    }

    fprintf(f, "id_pergunta;unidade;tema;dificuldade;enunciado\n");

    char enunciado[256];
    for (int i = 0; i < qtd; i++) {
        csv_sanitizar(banco[i].texto, enunciado, sizeof(enunciado));
        fprintf(f, "P%03d;%d;%s;%s;%s\n",
                banco[i].id + 1,
                banco[i].unidade,
                banco[i].tema,
                nivel_para_str(banco[i].nivel),
                enunciado);
    }

    fclose(f);
    printf("-> Tabela de perguntas exportada em '%s'!\n", ARQUIVO_PERGUNTAS_CSV);
}

/* ============================================================
 * iniciar_historico_csv
 * (Re)cria o arquivo de historico e escreve o cabecalho.
 * Deve ser chamado UMA vez, no inicio da partida.
 * ============================================================ */
void iniciar_historico_csv(void) {
    FILE *f = fopen(ARQUIVO_HISTORICO_CSV, "w");
    if (f == NULL) {
        printf("\n[!] Erro ao abrir '%s' para escrita.\n", ARQUIVO_HISTORICO_CSV);
        return;
    }
    fprintf(f,
        "turma;id_jogador;id_pergunta;unidade;tema;dificuldade;"
        "resposta_jogador;resposta_correta;resultado;pontos\n");
    fclose(f);
    printf("-> Historico de respostas iniciado em '%s'!\n\n", ARQUIVO_HISTORICO_CSV);
}

/* ============================================================
 * salvar_historico_resposta
 * Acrescenta (modo "a") UMA linha por pergunta respondida,
 * conforme o modelo do professor. Chamada a cada resposta.
 *
 *   jogador        — quem respondeu
 *   p              — a pergunta respondida
 *   resposta_jog   — alternativa escolhida (ja em maiuscula)
 *   acertou        — 1 = acertou | 0 = errou
 *   pontos         — pontos ganhos nesta resposta
 * ============================================================ */
void salvar_historico_resposta(Jogador *jogador, Pergunta *p,
                               char resposta_jog, int acertou, int pontos) {
    FILE *f = fopen(ARQUIVO_HISTORICO_CSV, "a");
    if (f == NULL) {
        printf("\n[!] Erro ao abrir '%s' para escrita.\n", ARQUIVO_HISTORICO_CSV);
        return;
    }

    char nome[50];
    csv_sanitizar(jogador->nome, nome, sizeof(nome));

    fprintf(f, "%s;%s;P%03d;%d;%s;%s;%c;%c;%s;%d\n",
            TURMA,
            nome,
            p->id + 1,
            p->unidade,
            p->tema,
            nivel_para_str(p->nivel),
            resposta_jog,
            p->resposta_correta,
            acertou ? "Acertou" : "Errou",
            pontos);

    fclose(f);
}

#endif /* HISTORICO_H */
