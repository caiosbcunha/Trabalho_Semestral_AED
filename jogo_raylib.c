/* Caminho do Conhecimento - versao grafica (Raylib).
 * Reaproveita os modulos do jogo (Fila, Pilha, Lista DE, Arvore AVL e
 * persistencia em disco); aqui fica apenas a interface grafica e os inputs.
 * Teclas: ESPACO rola | A/B/C/D responde | TAB estruturas | R reinicia | ESC sai.
 * Compilar: gcc jogo_raylib.c -o jogo_grafico -lraylib -lopengl32 -lgdi32 -lwinmm -lm */

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "jogador.h"
#include "perguntas.h"
#include "caminho.h"
#include "arvoreAVL.h"
#include "ranking.h"
#include "historico.h"
#include "estatisticas.h"

#define LARGURA_JANELA  1100
#define ALTURA_JANELA    720

#define BOARD_X   30
#define BOARD_Y   80
#define COLS      15
#define CELL_W    66
#define CELL_H    52
#define CELL_GAP   4
#define ROW_STEP 110

const Color COR_JOGADOR[MAX_JOGADORES] = { RED, BLUE, DARKGREEN, PURPLE };

typedef enum {
    ST_MENU, ST_AJUDA, ST_CADASTRO_QTD, ST_CADASTRO_NOME,
    ST_TURNO_ROLAR, ST_DADO, ST_ANDANDO, ST_PERGUNTA, ST_RESULTADO,
    ST_FIM, ST_ESTATISTICAS, ST_RANKING, ST_ESTRUTURAS
} Estado;

/* Estado global da partida (mesmas variaveis do main da versao terminal). */
tp_fila  fila_jogadores;
Jogador  banco_jogadores[MAX_JOGADORES];
int      qtd_jogadores;

tp_pilha pilha_u1, pilha_u2, pilha_u3;
Pergunta banco_perguntas[QTD_PERGUNTAS];

Casa     banco_casas[TAMANHO_CAMINHO];
Caminho *caminho;

ArvAVL  *avl_casas;

int      vencedor = -1;
int      turno    = 1;

Estado   estado;
Estado   estado_anterior;
int      id_atual;
Jogador *jogador;
int      dado, pos_ant, pos_landing;

Pergunta *perg_atual;
int       perg_id;

ResultadoJogada rj;
char     msg_turno[256];

char     buffer_nome[50];
int      nome_idx;

float    anim_timer;
int      dado_show;
int      anim_pos;

int      audio_ok = 0;
Sound    som_dado, som_acerto, som_erro, som_vitoria;

int      pedir_sair = 0;

EntradaRanking ranking_ent[MAX_JOGADORES];
int            ranking_qtd;

/* ---------- Auxiliares de desenho ---------- */

/* Desenha 'txt' quebrando em palavras dentro de 'maxWidth' pixels. */
int draw_text_wrapped(const char *txt, int x, int y,
                      int fontSize, int maxWidth, Color color) {
    int lineH = fontSize + 6;
    int curY  = y;
    int i = 0, n = (int)strlen(txt);
    char line[1024]; line[0] = '\0';
    char word[300];
    char test[1400];

    while (i < n) {
        int w = 0;
        while (i < n && txt[i] != ' ') { if (w < 299) word[w++] = txt[i]; i++; }
        word[w] = '\0';
        while (i < n && txt[i] == ' ') i++;

        if (line[0] == '\0') snprintf(test, sizeof(test), "%.299s", word);
        else                 snprintf(test, sizeof(test), "%.1023s %.299s", line, word);

        if (MeasureText(test, fontSize) > maxWidth && line[0] != '\0') {
            DrawText(line, x, curY, fontSize, color);
            curY += lineH;
            snprintf(line, sizeof(line), "%.299s", word);
        } else {
            snprintf(line, sizeof(line), "%.1023s", test);
        }
    }
    if (line[0] != '\0') { DrawText(line, x, curY, fontSize, color); curY += lineH; }
    return curY;
}

/* Gera um tom senoidal curto como Sound (com fade nas pontas, sem .wav). */
Sound gerar_tom(float freq, float dur, float vol) {
    int sr = 22050;
    unsigned int frames = (unsigned int)(sr * dur);
    short *d = (short *)malloc(frames * sizeof(short));
    for (unsigned int i = 0; i < frames; i++) {
        float t   = (float)i / sr;
        float env = 1.0f;
        float fade = 0.012f;
        if (t < fade)        env = t / fade;
        if (dur - t < fade)  env = (dur - t) / fade;
        float s = sinf(2.0f * 3.14159265f * freq * t);
        d[i] = (short)(s * env * vol * 32000.0f);
    }
    Wave w = { frames, (unsigned int)sr, 16, 1, d };
    Sound snd = LoadSoundFromWave(w);
    free(d);
    return snd;
}

void tocar(Sound s) { if (audio_ok) PlaySound(s); }

/* Face de um dado (1..6) como quadrado com pontinhos. */
void draw_die(int x, int y, int size, int valor) {
    Rectangle r = { (float)x, (float)y, (float)size, (float)size };
    DrawRectangleRec(r, RAYWHITE);
    DrawRectangleLinesEx(r, 3, DARKBLUE);

    float pr = size * 0.09f;
    float cx = x + size / 2.0f, cy = y + size / 2.0f;
    float ox = size * 0.26f,    oy = size * 0.26f;

    #define PIP(px, py) DrawCircle((int)(px), (int)(py), pr, BLACK)
    if (valor == 1 || valor == 3 || valor == 5) PIP(cx, cy);
    if (valor >= 2) { PIP(cx - ox, cy - oy); PIP(cx + ox, cy + oy); }
    if (valor >= 4) { PIP(cx + ox, cy - oy); PIP(cx - ox, cy + oy); }
    if (valor == 6) { PIP(cx - ox, cy);      PIP(cx + ox, cy); }
    #undef PIP
}

#define OPC_TOP   400
#define OPC_H      64
#define OPC_STEP   72

/* Retangulo da alternativa 'k' (usado no desenho e no clique). */
Rectangle rect_opcao(int k) {
    Rectangle r;
    r.x = 40;
    r.y = (float)(OPC_TOP + k * OPC_STEP);
    r.width  = 1020;
    r.height = OPC_H;
    return r;
}

