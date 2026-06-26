# Caminho do Conhecimento

Jogo de tabuleiro educativo em C para 2 a 4 jogadores, desenvolvido como
projeto da disciplina de Algoritmos e Estruturas de Dados do SENAI CIMATEC.
Os jogadores percorrem 45 casas respondendo perguntas sobre programação em C
e estruturas de dados, com casas de Sorte, Revés e Pergunta. O projeto aplica
na prática Fila (controle de turnos), Pilha (banco de perguntas por unidade
temática), Lista Duplamente Encadeada (tabuleiro navegável) e Árvore AVL
(estatísticas por casa com acesso logarítmico).

O jogo tem duas interfaces que **compartilham a mesma lógica e os mesmos módulos**:

- **`jogo.c`** — versão original em terminal (texto).
- **`jogo_raylib.c`** — versão gráfica com a biblioteca [Raylib]. Troca apenas a
  camada de janela, desenho e entrada; toda a lógica de estruturas de dados é a mesma.

## Estruturas de dados aplicadas

| Estrutura | Arquivo | Uso no jogo |
|-----------|---------|-------------|
| Fila (FIFO) | `fila.h` / `jogador.h` | Ordem das rodadas: o jogador da vez é removido da frente e reinserido no fim. |
| Pilha (LIFO) | `pilha.h` / `perguntas.h` | Um conjunto de perguntas embaralhado por unidade; retira-se sempre do topo. |
| Lista Dupla. Encadeada | `listade.h` / `caminho.h` | Tabuleiro de 45 casas, navegável para frente e para trás. |
| Árvore AVL | `arvoreAVL.h` | Estatísticas por casa (visitas, acertos, erros) em O(log n). |

Na versão gráfica, a tecla **TAB** abre uma tela que desenha essas estruturas
ao vivo: a Árvore AVL como árvore (nós e ramos), a Fila de turnos e as Pilhas
de perguntas por unidade.

## Regras principais

- **Tabuleiro:** 45 casas (15 por unidade), tipos Normal, Sorte, Revés e Pergunta.
- **Dado:** valor aleatório de 1 a 6 por jogada.
- **Perguntas:** 18 no total (6 por unidade, sendo 2 Fáceis, 2 Médias e 2 Difíceis).
  O nível é sorteado (pilha embaralhada). Quando a pilha de uma unidade esgota,
  ela é **reembaralhada e reconstituída** automaticamente.
- **Avanço por acerto conforme o nível:** Fácil = 1, Média = 2, Difícil = 3 casas
  (e +10/+20/+30 pontos). Erro faz voltar 1 casa.
- **Fim de jogo:** vence quem alcança/ultrapassa a última casa.

## Relatórios gerados em disco

Ao final da partida o jogo grava (separador `;`, prontos para Excel/Sheets/Power BI):

- `perguntas.csv` — tabela de referência das perguntas (`id_pergunta;unidade;tema;dificuldade;enunciado`).
- `historico_respostas.csv` — uma linha por pergunta respondida, no modelo recomendado pelo professor.
- `estatisticas_casas.csv` — visitas, acertos e erros por casa (lidos da Árvore AVL, em ordem).
- `resultados.txt` — placar e histórico por jogador, usado para montar o ranking final.

## Compilação e execução

Cada versão é compilada a partir de um único arquivo, que inclui todos os módulos.

### Versão terminal

```sh
gcc -Wall -Wextra -o jogo jogo.c
./jogo
```

### Versão gráfica (Raylib)

Requer a Raylib instalada. No MSYS2 (UCRT64):

```sh
pacman -S mingw-w64-ucrt-x86_64-raylib
gcc -Wall -Wextra -o jogo_grafico jogo_raylib.c -lraylib -lopengl32 -lgdi32 -lwinmm -lm
./jogo_grafico
```

No Windows, o atalho `jogar_grafico.bat` compila e executa a versão gráfica
(e `jogar.bat` faz o mesmo para a versão terminal).

### Controles (versão gráfica)

| Tecla | Ação |
|-------|------|
| `ESPAÇO` | rolar o dado / avançar |
| `A` `B` `C` `D` (ou clique) | responder a pergunta |
| `2` `3` `4` (ou clique) | escolher a quantidade de jogadores |
| `TAB` | abrir/fechar a tela das estruturas de dados |
| `R` | reiniciar a partida |
| `H` | regras (no menu) |
| `ESC` | sair |

[Raylib]: https://www.raylib.com/
