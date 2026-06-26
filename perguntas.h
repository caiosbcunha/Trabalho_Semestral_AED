#ifndef PERGUNTAS_H
#define PERGUNTAS_H

/* ============================================================
 * perguntas.h — Módulo de Perguntas para o Quiz
 * Depende: pilha.h
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "pilha.h"
#include "ui.h"

/* ----------------------------------------------------------
 * Constantes
 * ---------------------------------------------------------- */
#define QTD_PERGUNTAS        18   /* Total de perguntas no banco */
#define PERGUNTAS_POR_UNIDADE 6   /* Perguntas por unidade temática */

/* ----------------------------------------------------------
 * Estrutura de Dados: Pergunta
 * ---------------------------------------------------------- */
typedef struct {
    int  id;
    char texto[256];
    char opcao_a[256];
    char opcao_b[256];
    char opcao_c[256];
    char opcao_d[256];
    char resposta_correta; /* 'A', 'B', 'C' ou 'D' */
    int  nivel;            /* 1 = Fácil | 2 = Médio | 3 = Difícil */
    int  unidade;          /* 1, 2 ou 3 */
    char tema[40];         /* tema padronizado (p/ análise pedagógica) */
} Pergunta;

/* ============================================================
 * nivel_para_str
 * Converte o nível numérico no rótulo textual usado no CSV.
 *   1 → "Facil" | 2 → "Medio" | 3 → "Dificil"
 * (sem acentos, conforme orientação do modelo do professor)
 * ============================================================ */
const char *nivel_para_str(int nivel) {
    switch (nivel) {
        case 1:  return "Facil";
        case 2:  return "Medio";
        case 3:  return "Dificil";
        default: return "Desconhecido";
    }
}

/* ============================================================
 * embaralhar_vetor
 * Embaralha um array de inteiros usando o algoritmo Fisher-Yates.
 *   arr  — ponteiro para o array
 *   n    — número de elementos
 * ============================================================ */
void embaralhar_vetor(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j    = rand() % (i + 1);
        int temp = arr[i];
        arr[i]   = arr[j];
        arr[j]   = temp;
    }
}

/* ============================================================
 * preencher_pilha_unidade
 * (Re)inicializa a pilha de uma unidade: coleta os IDs das
 * perguntas daquela unidade, embaralha (Fisher-Yates) e empilha.
 *
 * Usada tanto na criação inicial quanto na REORGANIZAÇÃO do
 * conjunto quando ele se esgota durante o jogo (requisito:
 * "quando o conjunto se esgotar, embaralhar e reconstituir").
 * ============================================================ */
void preencher_pilha_unidade(tp_pilha *pilha, Pergunta banco[], int unidade) {
    int ids[PERGUNTAS_POR_UNIDADE];
    int count = 0;

    for (int i = 0; i < QTD_PERGUNTAS; i++)
        if (banco[i].unidade == unidade)
            ids[count++] = banco[i].id;

    embaralhar_vetor(ids, count);

    inicializa_pilha(pilha);
    for (int i = 0; i < count; i++)
        push(pilha, ids[i]);
}

/* ============================================================
 * criar_perguntas
 * Inicializa o banco de perguntas, separa por unidade,
 * embaralha e empilha os IDs em cada pilha correspondente.
 *
 *   pilha_u1, pilha_u2, pilha_u3 — pilhas de cada unidade
 *   banco[]                       — array de saída com todas as perguntas
 * ============================================================ */