Rectangle rect_botao_qtd(int q) {
    Rectangle r;
    r.x = 40.0f + (q - MIN_JOGADORES) * 130;
    r.y = 290;
    r.width  = 110;
    r.height = 80;
    return r;
}

Rectangle rect_menu(int i) {
    Rectangle r = { 430, (float)(310 + i * 80), 240, 60 };
    return r;
}

Rectangle rect_voltar(void) {
    Rectangle r = { 40, 650, 180, 50 };
    return r;
}

Rectangle rect_casa(int id) {
    int row = id / COLS;
    int col = id % COLS;
    Rectangle r;
    r.x = (float)(BOARD_X + col * (CELL_W + CELL_GAP));
    r.y = (float)(BOARD_Y + row * ROW_STEP);
    r.width  = CELL_W;
    r.height = CELL_H;
    return r;
}

Color cor_casa(int id, int tipo) {
    if (id == 0)                    return GOLD;
    if (id == TAMANHO_CAMINHO - 1)  return GOLD;
    switch (tipo) {
        case TIPO_SORTE:    return (Color){ 120, 200, 120, 255 };
        case TIPO_REVES:    return (Color){ 220, 130, 110, 255 };
        case TIPO_PERGUNTA: return (Color){ 120, 170, 230, 255 };
        default:            return (Color){ 210, 210, 210, 255 };
    }
}

char letra_casa(int id, int tipo) {
    if (id == 0)                    return 'I';
    if (id == TAMANHO_CAMINHO - 1)  return 'C';
    switch (tipo) {
        case TIPO_SORTE:    return '+';
        case TIPO_REVES:    return '-';
        case TIPO_PERGUNTA: return '?';
        default:            return '.';
    }
}

/* Tabuleiro (45 casas em 3 trilhas) + peoes dos jogadores. */
void desenhar_tabuleiro(void) {
    DrawText("CAMINHO DO CONHECIMENTO", BOARD_X, 18, 28, DARKBLUE);
    DrawText("Legenda:  [I]Inicio  [.]Normal  [+]Sorte  [-]Reves  [?]Pergunta  [C]Chegada",
             BOARD_X, 54, 16, DARKGRAY);

    for (int id = 0; id < TAMANHO_CAMINHO; id++) {
        Rectangle r = rect_casa(id);
        DrawRectangleRec(r, cor_casa(id, banco_casas[id].tipo_casa));
        DrawRectangleLinesEx(r, 2, DARKGRAY);

        char rotulo[8];
        snprintf(rotulo, sizeof(rotulo), "%02d%c", id,
                 letra_casa(id, banco_casas[id].tipo_casa));
        DrawText(rotulo, (int)r.x + 8, (int)r.y + 6, 18, BLACK);

        int slot = 0;
        for (int j = 0; j < qtd_jogadores; j++) {
            if (banco_jogadores[j].posicao_tabuleiro == id) {
                int cx = (int)r.x + 12 + slot * 15;
                int cy = (int)r.y + (int)r.height + 12;
                DrawCircle(cx, cy, 7, COR_JOGADOR[j]);
                DrawCircleLines(cx, cy, 7, BLACK);
                slot++;
            }
        }
    }
}

void desenhar_painel_jogadores(int x, int y) {
    DrawText("Jogadores", x, y, 20, DARKBLUE);
    for (int j = 0; j < qtd_jogadores; j++) {
        int yy = y + 30 + j * 26;
        DrawCircle(x + 8, yy + 8, 7, COR_JOGADOR[j]);
        char linha[128];
        snprintf(linha, sizeof(linha), "%s  -  casa %d  -  %d pts",
                 banco_jogadores[j].nome,
                 banco_jogadores[j].posicao_tabuleiro,
                 banco_jogadores[j].pontuacao);
        Color cor = (j == id_atual && estado != ST_CADASTRO_QTD &&
                     estado != ST_CADASTRO_NOME) ? MAROON : BLACK;
        DrawText(linha, x + 24, yy, 18, cor);
    }
}

/* ---------- Logica de efeito (mesmas formulas de aplicar_efeito_casa) ---------- */

void efeito_casa_simples(int pos) {
    Casa *casa = &banco_casas[pos];

    if (casa->tipo_casa == TIPO_SORTE) {
        int nova = mover_na_lista(caminho, pos, CASAS_BONUS_SORTE);
        jogador->posicao_tabuleiro = nova;
        snprintf(msg_turno, sizeof(msg_turno),
                 "SORTE!  Avanca %d casas:  %d -> %d", CASAS_BONUS_SORTE, pos, nova);
    } else if (casa->tipo_casa == TIPO_REVES) {
        int nova = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_REVES);
        jogador->posicao_tabuleiro = nova;
        snprintf(msg_turno, sizeof(msg_turno),
                 "REVES!  Volta %d casas:  %d -> %d", CASAS_PENALIDADE_REVES, pos, nova);
    } else {
        snprintf(msg_turno, sizeof(msg_turno), "Casa NORMAL. Sem efeito.");
    }
}

void obter_pergunta_da_casa(int pos) {
    tp_pilha *pilha = selecionar_pilha_por_id(pos, &pilha_u1, &pilha_u2, &pilha_u3);
    if (pilha_vazia(pilha))
        preencher_pilha_unidade(pilha, banco_perguntas, unidade_por_id(pos));
    pop(pilha, &perg_id);
    perg_atual = &banco_perguntas[perg_id];
}

