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
} Pergunta;

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
 * criar_perguntas
 * Inicializa o banco de perguntas, separa por unidade,
 * embaralha e empilha os IDs em cada pilha correspondente.
 *
 *   pilha_u1, pilha_u2, pilha_u3 — pilhas de cada unidade
 *   banco[]                       — array de saída com todas as perguntas
 * ============================================================ */
void criar_perguntas(tp_pilha *pilha_u1, tp_pilha *pilha_u2,
                     tp_pilha *pilha_u3, Pergunta banco[]) {

    /* ---- Banco de perguntas estático ---- */
    Pergunta perguntas_estaticas[QTD_PERGUNTAS] = {
        /* --- Unidade 1 --- */
        { 0, "Qual das seguintes estruturas de dados segue a política LIFO (Last In, First Out), onde o ultimo elemento a entrar é o primeiro a sair?",
            "Fila (Queue)",
            "Pilha (Stack)",
            "Vetor (Array)",
            "Lista Encadeada",
            'B', 1, 1 },
        { 1, "Na linguagem C, qual operador é utilizado para obter o endereco de memória de uma variável comum?",
            "*",
            "&",
            "->",
            "&&",
            'B', 1, 1 },
        { 2, "Em uma Fila (Queue) padrão, em qual extremidade ocorrem as insercoes de novos elementos?",
            "No início (Front)",
            "No topo (Top)",
            "No final (Rear/Tail)",
            "Em qualquer posição aleatória",
            'C', 1, 1 },
        { 3, "Ao passar um parâmetro para uma função por referência utilizando ponteiros em C, o que acontece se alterarmos o valor apontado dentro da função?",
            "O valor da variável original fora da função permanece inalterado.",
            "O valor da variável original fora da função é modificado permanentemente.",
            "O programa gera um erro de compilação por tentar acessar memória externa.",
            "Uma nova variável global é criada automaticamente com o novo valor.",
            'B', 2, 1 },
        { 4, "Considere uma Pilha inicialmente vazia. Se executarmos as seguintes operacoes: Push(10), Push(20), Push(30), Pop(), Push(40), qual será o elemento atualmente no topo da pilha?",
            "10",
            "20",
            "30",
            "40",
            'D', 2, 1 },
        { 5, "Dado o trecho de código em C: int v[] = {5, 15, 25}; int *p = v; p++; printf(\"%d\", *p);. Qual valor será impresso no console?",
            "5",
            "15",
            "25",
            "Um endereço de memória aleatório",
            'B', 3, 1 },

        /* --- Unidade 2 --- */
        { 6, "Qual é a funcao da biblioteca stdlib.h utilizada para liberar um bloco de memória que foi alocado dinamicamente anteriormente?",
            "malloc()",
            "free()",
            "clear()",
            "delete",
            'B', 1, 2 },
        { 7, "O que diferencia estruturalmente uma Lista Duplamente Encadeada de uma Lista Simplesmente Encadeada?",
            "A lista dupla possui sempre o dobro de elementos da lista simples.",
            "A lista dupla permite armazenar apenas valores do tipo ponto flutuante.",
            "Cada nó na lista dupla possui um ponteiro para o próximo nó e outro para o nó anterior.",
            "A lista dupla não requer o uso de alocação dinâmica de memória.",
            'C', 1, 2 },
        { 8, "Na analise de complexidade algoritmica (Notação Big-O), o que significa dizer que um algoritmo é O(n)?",
            "O tempo de execução é constante, independente do tamanho da entrada.",
            "O tempo de execução cresce linearmente conforme o tamanho da entrada aumenta.",
            "O algoritmo é extremamente lento e possui crescimento exponencial.",
            "O algoritmo realiza exatamente uma operação matemática por execução.",
            'B', 2, 2 },
        { 9, "Qual é a caracteristica fundamental do algoritmo de ordenação Bubble Sort?",
            "Escolhe o menor elemento do vetor e o coloca na primeira posição.",
            "Compara pares de elementos adjacentes e os troca se estiverem fora de ordem.",
            "Divide o vetor em duas metades e as ordena de forma recursiva.",
            "Utiliza um elemento \"pivô\" para organizar os elementos menores à esquerda e maiores à direita.",
            'B', 2, 2 },
        {10, "Ao inserir um novo nó no inicio de uma Lista Simplesmente Encadeada, qual deve ser a ordem lógica das operações de ponteiros?",
            "Fazer o \"próximo\" do novo nó apontar para o atual início e então atualizar o ponteiro de início para o novo nó.",
            "Atualizar o ponteiro de início para o novo nó e depois configurar o \"próximo\" do novo nó para o antigo início.",
            "Apontar o novo nó para NULL e depois ligá-lo ao último elemento da lista.",
            "Substituir o valor do dado do primeiro nó pelo novo dado, sem criar novos ponteiros.",
            'A', 3, 2 },
        {11, "Qual é a complexidade de tempo no pior caso para o algoritmo QuickSort e em qual situacao esse cenario ocorre?",
            "O(n log n), quando o pivô escolhido é sempre o elemento central do vetor.",
            "O(n^2), quando o vetor já está ordenado e o pivô escolhido é sempre uma das extremidades.",
            "O(n^3), quando existem muitos elementos repetidos no vetor de entrada.",
            "O(n), quando o vetor é pequeno o suficiente para caber no cache do processador.",
            'B', 3, 2 },

        /* --- Unidade 3 --- */
        {12, "Qual função da biblioteca stdio.h em C é utilizada para abrir um arquivo para leitura?",
            "fopen()",
            "fclose()",
            "fread()",
            "fwrite()",
            'A', 1, 3 },
        {13, "Em uma árvore binária, qual característica define que cada nó possui no máximo dois filhos?",
            "É uma propriedade estrutural da árvore binária.",
            "É uma regra de ordenação dos elementos.",
            "Significa que a árvore é necessariamente balanceada.",
            "Indica que a árvore não pode conter valores repetidos.",
            'A', 1, 3 },
        {14, "Qual tipo de árvore balanceada ajusta automaticamente rotações para manter a altura mínima após inserções e exclusões?",
            "Árvore AVL",
            "Lista duplamente encadeada",
            "Árvore B",
            "Tabela hash",
            'A', 2, 3 },
        {15, "Em um heap binário máximo (max-heap), qual elemento está garantidamente na raiz?",
            "O menor elemento armazenado",
            "O elemento inserido mais recentemente",
            "O maior elemento armazenado",
            "O elemento do meio do conjunto",
            'C', 2, 3 },
        {16, "Qual é a principal diferença entre uma árvore de decisão digital e uma árvore genealógica não digital?",
            "A árvore de decisão é usada apenas para armazenar arquivos.",
            "A árvore de decisão organiza condições e resultados; a árvore genealógica mostra relações de parentesco.",
            "A árvore genealógica não pode ser representada como estrutura de dados.",
            "A árvore de decisão não possui nós internos.",
            'B', 3, 3 },
        {17, "Qual exemplo a seguir representa uma árvore não digital em termos de organização hierárquica?",
            "Uma planilha eletrônica",
            "Um arquivo de texto simples",
            "Um banco de dados relacional",
            "O sistema de pastas de um escritório físico",
            'D', 3, 3 },
    
    };

    /* ---- Inicializa as pilhas ---- */
    inicializa_pilha(pilha_u1);
    inicializa_pilha(pilha_u2);
    inicializa_pilha(pilha_u3);

    /* ---- Separa IDs por unidade ---- */
    int ids_u1[PERGUNTAS_POR_UNIDADE];
    int ids_u2[PERGUNTAS_POR_UNIDADE];
    int ids_u3[PERGUNTAS_POR_UNIDADE];
    int count_u1 = 0, count_u2 = 0, count_u3 = 0;

    for (int i = 0; i < QTD_PERGUNTAS; i++) {
        banco[i] = perguntas_estaticas[i];

        switch (banco[i].unidade) {
            case 1: ids_u1[count_u1++] = banco[i].id; break;
            case 2: ids_u2[count_u2++] = banco[i].id; break;
            case 3: ids_u3[count_u3++] = banco[i].id; break;
        }
    }

    /* ---- Embaralha a ordem de cada unidade ---- */
    embaralhar_vetor(ids_u1, PERGUNTAS_POR_UNIDADE);
    embaralhar_vetor(ids_u2, PERGUNTAS_POR_UNIDADE);
    embaralhar_vetor(ids_u3, PERGUNTAS_POR_UNIDADE);

    /* ---- Empilha os IDs ---- */
    for (int i = 0; i < PERGUNTAS_POR_UNIDADE; i++) {
        push(pilha_u1, ids_u1[i]);
        push(pilha_u2, ids_u2[i]);
        push(pilha_u3, ids_u3[i]);
    }

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
    printf("\n[Nivel %d] %s\n", pergunta.nivel, pergunta.texto);
    printf("  A) %s\n", pergunta.opcao_a);
    printf("  B) %s\n", pergunta.opcao_b);
    printf("  C) %s\n", pergunta.opcao_c);
    printf("  D) %s\n", pergunta.opcao_d);
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