void criar_perguntas(tp_pilha *pilha_u1, tp_pilha *pilha_u2,
                     tp_pilha *pilha_u3, Pergunta banco[]) {

    /* ---- Banco de perguntas estático ----
     * Cada unidade tem exatamente 6 perguntas, sendo 2 de cada
     * dificuldade (2 Faceis, 2 Medias, 2 Dificeis), conforme o
     * requisito. O campo 'tema' usa nomes padronizados para a
     * análise pedagógica posterior em planilha.
     */
    Pergunta perguntas_estaticas[QTD_PERGUNTAS] = {
        /* --- Unidade 1 (Facil x2, Medio x2, Dificil x2) --- */
        { 0, "Qual das seguintes estruturas de dados segue a politica LIFO (Last In, First Out), onde o ultimo elemento a entrar e o primeiro a sair?",
            "Fila (Queue)",
            "Pilha (Stack)",
            "Vetor (Array)",
            "Lista Encadeada",
            'B', 1, 1, "Pilha" },
        { 1, "Na linguagem C, qual operador e utilizado para obter o endereco de memoria de uma variavel comum?",
            "*",
            "&",
            "->",
            "&&",
            'B', 1, 1, "Ponteiros" },
        { 2, "Em uma fila circular implementada sobre um vetor de tamanho MAX (com indices 'ini' e 'fim'), qual condicao normalmente indica que a fila esta CHEIA?",
            "ini == fim",
            "(fim + 1) % MAX == ini",
            "fim == MAX",
            "ini == 0 e fim == MAX - 1",
            'B', 3, 1, "Fila" },
        { 3, "Ao passar um parametro para uma funcao por referencia utilizando ponteiros em C, o que acontece se alterarmos o valor apontado dentro da funcao?",
            "O valor da variavel original fora da funcao permanece inalterado.",
            "O valor da variavel original fora da funcao e modificado permanentemente.",
            "O programa gera um erro de compilacao por tentar acessar memoria externa.",
            "Uma nova variavel global e criada automaticamente com o novo valor.",
            'B', 2, 1, "Ponteiros" },
        { 4, "Considere uma Pilha inicialmente vazia. Se executarmos as seguintes operacoes: Push(10), Push(20), Push(30), Pop(), Push(40), qual sera o elemento atualmente no topo da pilha?",
            "10",
            "20",
            "30",
            "40",
            'D', 2, 1, "Pilha" },
        { 5, "Dado o trecho de codigo em C: int v[] = {5, 15, 25}; int *p = v; p++; printf(\"%d\", *p);. Qual valor sera impresso no console?",
            "5",
            "15",
            "25",
            "Um endereco de memoria aleatorio",
            'B', 3, 1, "Ponteiros" },

        /* --- Unidade 2 (Facil x2, Medio x2, Dificil x2) --- */
        { 6, "Qual e a funcao da biblioteca stdlib.h utilizada para liberar um bloco de memoria que foi alocado dinamicamente anteriormente?",
            "malloc()",
            "free()",
            "clear()",
            "delete",
            'B', 1, 2, "Alocacao Dinamica" },
        { 7, "O que diferencia estruturalmente uma Lista Duplamente Encadeada de uma Lista Simplesmente Encadeada?",
            "A lista dupla possui sempre o dobro de elementos da lista simples.",
            "A lista dupla permite armazenar apenas valores do tipo ponto flutuante.",
            "Cada no na lista dupla possui um ponteiro para o proximo no e outro para o no anterior.",
            "A lista dupla nao requer o uso de alocacao dinamica de memoria.",
            'C', 1, 2, "Lista Duplamente Encadeada" },
        { 8, "Na analise de complexidade algoritmica (Notacao Big-O), o que significa dizer que um algoritmo e O(n)?",
            "O tempo de execucao e constante, independente do tamanho da entrada.",
            "O tempo de execucao cresce linearmente conforme o tamanho da entrada aumenta.",
            "O algoritmo e extremamente lento e possui crescimento exponencial.",
            "O algoritmo realiza exatamente uma operacao matematica por execucao.",
            'B', 2, 2, "Complexidade" },
        { 9, "Qual e a caracteristica fundamental do algoritmo de ordenacao Bubble Sort?",
            "Escolhe o menor elemento do vetor e o coloca na primeira posicao.",
            "Compara pares de elementos adjacentes e os troca se estiverem fora de ordem.",
            "Divide o vetor em duas metades e as ordena de forma recursiva.",
            "Utiliza um elemento \"pivo\" para organizar os elementos menores a esquerda e maiores a direita.",
            'B', 2, 2, "Ordenacao" },
        {10, "Ao inserir um novo no no inicio de uma Lista Simplesmente Encadeada, qual deve ser a ordem logica das operacoes de ponteiros?",
            "Fazer o \"proximo\" do novo no apontar para o atual inicio e entao atualizar o ponteiro de inicio para o novo no.",
            "Atualizar o ponteiro de inicio para o novo no e depois configurar o \"proximo\" do novo no para o antigo inicio.",
            "Apontar o novo no para NULL e depois liga-lo ao ultimo elemento da lista.",
            "Substituir o valor do dado do primeiro no pelo novo dado, sem criar novos ponteiros.",
            'A', 3, 2, "Lista Encadeada" },
        {11, "Qual e a complexidade de tempo no pior caso para o algoritmo QuickSort e em qual situacao esse cenario ocorre?",
            "O(n log n), quando o pivo escolhido e sempre o elemento central do vetor.",
            "O(n^2), quando o vetor ja esta ordenado e o pivo escolhido e sempre uma das extremidades.",
            "O(n^3), quando existem muitos elementos repetidos no vetor de entrada.",
            "O(n), quando o vetor e pequeno o suficiente para caber no cache do processador.",
            'B', 3, 2, "Ordenacao" },

        /* --- Unidade 3 (Facil x2, Medio x2, Dificil x2) --- */
        {12, "Qual funcao da biblioteca stdio.h em C e utilizada para abrir um arquivo para leitura?",
            "fopen()",
            "fclose()",
            "fread()",
            "fwrite()",
            'A', 1, 3, "Arquivos" },
        {13, "Em uma arvore binaria, qual caracteristica define que cada no possui no maximo dois filhos?",
            "E uma propriedade estrutural da arvore binaria.",
            "E uma regra de ordenacao dos elementos.",
            "Significa que a arvore e necessariamente balanceada.",
            "Indica que a arvore nao pode conter valores repetidos.",
            'A', 1, 3, "Arvore Binaria" },
        {14, "Qual tipo de arvore balanceada ajusta automaticamente rotacoes para manter a altura minima apos insercoes e exclusoes?",
            "Arvore AVL",
            "Lista duplamente encadeada",
            "Arvore B",
            "Tabela hash",
            'A', 2, 3, "Arvore AVL" },
        {15, "Em um heap binario maximo (max-heap), qual elemento esta garantidamente na raiz?",
            "O menor elemento armazenado",
            "O elemento inserido mais recentemente",
            "O maior elemento armazenado",
            "O elemento do meio do conjunto",
            'C', 2, 3, "Arvore Binaria" },
        {16, "Qual e a principal diferenca entre uma arvore de decisao digital e uma arvore genealogica nao digital?",
            "A arvore de decisao e usada apenas para armazenar arquivos.",
            "A arvore de decisao organiza condicoes e resultados; a arvore genealogica mostra relacoes de parentesco.",
            "A arvore genealogica nao pode ser representada como estrutura de dados.",
            "A arvore de decisao nao possui nos internos.",
            'B', 3, 3, "Arvore Binaria" },
        {17, "Qual exemplo a seguir representa uma arvore nao digital em termos de organizacao hierarquica?",
            "Uma planilha eletronica",
            "Um arquivo de texto simples",
            "Um banco de dados relacional",
            "O sistema de pastas de um escritorio fisico",
            'D', 3, 3, "Arvore Binaria" },

    };

    /* ---- Copia o banco estático para o banco de saída ---- */
    for (int i = 0; i < QTD_PERGUNTAS; i++)
        banco[i] = perguntas_estaticas[i];

    /* ---- Monta uma pilha embaralhada por unidade ---- */
    preencher_pilha_unidade(pilha_u1, banco, 1);
    preencher_pilha_unidade(pilha_u2, banco, 2);
    preencher_pilha_unidade(pilha_u3, banco, 3);

    printf("-> Perguntas criadas, embaralhadas e incluidas nas Pilhas!\n\n");
}