void resolver_pergunta(int pos, char resposta) {
    char resp_norm = (resposta >= 'a' && resposta <= 'z')
                     ? (char)(resposta - 32) : resposta;

    rj.respondeu_pergunta = 1;
    rj.id_pergunta        = perg_id;
    rj.resposta_jogador   = resp_norm;
    rj.casa_pergunta      = pos;

    if (verificar_resposta(*perg_atual, resposta)) {
        int pontos = perg_atual->nivel * PONTOS_POR_NIVEL;
        int avanco = perg_atual->nivel;
        jogador->pontuacao += pontos;

        if (jogador->qtd_acertadas < MAX_HISTORICO_PERGUNTAS)
            jogador->perguntas_acertadas[jogador->qtd_acertadas++] = perg_id;

        int nova = mover_na_lista(caminho, pos, avanco);
        jogador->posicao_tabuleiro = nova;

        rj.acertou       = 1;
        rj.pontos_ganhos = pontos;
        tocar(som_acerto);
        snprintf(msg_turno, sizeof(msg_turno),
                 "ACERTOU!  +%d pts.  Avanca %d casa(s):  %d -> %d",
                 pontos, avanco, pos, nova);
    } else {
        if (jogador->qtd_erradas < MAX_HISTORICO_PERGUNTAS)
            jogador->perguntas_erradas[jogador->qtd_erradas++] = perg_id;

        int nova = mover_na_lista(caminho, pos, -CASAS_PENALIDADE_ERRO);
        jogador->posicao_tabuleiro = nova;

        rj.acertou       = 0;
        rj.pontos_ganhos = 0;
        tocar(som_erro);
        snprintf(msg_turno, sizeof(msg_turno),
                 "ERROU.  Correta: %c.  Volta %d casa(s):  %d -> %d",
                 perg_atual->resposta_correta, CASAS_PENALIDADE_ERRO, pos, nova);
    }

    registrar_resultado_casa(avl_casas, rj.casa_pergunta, rj.acertou);
    salvar_historico_resposta(jogador, &banco_perguntas[rj.id_pergunta],
                              rj.resposta_jogador, rj.acertou, rj.pontos_ganhos);
}

/* ---------- Transicoes de turno ---------- */

void iniciar_turno(void) {
    removeFila(&fila_jogadores, &id_atual);
    jogador = &banco_jogadores[id_atual];
    estado  = ST_TURNO_ROLAR;
}

void liberar_caminho(void) {
    if (caminho == NULL) return;
    tp_no *atu = caminho->ini;
    while (atu != NULL) { tp_no *prox = atu->prox; free(atu); atu = prox; }
    free(caminho);
    caminho = NULL;
}

void reiniciar_jogo(void) {
    liberar_caminho();
    liberaABB(avl_casas);
    avl_casas = criarAVL();

    vencedor      = -1;
    turno         = 1;
    qtd_jogadores = 0;
    nome_idx      = 0;
    buffer_nome[0] = '\0';
    estado        = ST_MENU;
}

int avl_ids[TAMANHO_CAMINHO], avl_vis[TAMANHO_CAMINHO],
    avl_ac[TAMANHO_CAMINHO], avl_er[TAMANHO_CAMINHO], avl_n;

void coletar_avl(struct NO *no) {
    if (no == NULL) return;
    coletar_avl(no->esq);
    avl_ids[avl_n] = no->info;
    avl_vis[avl_n] = no->contagem;
    avl_ac[avl_n]  = no->acertos;
    avl_er[avl_n]  = no->erros;
    avl_n++;
    coletar_avl(no->dir);
}

/* Le resultados.txt e ordena por pontuacao (mesmo parse de exibir_ranking_do_disco). */
void carregar_ranking_do_disco(void) {
    ranking_qtd = 0;
    FILE *f = fopen(ARQUIVO_RESULTADOS, "r");
    if (f == NULL) return;

    char linha[600], tmp[512], *tok;
    int count;
    while (fgets(linha, sizeof(linha), f) && ranking_qtd < MAX_JOGADORES) {
        linha[strcspn(linha, "\n")] = '\0';

        if (strncmp(linha, "NOME:", 5) == 0) {
            strncpy(ranking_ent[ranking_qtd].nome, linha + 5, 49);
            ranking_ent[ranking_qtd].nome[49]    = '\0';
            ranking_ent[ranking_qtd].pontuacao   = 0;
            ranking_ent[ranking_qtd].qtd_acertos = 0;
            ranking_ent[ranking_qtd].qtd_erros   = 0;
        } else if (strncmp(linha, "PONTUACAO:", 10) == 0) {
            ranking_ent[ranking_qtd].pontuacao = atoi(linha + 10);
        } else if (strncmp(linha, "ACERTOS:", 8) == 0) {
            strncpy(tmp, linha + 8, 511); tmp[511] = '\0';
            count = 0;
            if (strcmp(tmp, "-") != 0) {
                tok = strtok(tmp, ",");
                while (tok != NULL && count < QTD_PERGUNTAS) {
                    ranking_ent[ranking_qtd].acertos[count++] = atoi(tok);
                    tok = strtok(NULL, ",");
                }
            }
            ranking_ent[ranking_qtd].qtd_acertos = count;
        } else if (strncmp(linha, "ERROS:", 6) == 0) {
            strncpy(tmp, linha + 6, 511); tmp[511] = '\0';
            count = 0;
            if (strcmp(tmp, "-") != 0) {
                tok = strtok(tmp, ",");
                while (tok != NULL && count < QTD_PERGUNTAS) {
                    ranking_ent[ranking_qtd].erros[count++] = atoi(tok);
                    tok = strtok(NULL, ",");
                }
            }
            ranking_ent[ranking_qtd].qtd_erros = count;
        } else if (strcmp(linha, "###") == 0) {
            ranking_qtd++;
        }
    }
    fclose(f);

    for (int i = 0; i < ranking_qtd - 1; i++)
        for (int j = 0; j < ranking_qtd - 1 - i; j++)
            if (ranking_ent[j].pontuacao < ranking_ent[j + 1].pontuacao) {
                EntradaRanking t   = ranking_ent[j];
                ranking_ent[j]     = ranking_ent[j + 1];
                ranking_ent[j + 1] = t;
            }
}

/* ---------- Atualizacao / input (uma funcao por estado) ---------- */

void update_menu(void) {
    if (IsKeyPressed(KEY_ENTER)) { estado = ST_CADASTRO_QTD; return; }
    if (IsKeyPressed(KEY_H))     { estado = ST_AJUDA;        return; }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        if      (CheckCollisionPointRec(m, rect_menu(0))) estado = ST_CADASTRO_QTD;
        else if (CheckCollisionPointRec(m, rect_menu(1))) estado = ST_AJUDA;
        else if (CheckCollisionPointRec(m, rect_menu(2))) pedir_sair = 1;
    }
}

void update_ajuda(void) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) { estado = ST_MENU; return; }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), rect_voltar()))
        estado = ST_MENU;
}

