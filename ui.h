#ifndef UI_H
#define UI_H

/* ============================================================
 * ui.h — Funcoes auxiliares de interface (terminal)
 *
 * Pequenos utilitarios usados para deixar a saida do jogo no
 * terminal mais organizada: limpar a tela entre os turnos,
 * imprimir divisorias e quebrar textos longos em varias linhas
 * sem cortar palavras (util para os enunciados das perguntas).
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Largura visual do tabuleiro/painel: 2 de margem + 15 casas x 5. */
#define LARGURA_TELA 77

/* ============================================================
 * limpar_tela
 * Limpa a tela do terminal (Windows: cls; demais: clear).
 * ============================================================ */
void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* ============================================================
 * linha_separadora
 * Imprime 'n' vezes o caractere 'c' e quebra a linha.
 * ============================================================ */
void linha_separadora(char c, int n) {
    for (int i = 0; i < n; i++) putchar(c);
    putchar('\n');
}

/* ============================================================
 * imprimir_titulo
 * Barra de titulo centralizada entre '=' ocupando LARGURA_TELA.
 * ============================================================ */
void imprimir_titulo(const char *titulo) {
    int len  = (int)strlen(titulo);
    int meio = LARGURA_TELA - len - 2;      /* espacos ao redor do texto */
    if (meio < 0) meio = 0;
    int esq = meio / 2;
    int dir = meio - esq;

    for (int i = 0; i < esq; i++) putchar('=');
    printf(" %s ", titulo);
    for (int i = 0; i < dir; i++) putchar('=');
    putchar('\n');
}

/* ============================================================
 * imprimir_texto_quebrado
 * Imprime 'txt' quebrando em linhas de no maximo 'largura'
 * colunas, sem cortar palavras, prefixando cada linha com
 * 'indent'. Acentos UTF-8 nao sao cortados pois a quebra so
 * ocorre nos espacos (ASCII).
 * ============================================================ */
void imprimir_texto_quebrado(const char *txt, int largura, const char *indent) {
    int n = (int)strlen(txt);
    int col = 0;
    int i = 0;

    printf("%s", indent);
    while (i < n) {
        /* delimita a proxima palavra (ate o proximo espaco) */
        int ini = i;
        while (i < n && txt[i] != ' ') i++;
        int tam = i - ini;

        if (col > 0 && col + 1 + tam > largura) {
            printf("\n%s", indent);   /* nao cabe: nova linha */
            col = 0;
        } else if (col > 0) {
            putchar(' ');             /* espaco entre palavras */
            col++;
        }

        for (int k = ini; k < i; k++) putchar(txt[k]);
        col += tam;

        while (i < n && txt[i] == ' ') i++; /* pula espacos */
    }
    putchar('\n');
}

/* ============================================================
 * resumo_texto
 * Copia ate 'max_bytes' de 'src' para 'dst' SEM cortar um
 * caractere UTF-8 ao meio, anexando "..." quando houver corte.
 * 'dst' deve ter espaco para max_bytes + 4 caracteres.
 * ============================================================ */
void resumo_texto(const char *src, char *dst, int max_bytes) {
    int cut = 0;
    while (src[cut] != '\0' && cut < max_bytes) {
        unsigned char c = (unsigned char)src[cut];
        int largura = 1;                 /* bytes do caractere UTF-8 */
        if      (c >= 0xF0) largura = 4;
        else if (c >= 0xE0) largura = 3;
        else if (c >= 0xC0) largura = 2;
        if (cut + largura > max_bytes) break;  /* nao cabe inteiro */
        cut += largura;
    }
    memcpy(dst, src, cut);
    dst[cut] = '\0';
    if (src[cut] != '\0') strcat(dst, "..."); /* houve corte */
}

/* ============================================================
 * imprimir_opcao
 * Imprime uma alternativa no formato "    X) <texto>", quebrando
 * o texto em varias linhas com recuo alinhado sob o texto da
 * primeira linha (recuo de 7 espacos = tamanho de "    X) ").
 * ============================================================ */
void imprimir_opcao(char letra, const char *txt) {
    const char *recuo = "       "; /* 7 espacos */
    int largura = 62;              /* colunas uteis por linha */
    int n   = (int)strlen(txt);
    int col = 7;                   /* "    X) " ja ocupou 7 colunas */
    int i   = 0;

    printf("    %c) ", letra);
    while (i < n) {
        int ini = i;
        while (i < n && txt[i] != ' ') i++;
        int tam = i - ini;

        if (col > 7 && col + 1 + tam > largura) {
            printf("\n%s", recuo);
            col = 7;
        } else if (col > 7) {
            putchar(' ');
            col++;
        }

        for (int k = ini; k < i; k++) putchar(txt[k]);
        col += tam;

        while (i < n && txt[i] == ' ') i++;
    }
    putchar('\n');
}

#endif /* UI_H */