/* ============================================================
 * exibir_perguntas_embaralhadas
 * Percorre as pilhas sem esvaziá-las e imprime todas as
 * perguntas com suas opções e resposta correta.
 * ============================================================ */
void exibir_perguntas_embaralhadas(tp_pilha *pilha_u1, tp_pilha *pilha_u2,
                                   tp_pilha *pilha_u3, Pergunta banco[]) {

    const char *labels[] = { "Unidade 1", "Unidade 2", "Unidade 3" };
    tp_pilha   *pilhas[] = { pilha_u1,    pilha_u2,    pilha_u3    };

    for (int u = 0; u < 3; u++) {
        printf("\n--- Perguntas Embaralhadas (%s) ---\n", labels[u]);

        for (int i = pilhas[u]->topo; i >= 0; i--) {
            int id = pilhas[u]->item[i];

            printf("  [Nivel %d] %s\n",  banco[id].nivel, banco[id].texto);
            printf("    A) %s\n",         banco[id].opcao_a);
            printf("    B) %s\n",         banco[id].opcao_b);
            printf("    C) %s\n",         banco[id].opcao_c);
            printf("    D) %s\n",         banco[id].opcao_d);
            printf("    Resposta: %c\n\n",banco[id].resposta_correta);
        }
    }

    printf("-------------------------------------------\n");
}

/* ============================================================
 * exibir_pergunta
 * Exibe uma pergunta individual com suas quatro opções.
 * ============================================================ */
void exibir_pergunta(Pergunta pergunta) {
    printf("\n");
    imprimir_texto_quebrado(pergunta.texto, 70, "  ");
    printf("\n");
    imprimir_opcao('A', pergunta.opcao_a);
    imprimir_opcao('B', pergunta.opcao_b);
    imprimir_opcao('C', pergunta.opcao_c);
    imprimir_opcao('D', pergunta.opcao_d);
}

/* ============================================================
 * verificar_resposta
 * Retorna 1 se a resposta do jogador estiver correta, 0 caso contrário.
 * Aceita letras minúsculas ('a'–'d') automaticamente.
 * ============================================================ */
int verificar_resposta(Pergunta pergunta, char resposta_jogador) {
    char upper = (resposta_jogador >= 'a' && resposta_jogador <= 'd')
                 ? resposta_jogador - 32
                 : resposta_jogador;

    return (upper == pergunta.resposta_correta);
}

#endif /* PERGUNTAS_H */