void update_cadastro_qtd(void) {
    int q = 0;
    if (IsKeyPressed(KEY_TWO)   || IsKeyPressed(KEY_KP_2)) q = 2;
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) q = 3;
    if (IsKeyPressed(KEY_FOUR)  || IsKeyPressed(KEY_KP_4)) q = 4;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        for (int b = MIN_JOGADORES; b <= MAX_JOGADORES; b++)
            if (CheckCollisionPointRec(m, rect_botao_qtd(b))) q = b;
    }

    if (q >= MIN_JOGADORES && q <= MAX_JOGADORES) {
        qtd_jogadores = q;
        inicializaFila(&fila_jogadores);
        nome_idx = 0;
        buffer_nome[0] = '\0';
        estado = ST_CADASTRO_NOME;
    }
}

void update_cadastro_nome(void) {
    int c = GetCharPressed();
    int len = (int)strlen(buffer_nome);
    while (c > 0) {
        if (c >= 32 && c <= 125 && len < 49) {
            buffer_nome[len++] = (char)c;
            buffer_nome[len]   = '\0';
        }
        c = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && len > 0) buffer_nome[len - 1] = '\0';

    if (IsKeyPressed(KEY_ENTER) && strlen(buffer_nome) > 0) {
        Jogador *jg = &banco_jogadores[nome_idx];
        jg->id = nome_idx;
        strncpy(jg->nome, buffer_nome, 49);
        jg->nome[49]           = '\0';
        jg->posicao_tabuleiro  = 0;
        jg->pontuacao          = 0;
        jg->qtd_acertadas      = 0;
        jg->qtd_erradas        = 0;
        insereFila(&fila_jogadores, jg->id);

        nome_idx++;
        buffer_nome[0] = '\0';

        if (nome_idx >= qtd_jogadores) {
            criar_perguntas(&pilha_u1, &pilha_u2, &pilha_u3, banco_perguntas);
            salvar_perguntas_csv(banco_perguntas, QTD_PERGUNTAS);
            iniciar_historico_csv();
            caminho = criar_caminho(banco_casas);

            turno = 1;
            iniciar_turno();
        }
    }
}

void update_turno_rolar(void) {
    if (!IsKeyPressed(KEY_SPACE)) return;

    dado        = rolar_dado();
    pos_ant     = jogador->posicao_tabuleiro;
    pos_landing = mover_na_lista(caminho, pos_ant, dado);

    anim_timer = 0.0f;
    dado_show  = 1;
    tocar(som_dado);
    estado = ST_DADO;
}

void update_dado(void) {
    anim_timer += GetFrameTime();
    dado_show = (rand() % 6) + 1;

    if (anim_timer >= 0.8f) {
        dado_show  = dado;
        anim_pos   = pos_ant;
        anim_timer = 0.0f;
        estado     = ST_ANDANDO;
    }
}

/* Ao terminar a caminhada, efetiva a posicao real, registra a visita
 * na AVL e aplica o efeito da casa (igual ao fluxo da versao terminal). */
void update_andando(void) {
    anim_timer += GetFrameTime();
    if (anim_timer < 0.14f) return;
    anim_timer = 0.0f;

    if (anim_pos < pos_landing) {
        anim_pos++;
        jogador->posicao_tabuleiro = anim_pos;
    }

    if (anim_pos >= pos_landing) {
        jogador->posicao_tabuleiro = pos_landing;
        inserir(avl_casas, pos_landing);
        rj = (ResultadoJogada){ 0, -1, ' ', 0, 0, pos_landing };

        if (banco_casas[pos_landing].tipo_casa == TIPO_PERGUNTA) {
            obter_pergunta_da_casa(pos_landing);
            estado = ST_PERGUNTA;
        } else {
            efeito_casa_simples(pos_landing);
            if (banco_casas[pos_landing].tipo_casa != TIPO_NORMAL) tocar(som_dado);
            estado = ST_RESULTADO;
        }
    }
}

void update_pergunta(void) {
    char escolha = 0;
    if (IsKeyPressed(KEY_A)) escolha = 'A';
    if (IsKeyPressed(KEY_B)) escolha = 'B';
    if (IsKeyPressed(KEY_C)) escolha = 'C';
    if (IsKeyPressed(KEY_D)) escolha = 'D';

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        for (int k = 0; k < 4; k++)
            if (CheckCollisionPointRec(m, rect_opcao(k))) escolha = (char)('A' + k);
    }

    if (escolha) {
        resolver_pergunta(pos_landing, escolha);
        estado = ST_RESULTADO;
    }
}

void update_resultado(void) {
    if (!IsKeyPressed(KEY_SPACE)) return;

    if (jogador->posicao_tabuleiro >= TAMANHO_CAMINHO - 1) {
        vencedor = id_atual;
        tocar(som_vitoria);

        salvar_estatisticas_casas(avl_casas, banco_casas);
        salvar_resultados(banco_jogadores, qtd_jogadores);

        estado = ST_FIM;
        return;
    }

    insereFila(&fila_jogadores, id_atual);
    turno++;
    iniciar_turno();
}

/* ---------- Desenho (uma funcao por estado) ---------- */

int draw_botao(Rectangle r, const char *txt, int fontSize) {
    int hov = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRec(r, hov ? (Color){ 200, 220, 245, 255 } : LIGHTGRAY);
    DrawRectangleLinesEx(r, hov ? 3 : 2, hov ? DARKBLUE : DARKGRAY);
    int tw = MeasureText(txt, fontSize);
    DrawText(txt, (int)(r.x + (r.width - tw) / 2),
             (int)(r.y + (r.height - fontSize) / 2), fontSize, DARKBLUE);
    return hov;
}

void draw_menu(void) {
    DrawText("CAMINHO DO",   350, 90,  64, DARKBLUE);
    DrawText("CONHECIMENTO", 300, 158, 64, DARKBLUE);
    DrawText("Jogo de tabuleiro educativo  -  Trabalho de AED", 360, 250, 20, DARKGRAY);

    const char *lab[3] = { "JOGAR", "COMO JOGAR", "SAIR" };
    for (int i = 0; i < 3; i++) draw_botao(rect_menu(i), lab[i], 26);

    DrawText("ENTER = jogar     H = como jogar", 410, 580, 18, GRAY);
}

