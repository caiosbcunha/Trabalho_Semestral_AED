# Documentação Técnica — Caminho do Conhecimento

Documentação completa do projeto **Caminho do Conhecimento**, um jogo de
tabuleiro educativo em C desenvolvido como trabalho semestral da disciplina de
**Algoritmos e Estruturas de Dados (AED)** do SENAI CIMATEC.

> Para uma visão rápida (regras, compilação e controles), veja o
> [`README.md`](README.md). Este documento aprofunda a **arquitetura**, os
> **módulos**, as **estruturas de dados**, os **fluxos de execução** e os
> **formatos de arquivo**.

---

## Índice

1. [Visão geral](#1-visão-geral)
2. [Estrutura de arquivos](#2-estrutura-de-arquivos)
3. [Arquitetura e dependências](#3-arquitetura-e-dependências)
4. [Estruturas de dados aplicadas](#4-estruturas-de-dados-aplicadas)
5. [Módulos em detalhe](#5-módulos-em-detalhe)
6. [Regras do jogo](#6-regras-do-jogo)
7. [Fluxo de execução](#7-fluxo-de-execução)
8. [Arquivos gerados (persistência)](#8-arquivos-gerados-persistência)
9. [As duas interfaces (terminal × gráfica)](#9-as-duas-interfaces-terminal--gráfica)
10. [Compilação e execução](#10-compilação-e-execução)
11. [Glossário de constantes](#11-glossário-de-constantes)

---

## 1. Visão geral

O jogo simula um tabuleiro linear de **45 casas**, dividido em **3 unidades
temáticas** (15 casas cada). De 2 a 4 jogadores se alternam em turnos: cada um
rola um dado (1–6), avança e sofre o efeito da casa onde parou. Casas de
**Pergunta** aplicam um quiz sobre programação em C e estruturas de dados; o
avanço e a pontuação dependem da dificuldade da pergunta. Vence quem alcançar
ou ultrapassar a última casa (Chegada).

O objetivo pedagógico do trabalho é aplicar, na prática, **quatro estruturas de
dados** clássicas, cada uma resolvendo um problema real do jogo:

| Estrutura | Papel no jogo |
|-----------|---------------|
| **Fila (FIFO)** | Ordem dos turnos entre os jogadores. |
| **Pilha (LIFO)** | Banco de perguntas embaralhado por unidade. |
| **Lista Duplamente Encadeada** | O tabuleiro navegável (frente/trás). |
| **Árvore AVL** | Estatísticas por casa com acesso O(log n). |

O projeto entrega **duas interfaces que compartilham exatamente os mesmos
módulos de lógica**: uma versão de terminal (`jogo.c`) e uma versão gráfica com
Raylib (`jogo_raylib.c`). Somente a camada de apresentação/entrada muda.

---

## 2. Estrutura de arquivos

```
Trabalho_Semestral_AED/
├── jogo.c                  # Ponto de entrada — versão TERMINAL
├── jogo_raylib.c           # Ponto de entrada — versão GRÁFICA (Raylib)
│
├── fila.h                  # Estrutura de dados: Fila circular (FIFO)
├── pilha.h                 # Estrutura de dados: Pilha (LIFO)
├── listade.h               # Estrutura de dados: Lista Duplamente Encadeada
├── arvoreAVL.h             # Estrutura de dados: Árvore AVL
│
├── jogador.h               # Domínio: Jogador + cadastro na fila de turnos
├── perguntas.h             # Domínio: Pergunta + pilhas por unidade + quiz
├── caminho.h               # Domínio: Tabuleiro (Casa) + efeitos + tabuleiro
│
├── ui.h                    # Interface de terminal (limpar tela, textos)
├── ranking.h               # Persistência: resultados.txt + ranking final
├── historico.h             # Persistência: perguntas.csv + histórico CSV
├── estatisticas.h          # Persistência: estatísticas por casa (CSV)
│
├── jogar.bat               # Compila e executa a versão terminal (Windows)
├── jogar_grafico.bat       # Compila e executa a versão gráfica (Windows)
│
├── README.md               # Visão geral do projeto
├── DOCUMENTACAO.md         # Este documento
├── .gitignore              # Ignora executáveis, output/ e arquivos gerados
│
└── output/                 # (gerado) executáveis e arquivos de saída
```

> **Nota sobre organização:** todos os módulos são *header-only* — cada `.h`
> contém tanto as declarações quanto as implementações das funções. Isso é
> simples para um trabalho acadêmico (basta compilar um único `.c`), mas
> implica que cada arquivo só pode ser incluído em **uma** unidade de compilação
> por binário (o que é o caso aqui, pois cada versão compila um único `.c`).

---

## 3. Arquitetura e dependências

O projeto separa três camadas:

- **Estruturas de dados genéricas** (`fila.h`, `pilha.h`, `listade.h`,
  `arvoreAVL.h`) — independentes do jogo, operam sobre `int`.
- **Domínio do jogo** (`jogador.h`, `perguntas.h`, `caminho.h`) — usa as
  estruturas genéricas para modelar jogadores, perguntas e tabuleiro.
- **Apresentação e persistência** (`ui.h`, `ranking.h`, `historico.h`,
  `estatisticas.h`) — interface de terminal e gravação de relatórios.

Grafo de dependências entre os cabeçalhos (`A → B` = "A inclui B"):

```
jogo.c
 ├── ui.h
 ├── jogador.h ────────► fila.h
 ├── perguntas.h ──────► pilha.h, ui.h
 ├── caminho.h ────────► listade.h, jogador.h, perguntas.h
 ├── arvoreAVL.h
 ├── ranking.h ────────► jogador.h, perguntas.h
 ├── historico.h ──────► perguntas.h, jogador.h
 └── estatisticas.h ───► arvoreAVL.h, caminho.h

jogo_raylib.c            (mesmas dependências + raylib.h)
```

As estruturas genéricas usam `typedef int tp_item;`, o que mantém o código dos
TADs limpo e reutilizável. O domínio armazena, nas estruturas, **IDs inteiros**
(ids de jogador, ids de pergunta, ids de casa) que indexam os "bancos" (arrays)
onde os dados completos ficam guardados.

---

## 4. Estruturas de dados aplicadas

### 4.1 Fila circular (FIFO) — `fila.h`

Fila implementada sobre um vetor estático circular (`MAX = 100`), com índices
`ini` e `fim`. Guarda a **ordem dos turnos**: a cada rodada, o jogador da vez é
removido da frente (`removeFila`) e, se ainda não venceu, reinserido no fim
(`insereFila`). Isso naturalmente produz o rodízio de turnos.

| Função | O(·) | Descrição |
|--------|------|-----------|
| `inicializaFila` | O(1) | `ini = fim = MAX-1` (fila vazia). |
| `filaVazia` / `filaCheia` | O(1) | Testes de contorno. |
| `insereFila` | O(1) | Insere no fim; retorna 0 se cheia. |
| `removeFila` | O(1) | Remove da frente; retorna 0 se vazia. |
| `tamanhoFila` | O(1) | Nº de elementos (trata o "wrap-around"). |
| `proximo` | O(1) | Próximo índice circular. |

### 4.2 Pilha (LIFO) — `pilha.h`

Pilha sobre vetor estático (`MAX = 100`), com `topo`. Cada **unidade temática**
tem sua pilha de IDs de pergunta. Como a pilha é preenchida com os IDs já
**embaralhados** (Fisher-Yates), retirar do topo (`pop`) equivale a sortear uma
pergunta de nível aleatório. Quando a pilha esgota, ela é reembaralhada e
reconstituída.

| Função | O(·) | Descrição |
|--------|------|-----------|
| `inicializa_pilha` | O(1) | `topo = -1`. |
| `pilha_vazia` / `pilha_cheia` | O(1) | Testes de contorno. |
| `push` | O(1) | Empilha; 0 se cheia. |
| `pop` | O(1) | Desempilha; 0 se vazia. |
| `top` | O(1) | Lê o topo sem remover. |
| `altura_pilha` | O(1) | Nº de elementos (`topo + 1`). |

### 4.3 Lista Duplamente Encadeada — `listade.h`

Lista com nós contendo `ant`, `info` e `prox`, com ponteiros para `ini` e
`fim`. Modela o **tabuleiro**: cada nó guarda o `id` de uma casa. A propriedade
"duplamente encadeada" é essencial porque o jogo precisa **andar para trás**
(casa Revés, penalidade por erro) e **para frente** (dado, Sorte, acerto).

| Função | O(·) | Descrição |
|--------|------|-----------|
| `inicializa_listad` | O(1) | Aloca a lista vazia. |
| `lista_vazia` | O(1) | Testa `ini == NULL`. |
| `insere_listad_no_fim` | O(1) | Insere no fim (monta o tabuleiro em ordem). |
| `imprime_listad` | O(n) | Percorre em ordem (1) ou inversa (2). |
| `remove_listad` | O(n) | Remove por valor, ajustando `ant`/`prox`/`fim`. |

### 4.4 Árvore AVL — `arvoreAVL.h`

Árvore binária de busca **balanceada por altura**, com rotações LL/RR/LR/RL.
Cada nó representa uma **casa visitada**, com a chave `info` (id da casa) e três
contadores: `contagem` (visitas), `acertos` e `erros`. O balanceamento garante
busca e inserção em **O(log n)** no pior caso — o requisito central do trabalho.

Campos do nó (`struct NO`):

```c
struct NO {
    struct NO *esq;
    int info;       // chave — id da casa
    int contagem;   // nº de visitas àquela casa
    int acertos;    // perguntas acertadas naquela casa
    int erros;      // perguntas erradas naquela casa
    int alt;        // altura (para o cálculo do fator de balanceamento)
    struct NO *dir;
};
```

| Função | O(·) | Descrição |
|--------|------|-----------|
| `criarAVL` | O(1) | Cria a raiz (ponteiro para ponteiro). |
| `inserir` | O(log n) | Insere a casa; **se já existe, só incrementa `contagem`** e rebalanceia. |
| `registrar_resultado_casa` | O(log n) | Busca a casa e soma +1 em `acertos` ou `erros`. |
| `obterContagem` / `consultarValorAVL` | O(log n) | Consulta binária de visitas / existência. |
| `emOrd_casas` | O(n) | Percurso em ordem imprimindo visitas/acertos/erros. |
| `remover` | O(log n) | Remove com rebalanceamento (não usado no loop principal). |
| `RotacaoLL/RR/LR/RL` | O(1) | Rotações de balanceamento. |
| `liberaABB` | O(n) | Libera toda a árvore. |

> **Detalhe de projeto:** a "visita" de uma casa é registrada com `inserir`
> **antes** de a pergunta ser respondida. Assim, quando
> `registrar_resultado_casa` é chamada, o nó com certeza já existe.

---

## 5. Módulos em detalhe

### 5.1 `jogador.h` — Jogadores e cadastro

Define a `struct Jogador` (id, nome, posição, pontuação e histórico de acertos
e erros por id de pergunta) e a função `criar_jogadores`, que:

1. pergunta a quantidade (2–4, com validação);
2. lê o nome de cada jogador;
3. zera pontuação e contadores;
4. insere o id de cada jogador na fila de turnos.

Constantes: `MAX_JOGADORES = 4`, `MIN_JOGADORES = 2`,
`MAX_HISTORICO_PERGUNTAS = 18`.

### 5.2 `perguntas.h` — Banco de perguntas e quiz

Define a `struct Pergunta` (enunciado, 4 opções, resposta correta, `nivel`
1–3, `unidade` 1–3, `tema`). Contém um **banco estático de 18 perguntas**
(6 por unidade, sendo 2 Fáceis, 2 Médias e 2 Difíceis).

Funções principais:

- `embaralhar_vetor` — **Fisher-Yates** sobre um vetor de IDs.
- `preencher_pilha_unidade` — coleta os IDs de uma unidade, embaralha e empilha
  (usada na criação **e** na reconstituição quando a pilha esgota).
- `criar_perguntas` — inicializa o banco e monta as três pilhas.
- `exibir_pergunta` / `verificar_resposta` — exibem e corrigem uma pergunta
  (aceita resposta em minúscula).
- `nivel_para_str` — converte nível numérico em `"Facil"/"Medio"/"Dificil"`.

### 5.3 `caminho.h` — Tabuleiro, efeitos e renderização

O módulo central da lógica. Define:

- `struct Casa` (`id_casa`, `tipo_casa`) e `typedef tp_listad Caminho`.
- `struct ResultadoJogada` — "pacote de saída" de uma jogada, informando se
  houve pergunta, qual foi, a resposta do jogador, se acertou, pontos ganhos e
  a casa. É consumido pelo `main` para atualizar a AVL e o histórico.

Tipos de casa: `TIPO_NORMAL(1)`, `TIPO_SORTE(2)`, `TIPO_REVES(3)`,
`TIPO_PERGUNTA(4)`. O layout fixo das 45 casas está em `criar_caminho`.

Funções principais:

- `criar_caminho` — monta a lista dupla com as 45 casas e seus tipos.
- `encontrar_no_por_id` — localiza o nó de uma casa (O(n)).
- `mover_na_lista` — anda `passos` casas para frente (>0) ou para trás (<0),
  respeitando os limites do tabuleiro.
- `rolar_dado` — retorna `rand() % 6 + 1`.
- `selecionar_pilha_por_id` / `unidade_por_id` — mapeiam a casa para a unidade.
- **`aplicar_efeito_casa`** — o coração do jogo: aplica o efeito da casa e
  devolve o `ResultadoJogada` (ver [regras](#6-regras-do-jogo)).
- `imprimir_tabuleiro` — desenha o tabuleiro no terminal (3 trilhas de 15
  casas, com os jogadores centralizados sob cada casa).

### 5.4 `ui.h` — Utilitários de terminal

Funções de apresentação sem dependência do domínio: `limpar_tela` (portável
Windows/Unix), `linha_separadora`, `imprimir_titulo`, `imprimir_texto_quebrado`
e `imprimir_opcao` (quebram texto longo sem cortar palavras nem caracteres
UTF-8) e `resumo_texto` (trunca respeitando UTF-8 e acrescenta `"..."`).
`LARGURA_TELA = 77`.

### 5.5 `ranking.h` — Resultados e ranking

- `salvar_resultados` — grava `resultados.txt` (um bloco por jogador).
- `exibir_ranking_do_disco` — **relê** o arquivo, ordena por pontuação
  (Bubble Sort, adequado para ≤ 4 jogadores) e exibe o ranking com o resumo de
  cada pergunta acertada/errada. Ler do disco (em vez de usar a memória)
  demonstra o ciclo completo de persistência exigido pelo trabalho.

### 5.6 `historico.h` — Exportação pedagógica (CSV)

Gera, com separador `;` e cabeçalho:

- `perguntas.csv` — tabela de referência das perguntas
  (`id_pergunta;unidade;tema;dificuldade;enunciado`), com id no formato `P001`.
- `historico_respostas.csv` — **uma linha por resposta** dada durante o jogo
  (modelo recomendado pelo professor). `iniciar_historico_csv` cria o arquivo
  com cabeçalho no início; `salvar_historico_resposta` acrescenta (modo `"a"`) a
  cada pergunta respondida. `csv_sanitizar` troca `;` por `,` para não corromper
  as colunas. A constante `TURMA` (`"AED-2026.1"`) identifica a turma.

### 5.7 `estatisticas.h` — Estatísticas por casa (CSV)

`salvar_estatisticas_casas` percorre a **AVL em ordem** e grava
`estatisticas_casas.csv` (`casa;unidade;tipo;visitas;acertos;erros`),
enriquecendo cada linha com a unidade e o tipo da casa. Como a chave da AVL é o
id da casa, a saída já sai ordenada por casa.

---

## 6. Regras do jogo

### 6.1 Tabuleiro

45 casas em 3 unidades de 15. Distribuição de tipos definida em
`criar_caminho`. Legenda: `I` início, `C` chegada, `.` normal, `+` sorte,
`-` revés, `?` pergunta.

### 6.2 Efeito de cada casa (`aplicar_efeito_casa`)

| Casa | Efeito |
|------|--------|
| **Normal** | Nada acontece. |
| **Sorte** | Avança `CASAS_BONUS_SORTE = 3` casas. |
| **Revés** | Volta `CASAS_PENALIDADE_REVES = 2` casas. |
| **Pergunta** | Sorteia uma pergunta da pilha da unidade e aplica o quiz. |

### 6.3 Casa de pergunta

1. Seleciona a pilha da unidade da casa. Se estiver vazia, **reembaralha e
   reconstitui** aquela unidade (`preencher_pilha_unidade`).
2. `pop` retira a pergunta do topo (nível aleatório, pois a pilha foi
   embaralhada).
3. O jogador responde A/B/C/D.

| Resultado | Pontos | Movimento |
|-----------|--------|-----------|
| **Acerto** | `nivel × 10` (Fácil 10, Médio 20, Difícil 30) | avança `nivel` casas (1/2/3) |
| **Erro** | 0 (não perde pontos) | volta `CASAS_PENALIDADE_ERRO = 1` casa |

O acerto/erro é contabilizado na AVL da casa e registrado no
`historico_respostas.csv`, e o id da pergunta entra no histórico do jogador.

### 6.4 Dado e vitória

- Dado: `rand() % 6 + 1` (1 a 6).
- Vitória: o jogador que atinge `posicao >= TAMANHO_CAMINHO - 1` (casa 44)
  vence e o jogo termina.

---

## 7. Fluxo de execução

Fluxo da versão terminal (`jogo.c`):

```
main()
 ├── srand(time)                       # semente aleatória
 ├── criar_jogadores()                 # Fila de turnos preenchida
 ├── criar_perguntas()                 # 3 pilhas embaralhadas
 ├── salvar_perguntas_csv()            # perguntas.csv
 ├── iniciar_historico_csv()           # cabeçalho de historico_respostas.csv
 ├── criar_caminho()                   # Lista dupla com 45 casas
 │
 └── LOOP (enquanto não há vencedor):
      ├── removeFila()                 # pega o jogador da vez
      ├── imprimir_tabuleiro()
      ├── rolar_dado() + mover_na_lista()
      ├── inserir(avl, nova_pos)       # registra a VISITA (O(log n))
      ├── aplicar_efeito_casa() ─► ResultadoJogada
      │     └── se pergunta:
      │           ├── registrar_resultado_casa(avl)   # acerto/erro
      │           └── salvar_historico_resposta()     # 1 linha no CSV
      ├── se venceu → sai do loop
      └── insereFila()                 # devolve o jogador ao fim da fila
 │
 ├── emOrd_casas(avl)                  # estatísticas na tela (em ordem)
 ├── salvar_estatisticas_casas(avl)    # estatisticas_casas.csv
 ├── salvar_resultados()               # resultados.txt
 ├── exibir_ranking_do_disco()         # RELÊ o disco e ordena
 └── liberaABB(avl)                    # libera a árvore
```

A versão gráfica executa a **mesma sequência lógica**, porém dirigida por uma
**máquina de estados** (ver seção 9) em vez de um loop bloqueante com `ENTER`.

---

## 8. Arquivos gerados (persistência)

Todos gerados na pasta de trabalho (via `.bat`, na pasta `output/`). Estão no
`.gitignore` por serem saídas de tempo de execução.

### `resultados.txt`
Formato textual, um bloco por jogador, terminado por `###`:
```
NOME:<nome>
PONTUACAO:<valor>
ACERTOS:<id1>,<id2>,...   (ou "-" se nenhum)
ERROS:<id1>,<id2>,...     (ou "-" se nenhum)
###
```
Lido de volta por `exibir_ranking_do_disco` para montar o ranking.

### `perguntas.csv`  (separador `;`)
```
id_pergunta;unidade;tema;dificuldade;enunciado
P001;1;Pilha;Facil;Qual das seguintes estruturas...
```

### `historico_respostas.csv`  (separador `;`, uma linha por resposta)
```
turma;id_jogador;id_pergunta;unidade;tema;dificuldade;resposta_jogador;resposta_correta;resultado;pontos
AED-2026.1;Ana;P002;1;Ponteiros;Facil;B;B;Acertou;10
```

### `estatisticas_casas.csv`  (separador `;`, AVL em ordem)
```
casa;unidade;tipo;visitas;acertos;erros
2;1;Pergunta;3;2;1
```

> Todos os CSVs usam `;` e cabeçalho na primeira linha, prontos para abrir no
> Excel, Google Sheets ou Power BI para a análise pedagógica.

---

## 9. As duas interfaces (terminal × gráfica)

Ambas incluem **os mesmos módulos de domínio e persistência**. A diferença está
apenas na apresentação e na entrada.

| Aspecto | `jogo.c` (terminal) | `jogo_raylib.c` (gráfica) |
|---------|---------------------|---------------------------|
| Loop | `while` bloqueante com `ENTER` | Máquina de estados a 60 FPS |
| Entrada | `scanf` / `getchar` | Teclado + mouse (Raylib) |
| Tabuleiro | ASCII (`imprimir_tabuleiro`) | Retângulos coloridos + peões |
| Dado | número impresso | animação com face de dado |
| Estruturas de dados | — | tela **TAB** desenha AVL, Fila e Pilhas ao vivo |
| Áudio | — | tons senoidais gerados em runtime |
| Efeito/quiz | `aplicar_efeito_casa` | `efeito_casa_simples` + `resolver_pergunta` (mesmas fórmulas) |

### Máquina de estados da versão gráfica

```
ST_MENU → ST_AJUDA
ST_MENU → ST_CADASTRO_QTD → ST_CADASTRO_NOME → ST_TURNO_ROLAR
ST_TURNO_ROLAR → ST_DADO → ST_ANDANDO → ┬─ ST_PERGUNTA ─┐
                                        └─────────────► ST_RESULTADO
ST_RESULTADO → (próximo turno) ST_TURNO_ROLAR
ST_RESULTADO → (vitória) ST_FIM → ST_ESTATISTICAS → ST_RANKING
[TAB] alterna com ST_ESTRUTURAS   |   [R] reinicia   |   [ESC] sai
```

A tela **ST_ESTRUTURAS** (tecla TAB) é o diferencial didático: desenha a
**Árvore AVL** como árvore (nós posicionados por percurso em ordem e
profundidade, coloridos por acerto/erro), a **Fila** de turnos e as três
**Pilhas** de perguntas — tudo refletindo o estado real das estruturas naquele
instante do jogo.

Controles da versão gráfica:

| Tecla | Ação |
|-------|------|
| `ESPAÇO` | rolar o dado / avançar |
| `A` `B` `C` `D` (ou clique) | responder a pergunta |
| `2` `3` `4` (ou clique) | escolher a quantidade de jogadores |
| `TAB` | abrir/fechar a tela das estruturas de dados |
| `R` | reiniciar a partida |
| `H` | regras (no menu) |
| `ESC` | sair |

---

## 10. Compilação e execução

### Versão terminal
```sh
gcc -Wall -Wextra -o jogo jogo.c
./jogo
```

### Versão gráfica (Raylib)
Requer a Raylib. No MSYS2 (UCRT64):
```sh
pacman -S mingw-w64-ucrt-x86_64-raylib
gcc -Wall -Wextra -o jogo_grafico jogo_raylib.c -lraylib -lopengl32 -lgdi32 -lwinmm -lm
./jogo_grafico
```

### No Windows (atalhos)
- `jogar.bat` — compila e executa a versão terminal em `output/`.
- `jogar_grafico.bat` — compila e executa a versão gráfica em `output/`.

Ambos os `.bat` detectam o `gcc` no PATH ou em `C:\msys64\ucrt64\bin`, criam a
pasta `output/` e rodam o jogo a partir dela (para que os arquivos gerados
fiquem organizados nessa pasta).

---

## 11. Glossário de constantes

| Constante | Arquivo | Valor | Significado |
|-----------|---------|-------|-------------|
| `MAX` | `fila.h`, `pilha.h` | 100 | Capacidade dos vetores de fila/pilha. |
| `TAMANHO_CAMINHO` | `caminho.h` | 45 | Casas do tabuleiro. |
| `CASAS_POR_LINHA` | `caminho.h` | 15 | Casas por trilha (= 1 unidade). |
| `LIMITE_U1` / `LIMITE_U2` | `caminho.h` | 15 / 30 | Fronteiras entre unidades. |
| `CASAS_BONUS_SORTE` | `caminho.h` | 3 | Avanço da casa Sorte. |
| `CASAS_PENALIDADE_REVES` | `caminho.h` | 2 | Recuo da casa Revés. |
| `CASAS_PENALIDADE_ERRO` | `caminho.h` | 1 | Recuo ao errar pergunta. |
| `PONTOS_POR_NIVEL` | `caminho.h` | 10 | Multiplicador de pontos (× nível). |
| `MAX_JOGADORES` / `MIN_JOGADORES` | `jogador.h` | 4 / 2 | Faixa de jogadores. |
| `MAX_HISTORICO_PERGUNTAS` | `jogador.h` | 18 | Histórico por jogador. |
| `QTD_PERGUNTAS` | `perguntas.h` | 18 | Total de perguntas no banco. |
| `PERGUNTAS_POR_UNIDADE` | `perguntas.h` | 6 | Perguntas por unidade. |
| `LARGURA_TELA` | `ui.h` | 77 | Largura visual do painel de terminal. |
| `TURMA` | `historico.h` | `"AED-2026.1"` | Identificador da turma no CSV. |
| `LARGURA_JANELA` / `ALTURA_JANELA` | `jogo_raylib.c` | 1100 / 720 | Tamanho da janela gráfica. |

---

*Documento gerado a partir do estado atual do repositório
(`main`, commit `45a1f72`).*