void draw_ajuda(void) {
    DrawText("COMO JOGAR", 40, 30, 38, DARKBLUE);

    const char *linhas[] = {
        "Objetivo: ser o primeiro a chegar na ultima casa (CHEGADA).",
        "",
        "A cada turno, o jogador da vez rola o dado e anda o numero de casas.",
        "",
        "Tipos de casa:",
        "   [.] Normal    - nada acontece.",
        "   [+] Sorte     - avanca 3 casas.",
        "   [-] Reves     - volta 2 casas.",
        "   [?] Pergunta  - responda A/B/C/D (clicando ou pelo teclado):",
        "         Acerto -> ganha pontos (Facil 10, Medio 20, Dificil 30) e",
        "                   avanca (1, 2 ou 3 casas conforme o nivel).",
        "         Erro   -> volta 1 casa (sem perder pontos).",
        "",
        "Estruturas de dados usadas (aperte TAB no jogo para visualizar):",
        "   Fila (turnos)  |  Pilhas (perguntas por unidade)  |  Arvore AVL (estatisticas).",
        "",
        "Teclas:  ESPACO = rolar  |  A/B/C/D = responder  |  TAB = estruturas  |  R = reiniciar.",
    };
    int n = (int)(sizeof(linhas) / sizeof(linhas[0]));
    for (int i = 0; i < n; i++)
        DrawText(linhas[i], 40, 100 + i * 30, 19, BLACK);

    draw_botao(rect_voltar(), "VOLTAR", 22);
    DrawText("(ENTER / BACKSPACE tambem voltam)", 240, 666, 16, GRAY);
}

void draw_cadastro_qtd(void) {
    DrawText("CAMINHO DO CONHECIMENTO", 40, 80, 40, DARKBLUE);
    DrawText("Cadastro de jogadores", 40, 150, 26, DARKGRAY);
    DrawText("Quantos jogadores?  Clique no numero ou pressione 2, 3 ou 4.",
             40, 230, 24, BLACK);

    Vector2 m = GetMousePosition();
    for (int q = MIN_JOGADORES; q <= MAX_JOGADORES; q++) {
        Rectangle r = rect_botao_qtd(q);
        int hover = CheckCollisionPointRec(m, r);
        DrawRectangleRec(r, hover ? (Color){ 200, 220, 245, 255 } : LIGHTGRAY);
        DrawRectangleLinesEx(r, hover ? 3 : 2, hover ? DARKBLUE : DARKGRAY);
        char t[4]; snprintf(t, sizeof(t), "%d", q);
        DrawText(t, (int)r.x + 45, (int)r.y + 22, 40, DARKBLUE);
    }
}

void draw_cadastro_nome(void) {
    DrawText("CAMINHO DO CONHECIMENTO", 40, 80, 40, DARKBLUE);
    char cab[64];
    snprintf(cab, sizeof(cab), "Nome do Jogador %d de %d:", nome_idx + 1, qtd_jogadores);
    DrawText(cab, 40, 170, 26, DARKGRAY);

    Rectangle cx = { 40, 230, 700, 60 };
    DrawRectangleRec(cx, RAYWHITE);
    DrawRectangleLinesEx(cx, 2, DARKBLUE);
    DrawText(buffer_nome, 52, 246, 30, BLACK);

    if (((int)(GetTime() * 2)) % 2 == 0) {
        int w = MeasureText(buffer_nome, 30);
        DrawText("_", 54 + w, 246, 30, BLACK);
    }
    DrawText("Digite o nome e pressione ENTER.", 40, 320, 20, GRAY);

    for (int j = 0; j < nome_idx; j++) {
        char l[80];
        snprintf(l, sizeof(l), "%d) %s", j + 1, banco_jogadores[j].nome);
        DrawCircle(50, 380 + j * 28 + 8, 7, COR_JOGADOR[j]);
        DrawText(l, 70, 380 + j * 28, 20, BLACK);
    }
}

void draw_turno_rolar(void) {
    desenhar_tabuleiro();
    desenhar_painel_jogadores(40, 420);

    char cab[160];
    snprintf(cab, sizeof(cab), "TURNO %d   |   Vez de: %s   |   Casa %d   |   %d pts",
             turno, jogador->nome, jogador->posicao_tabuleiro, jogador->pontuacao);
    DrawText(cab, 40, 560, 24, MAROON);
    DrawText("Pressione  [ESPACO]  para rolar o dado.", 40, 600, 22, DARKBLUE);
    DrawText("[TAB] estruturas de dados (AVL / Fila / Pilhas)    [R] reiniciar",
             40, 632, 18, GRAY);
}

void draw_animacao(int girando) {
    desenhar_tabuleiro();
    desenhar_painel_jogadores(40, 420);

    int ds = 100;
    int dx = LARGURA_JANELA - ds - 40;
    int dy = 540;
    draw_die(dx, dy, ds, dado_show);

    char cab[160];
    if (girando) {
        snprintf(cab, sizeof(cab), "TURNO %d   |   %s   |   rolando o dado...",
                 turno, jogador->nome);
        DrawText(cab, 40, 568, 24, MAROON);
    } else {
        snprintf(cab, sizeof(cab),
                 "TURNO %d   |   %s   |   Dado: %d   ->   andando ate a casa %d",
                 turno, jogador->nome, dado, pos_landing);
        DrawText(cab, 40, 568, 24, MAROON);
        snprintf(cab, sizeof(cab), "Casa atual: %d", anim_pos);
        DrawText(cab, 40, 604, 22, DARKBLUE);
    }
}

/* revisao=0: respondendo; revisao=1: correta em verde, sua resposta errada em vermelho. */
void draw_tela_pergunta(int revisao) {
    char cab[200];
    snprintf(cab, sizeof(cab),
             "TURNO %d   |   %s   |   Casa %d  [PERGUNTA]   Unid.%d - %s - %s",
             turno, jogador->nome, pos_landing, perg_atual->unidade,
             perg_atual->tema, nivel_para_str(perg_atual->nivel));
    DrawText(cab, 40, 24, 20, MAROON);
    DrawRectangleRec((Rectangle){ 40, 58, 1020, 2 }, LIGHTGRAY);

    draw_text_wrapped(perg_atual->texto, 40, 80, 24, 1020, BLACK);

    const char *opcoes[4] = { perg_atual->opcao_a, perg_atual->opcao_b,
                              perg_atual->opcao_c, perg_atual->opcao_d };
    Vector2 m = GetMousePosition();
    for (int k = 0; k < 4; k++) {
        Rectangle r   = rect_opcao(k);
        char letra_op = (char)('A' + k);
        int  correta  = (letra_op == perg_atual->resposta_correta);
        int  marcada  = (letra_op == rj.resposta_jogador);

        Color fundo, borda, txt;
        float esp = 2;

        if (revisao) {
            if (correta) {
                fundo = (Color){ 200, 235, 200, 255 }; borda = DARKGREEN; txt = DARKGREEN; esp = 3;
            } else if (marcada) {
                fundo = (Color){ 245, 205, 200, 255 }; borda = MAROON;    txt = MAROON;    esp = 3;
            } else {
                fundo = (Color){ 235, 235, 235, 255 }; borda = DARKGRAY;  txt = DARKGRAY;
            }
        } else {
            fundo = CheckCollisionPointRec(m, r)
                    ? (Color){ 200, 220, 245, 255 } : (Color){ 235, 235, 235, 255 };
            borda = DARKGRAY; txt = BLACK;
        }

        DrawRectangleRec(r, fundo);
        DrawRectangleLinesEx(r, esp, borda);

        char letra[8]; snprintf(letra, sizeof(letra), "%c)", letra_op);
        DrawText(letra, (int)r.x + 12, (int)r.y + 18, 24, revisao ? txt : DARKBLUE);
        draw_text_wrapped(opcoes[k], (int)r.x + 56, (int)r.y + 12, 19, 850, txt);

        if (revisao && correta)
            DrawText("CERTA", (int)(r.x + r.width) - 110, (int)r.y + 20, 20, DARKGREEN);
        else if (revisao && marcada)
            DrawText("SUA RESP.", (int)(r.x + r.width) - 130, (int)r.y + 20, 20, MAROON);
    }

    int yRodape = OPC_TOP + 4 * OPC_STEP + 4;
    if (revisao) {
        DrawText(msg_turno, 40, yRodape, 18, rj.acertou ? DARKGREEN : MAROON);
        DrawText("[ESPACO] continuar", 900, yRodape, 18, DARKBLUE);
    } else {
        DrawText("Responda pelo teclado (A/B/C/D) ou clicando na alternativa.",
                 40, yRodape, 18, GRAY);
    }
}

void draw_resultado(void) {
    if (rj.respondeu_pergunta) {
        draw_tela_pergunta(1);
        return;
    }

    desenhar_tabuleiro();
    desenhar_painel_jogadores(40, 420);

    DrawText("Resultado do turno:", 40, 560, 22, DARKBLUE);
    DrawText(msg_turno, 40, 592, 22, DARKGRAY);
    DrawText("Pressione  [ESPACO]  para continuar.", 40, 640, 20, GRAY);
}

void draw_fim(void) {
    ClearBackground((Color){ 245, 245, 230, 255 });
    DrawText("FIM DE JOGO", 40, 60, 48, GOLD);

    char l[160];
    snprintf(l, sizeof(l), "*** %s chegou na CHEGADA e VENCEU! ***",
             banco_jogadores[vencedor].nome);
    DrawText(l, 40, 150, 28, DARKGREEN);

    snprintf(l, sizeof(l), "Total de turnos: %d", turno);
    DrawText(l, 40, 200, 24, BLACK);

    for (int j = 0; j < qtd_jogadores; j++) {
        snprintf(l, sizeof(l), "%s  -  %d pts  (acertos: %d  erros: %d)",
                 banco_jogadores[j].nome, banco_jogadores[j].pontuacao,
                 banco_jogadores[j].qtd_acertadas, banco_jogadores[j].qtd_erradas);
        DrawCircle(52, 270 + j * 34 + 10, 8, COR_JOGADOR[j]);
        DrawText(l, 72, 270 + j * 34, 22, BLACK);
    }

    DrawText("Arquivos gerados: resultados.txt, perguntas.csv,", 40, 470, 18, GRAY);
    DrawText("historico_respostas.csv, estatisticas_casas.csv", 40, 494, 18, GRAY);
    DrawText("Pressione  [ESPACO]  para ver as estatisticas por casa (AVL).",
             40, 560, 22, DARKBLUE);
    DrawText("[R] jogar de novo", 40, 596, 20, GRAY);
}

/* Estatisticas por casa (AVL em ordem) como barras empilhadas. */
void draw_estatisticas(void) {
    ClearBackground(RAYWHITE);
    DrawText("ESTATISTICAS POR CASA  (Arvore AVL - percurso em ordem)",
             30, 24, 24, DARKBLUE);

    Color C_VIS = (Color){ 90, 150, 220, 255 };
    Color C_AC  = (Color){ 70, 180, 90,  255 };
    Color C_ER  = (Color){ 215, 90, 80,  255 };

    DrawRectangle(30, 62, 16, 16, C_VIS); DrawText("visitas", 52, 61, 18, DARKGRAY);
    DrawRectangle(150, 62, 16, 16, C_AC); DrawText("acertos", 172, 61, 18, DARKGRAY);
    DrawRectangle(280, 62, 16, 16, C_ER); DrawText("erros",   302, 61, 18, DARKGRAY);
    DrawText("(barra empilhada; altura total = visitas)", 380, 61, 16, GRAY);

    if (avl_n == 0) {
        DrawText("(sem estatisticas)", 40, 120, 20, GRAY);
    } else {
        int   plotL = 40, plotR = 1060, baseY = 600, topY = 150;
        int   plotW = plotR - plotL;
        float barMaxH = (float)(baseY - topY);

        int maxV = 1;
        for (int i = 0; i < avl_n; i++) if (avl_vis[i] > maxV) maxV = avl_vis[i];

        for (int g = 0; g <= 4; g++) {
            int gy = baseY - (int)(barMaxH * g / 4);
            DrawRectangle(plotL, gy, plotW, 1, (Color){ 225, 225, 225, 255 });
            char vl[16]; snprintf(vl, sizeof(vl), "%d", maxV * g / 4);
            DrawText(vl, plotL - 28, gy - 8, 14, GRAY);
        }
        DrawRectangle(plotL, baseY, plotW, 2, DARKGRAY);

        float groupW = (float)plotW / avl_n;
        int   barW   = (int)(groupW * 0.6f); if (barW > 44) barW = 44; if (barW < 3) barW = 3;
        int   fonte  = (avl_n > 30) ? 10 : 14;

        for (int i = 0; i < avl_n; i++) {
            float cx = plotL + (i + 0.5f) * groupW;
            int   x  = (int)(cx - barW / 2.0f);

            int hA = (int)((float)avl_ac[i]  / maxV * barMaxH);
            int hE = (int)((float)avl_er[i]  / maxV * barMaxH);
            int hV = (int)((float)avl_vis[i] / maxV * barMaxH);
            int hR = hV - hA - hE; if (hR < 0) hR = 0;

            int yb = baseY;
            DrawRectangle(x, yb - hA, barW, hA, C_AC); yb -= hA;
            DrawRectangle(x, yb - hE, barW, hE, C_ER); yb -= hE;
            DrawRectangle(x, yb - hR, barW, hR, C_VIS);

            char t[8]; snprintf(t, sizeof(t), "%d", avl_ids[i]);
            DrawText(t, (int)(cx - MeasureText(t, fonte) / 2.0f), baseY + 6, fonte, BLACK);

            if (groupW >= 20) {
                char v[8]; snprintf(v, sizeof(v), "%d", avl_vis[i]);
                DrawText(v, (int)(cx - MeasureText(v, 12) / 2.0f), baseY - hV - 16, 12, DARKGRAY);
            }
        }
        DrawText("casa (id)", plotR - 90, baseY + 28, 16, DARKGRAY);
    }

    DrawText("Pressione  [ESPACO]  para ver o RANKING (lido do disco).",
             30, ALTURA_JANELA - 38, 20, DARKBLUE);
}

void draw_ranking(void) {
    ClearBackground((Color){ 245, 245, 255, 255 });
    DrawText("RANKING FINAL  (lido de resultados.txt)", 30, 30, 26, DARKBLUE);

    int y = 80;
    for (int i = 0; i < ranking_qtd; i++) {
        char l[160];
        snprintf(l, sizeof(l), "%d.  %s   -   %d pts",
                 i + 1, ranking_ent[i].nome, ranking_ent[i].pontuacao);
        DrawText(l, 30, y, 24, (i == 0) ? GOLD : BLACK);
        y += 32;

        if (ranking_ent[i].qtd_acertos == 0) {
            DrawText("    Acertos: nenhum", 30, y, 18, DARKGRAY); y += 24;
        } else {
            for (int j = 0; j < ranking_ent[i].qtd_acertos; j++) {
                int id = ranking_ent[i].acertos[j];
                char curto[64];
                resumo_texto(banco_perguntas[id].texto, curto, 56);
                char ln[160];
                snprintf(ln, sizeof(ln), "    [OK] (id %2d) %s", id, curto);
                DrawText(ln, 30, y, 16, DARKGREEN); y += 20;
            }
        }
        if (ranking_ent[i].qtd_erros == 0) {
            DrawText("    Erros: nenhum", 30, y, 18, DARKGRAY); y += 24;
        } else {
            for (int j = 0; j < ranking_ent[i].qtd_erros; j++) {
                int id = ranking_ent[i].erros[j];
                char curto[64];
                resumo_texto(banco_perguntas[id].texto, curto, 56);
                char ln[180];
                snprintf(ln, sizeof(ln), "    [X] (id %2d) %s  (Correta: %c)",
                         id, curto, banco_perguntas[id].resposta_correta);
                DrawText(ln, 30, y, 16, MAROON); y += 20;
            }
        }
        y += 12;
    }
    DrawText("Pressione  [ESC]  para sair.", 30, ALTURA_JANELA - 36, 20, DARKBLUE);
}

/* ---------- Tela das estruturas de dados (tecla TAB) ---------- */

int avl_islot;

/* AVL desenhada como arvore: x pela posicao em-ordem, y pela profundidade. */
int draw_avl_rec(struct NO *no, int depth,
                 int xLeft, int slotW, int yTop, int levelH) {
    if (no == NULL) return 0;

    int xl = draw_avl_rec(no->esq, depth + 1, xLeft, slotW, yTop, levelH);
    int myx = xLeft + avl_islot * slotW + slotW / 2;
    avl_islot++;
    int myy = yTop + depth * levelH;
    int xr = draw_avl_rec(no->dir, depth + 1, xLeft, slotW, yTop, levelH);

    int childY = yTop + (depth + 1) * levelH;
    if (no->esq != NULL) DrawLine(myx, myy, xl, childY, DARKGRAY);
    if (no->dir != NULL) DrawLine(myx, myy, xr, childY, DARKGRAY);

    int rad = slotW / 2 - 3;
    if (rad > 18) rad = 18;
    if (rad < 9)  rad = 9;

    Color fundo = (Color){ 210, 225, 245, 255 };
    if (no->acertos > no->erros) fundo = (Color){ 200, 235, 200, 255 };
    else if (no->erros > no->acertos) fundo = (Color){ 245, 210, 205, 255 };

    DrawCircle(myx, myy, (float)rad, fundo);
    DrawCircleLines(myx, myy, (float)rad, DARKBLUE);

    char t[8]; snprintf(t, sizeof(t), "%d", no->info);
    DrawText(t, myx - MeasureText(t, 16) / 2, myy - 8, 16, BLACK);

    char c[16]; snprintf(c, sizeof(c), "x%d", no->contagem);
    DrawText(c, myx - MeasureText(c, 12) / 2, myy + rad + 1, 12, DARKGRAY);

    return myx;
}

void draw_estruturas(void) {
    DrawText("ESTRUTURAS DE DADOS    (TAB volta ao jogo)", 30, 16, 24, DARKBLUE);

    DrawText("Arvore AVL - estatisticas por casa  (no = id da casa | xN = visitas | "
             "verde: mais acertos, vermelho: mais erros)", 30, 50, 15, DARKGRAY);

    int avl_total = (avl_casas != NULL) ? totalNOsABB(avl_casas) : 0;
    if (avl_total == 0) {
        DrawText("(arvore vazia - ninguem caiu numa casa ainda)", 40, 90, 18, GRAY);
    } else {
        avl_islot = 0;
        int slotW = 1040 / avl_total;
        if (slotW < 18) slotW = 18;
        draw_avl_rec(*avl_casas, 0, 30, slotW, 84, 42);
    }

    int yF = 396;
    DrawText("Fila de turnos (FIFO) - proximo a jogar a ESQUERDA:", 30, yF, 16, DARKGRAY);
    {
        int x = 30, y = yF + 24, ordem = 0;
        int i = fila_jogadores.ini;
        while (i != fila_jogadores.fim) {
            i = proximo(i);
            int id = fila_jogadores.item[i];
            Rectangle r = { (float)x, (float)y, 160, 34 };
            DrawRectangleRec(r, (ordem == 0) ? (Color){ 200, 235, 200, 255 } : LIGHTGRAY);
            DrawRectangleLinesEx(r, 2, DARKGRAY);
            DrawCircle(x + 14, y + 17, 7, COR_JOGADOR[id]);
            DrawText(banco_jogadores[id].nome, x + 28, y + 8, 18, BLACK);
            x += 172; ordem++;
        }
        if (ordem == 0) DrawText("(vazia)", 40, y + 6, 18, GRAY);
    }

    int yP = 470;
    DrawText("Pilhas de perguntas por unidade (LIFO) - TOPO em cima (proxima a sair):",
             30, yP, 16, DARKGRAY);

    tp_pilha   *pilhas[3] = { &pilha_u1, &pilha_u2, &pilha_u3 };
    const char *lab[3]    = { "Unidade 1", "Unidade 2", "Unidade 3" };
    for (int u = 0; u < 3; u++) {
        int colx = 30 + u * 360;
        DrawText(lab[u], colx, yP + 24, 18, DARKBLUE);

        tp_pilha *p = pilhas[u];
        int yy = yP + 50;
        for (int s = p->topo; s >= 0; s--) {
            int id = p->item[s];
            Rectangle r = { (float)colx, (float)yy, 330, 22 };
            DrawRectangleRec(r, (s == p->topo) ? (Color){ 200, 220, 245, 255 }
                                               : (Color){ 235, 235, 235, 255 });
            DrawRectangleLinesEx(r, 1, DARKGRAY);
            char t[96];
            snprintf(t, sizeof(t), "P%03d  (nivel %d)  %s",
                     id + 1, banco_perguntas[id].nivel, banco_perguntas[id].tema);
            DrawText(t, colx + 6, yy + 3, 14, BLACK);
            yy += 24;
            if (yy > 700) break;
        }
        if (p->topo < 0) DrawText("(vazia)", colx, yP + 50, 16, GRAY);
    }
}

int main(void) {
    srand((unsigned)time(NULL));

    avl_casas = criarAVL();

    InitWindow(LARGURA_JANELA, ALTURA_JANELA, "Caminho do Conhecimento - Raylib");
    SetTargetFPS(60);

    InitAudioDevice();
    audio_ok = IsAudioDeviceReady();
    if (audio_ok) {
        som_dado    = gerar_tom(420.0f, 0.10f, 0.5f);
        som_acerto  = gerar_tom(880.0f, 0.18f, 0.5f);
        som_erro    = gerar_tom(160.0f, 0.30f, 0.5f);
        som_vitoria = gerar_tom(660.0f, 0.45f, 0.6f);
    }

    estado = ST_MENU;

    while (!WindowShouldClose() && !pedir_sair) {

        switch (estado) {
            case ST_MENU:          update_menu();          break;
            case ST_AJUDA:         update_ajuda();         break;
            case ST_CADASTRO_QTD:  update_cadastro_qtd();  break;
            case ST_CADASTRO_NOME: update_cadastro_nome(); break;
            case ST_TURNO_ROLAR:   update_turno_rolar();   break;
            case ST_DADO:          update_dado();          break;
            case ST_ANDANDO:       update_andando();       break;
            case ST_PERGUNTA:      update_pergunta();      break;
            case ST_RESULTADO:     update_resultado();     break;
            case ST_FIM:
                if (IsKeyPressed(KEY_SPACE)) {
                    avl_n = 0;
                    if (avl_casas != NULL) coletar_avl(*avl_casas);
                    estado = ST_ESTATISTICAS;
                }
                break;
            case ST_ESTATISTICAS:
                if (IsKeyPressed(KEY_SPACE)) {
                    carregar_ranking_do_disco();
                    estado = ST_RANKING;
                }
                break;
            case ST_RANKING:    break;
            case ST_ESTRUTURAS: break;
        }

        if (IsKeyPressed(KEY_TAB)) {
            if (estado == ST_ESTRUTURAS) {
                estado = estado_anterior;
            } else if (estado == ST_TURNO_ROLAR || estado == ST_PERGUNTA ||
                       estado == ST_RESULTADO) {
                estado_anterior = estado;
                estado = ST_ESTRUTURAS;
            }
        }

        if (IsKeyPressed(KEY_R) &&
            estado != ST_MENU  && estado != ST_AJUDA &&
            estado != ST_CADASTRO_QTD && estado != ST_CADASTRO_NOME) {
            reiniciar_jogo();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (estado) {
            case ST_MENU:          draw_menu();           break;
            case ST_AJUDA:         draw_ajuda();          break;
            case ST_CADASTRO_QTD:  draw_cadastro_qtd();   break;
            case ST_CADASTRO_NOME: draw_cadastro_nome();  break;
            case ST_TURNO_ROLAR:   draw_turno_rolar();    break;
            case ST_DADO:          draw_animacao(1);      break;
            case ST_ANDANDO:       draw_animacao(0);      break;
            case ST_PERGUNTA:      draw_tela_pergunta(0); break;
            case ST_RESULTADO:     draw_resultado();      break;
            case ST_FIM:           draw_fim();            break;
            case ST_ESTATISTICAS:  draw_estatisticas();   break;
            case ST_RANKING:       draw_ranking();        break;
            case ST_ESTRUTURAS:    draw_estruturas();     break;
        }

        EndDrawing();
    }

    if (audio_ok) {
        UnloadSound(som_dado);   UnloadSound(som_acerto);
        UnloadSound(som_erro);   UnloadSound(som_vitoria);
    }
    CloseAudioDevice();
    CloseWindow();
    liberaABB(avl_casas);
    return 0;
}
