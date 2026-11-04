/*
 * rodar_todos.c - compila e executa os algoritmos de ordenacao, um de cada vez.
 *
 * Uso (na pasta do projeto):   rodar_todos [algoritmo ...]   (sem argumento: roda os quatro)
 *
 * Compila src/<nome>.c duas vezes (com e sem -DCONTAR) para build/ e roda o
 * executavel dentro de resultados/. Usa o gcc se estiver no PATH; senao, o cl (MSVC),
 * que exige o "Developer Command Prompt".
 *
 * Compilar este programa (gera build\rodar_todos.exe):
 *   gcc rodar_todos.c -o build\rodar_todos.exe
 *   cl rodar_todos.c /Fo"build\\" /Fe"build\rodar_todos.exe"
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#define SEP "\\"
#define EXE ".exe"
#define NULO "nul"
#define CRIAR_PASTA(p) _mkdir(p)
#define IR_PARA(p) _chdir(p)
#else
#include <sys/stat.h>
#include <unistd.h>
#define SEP "/"
#define EXE ""
#define NULO "/dev/null"
#define CRIAR_PASTA(p) mkdir((p), 0755)
#define IR_PARA(p) chdir(p)
#endif

#define PASTA_FONTES "src"
#define PASTA_BUILD "build"
#define PASTA_RESULTADOS "resultados"

static const char *const ALGORITMOS[] = {
    "selection_sort", "quick_sort", "binary_insertion_sort", "avl_tree_sort"
};
#define NUM_ALGORITMOS ((int)(sizeof ALGORITMOS / sizeof ALGORITMOS[0]))

/* Executa um comando de shell. O comando vai entre aspas extras porque o cmd.exe
 * apaga o primeiro e o último par de aspas da linha. Devolve o código de saída. */
static int executar(const char *comando)
{
    char linha[2048];
#ifdef _WIN32
    int n = snprintf(linha, sizeof linha, "\"%s\"", comando);
#else
    int n = snprintf(linha, sizeof linha, "%s", comando);
#endif
    if (n < 0 || (size_t)n >= sizeof linha) {
        fprintf(stderr, "ERRO: comando longo demais\n");
        return 1;
    }
    return system(linha);
}

/* 0 = gcc, 1 = MSVC (cl), -1 = nenhum. Só procura no PATH, sem executar o compilador. */
static int detectar_compilador(void)
{
#ifdef _WIN32
    if (executar("where gcc > nul 2>&1") == 0)
        return 0;
    if (executar("where cl > nul 2>&1") == 0)
        return 1;
#else
    if (executar("command -v gcc > /dev/null 2>&1") == 0)
        return 0;
#endif
    return -1;
}

/* Apaga os arquivos intermediários da compilação; só o .exe fica em build/. */
static void apagar_intermediarios(const char *nome)
{
    static const char *const sufixos[] = {"_contagem.o", "_contagem.obj", ".obj"};
    char caminho[256];
    for (size_t i = 0; i < sizeof sufixos / sizeof sufixos[0]; i++) {
        snprintf(caminho, sizeof caminho, PASTA_BUILD SEP "%s%s", nome, sufixos[i]);
        remove(caminho);   /* se não existir, tudo bem */
    }
}

static int compilar(int compilador, const char *nome)
{
    int erro;
    char cmd[1536];
    if (compilador == 0) {
        const char *flags = "-O2 -std=c11 -Wall -Wextra -DDUAS_VERSOES";
#ifdef _WIN32
        const char *extra = "-lm";
#else
        const char *extra = "-lm -pthread";
#endif
        printf("Compilando %s.c com gcc -O2: versao com contadores + versao de tempo ...\n", nome);
        fflush(stdout);
        snprintf(cmd, sizeof cmd, "gcc %s -DCONTAR -c \"" PASTA_FONTES SEP "%s.c\" -o \"" PASTA_BUILD SEP "%s_contagem.o\"",
                 flags, nome, nome);
        if (executar(cmd) != 0)
            return 1;
        snprintf(cmd, sizeof cmd,
                 "gcc %s \"" PASTA_FONTES SEP "%s.c\" \"" PASTA_BUILD SEP "%s_contagem.o\" -o \"" PASTA_BUILD SEP "%s" EXE "\" %s",
                 flags, nome, nome, nome, extra);
        erro = executar(cmd) != 0;
        apagar_intermediarios(nome);
        return erro;
    }
    {
        const char *flags = "/nologo /O2 /std:c11 /W4 /utf-8 /DDUAS_VERSOES";
        printf("Compilando %s.c com MSVC /O2: versao com contadores + versao de tempo ...\n", nome);
        fflush(stdout);
        snprintf(cmd, sizeof cmd, "cl %s /DCONTAR /c \"" PASTA_FONTES SEP "%s.c\" /Fo\"" PASTA_BUILD SEP "%s_contagem.obj\"",
                 flags, nome, nome);
        if (executar(cmd) != 0)
            return 1;
        snprintf(cmd, sizeof cmd,
                 "cl %s \"" PASTA_FONTES SEP "%s.c\" \"" PASTA_BUILD SEP "%s_contagem.obj\" /Fo\"" PASTA_BUILD SEP "%s.obj\" /Fe\""
                 PASTA_BUILD SEP "%s" EXE "\"",
                 flags, nome, nome, nome, nome);
        erro = executar(cmd) != 0;
        apagar_intermediarios(nome);
        return erro;
    }
}

/* Roda o executavel de dentro de resultados/, onde ele grava os arquivos. */
static int rodar(const char *nome)
{
    char cmd[512];
    int codigo;
    if (IR_PARA(PASTA_RESULTADOS) != 0) {
        fprintf(stderr, "ERRO: nao foi possivel entrar em " PASTA_RESULTADOS "\n");
        return 1;
    }
    snprintf(cmd, sizeof cmd, "\"..\\" PASTA_BUILD "\\%s" EXE "\"", nome);
#ifndef _WIN32
    snprintf(cmd, sizeof cmd, "\"../" PASTA_BUILD "/%s\"", nome);
#endif
    codigo = executar(cmd);
    IR_PARA("..");
    return codigo != 0;
}

int main(int argc, char **argv)
{
    int escolhidos[NUM_ALGORITMOS] = {0};
    int compilador, falhas = 0;
    char falhou[NUM_ALGORITMOS][64];
    FILE *teste = fopen(PASTA_FONTES SEP "medicao.h", "r");

    if (teste == NULL) {
        fprintf(stderr, "ERRO: rode este programa na pasta do projeto (a que contem " PASTA_FONTES ").\n");
        return 1;
    }
    fclose(teste);

#ifdef _WIN32
    /* Um experimento por vez: dois ao mesmo tempo disputariam a CPU e estragariam
     * os tempos (o mutex é liberado sozinho quando o programa termina). */
    CreateMutexA(NULL, FALSE, "Local\\rodar_todos_experimento");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        fprintf(stderr, "ERRO: ja existe um rodar_todos rodando. Espere ele terminar.\n");
        return 1;
    }
#endif

    if (argc <= 1) {
        for (int i = 0; i < NUM_ALGORITMOS; i++)
            escolhidos[i] = 1;
    } else {
        for (int a = 1; a < argc; a++) {
            int achou = 0;
            for (int i = 0; i < NUM_ALGORITMOS; i++)
                if (strcmp(argv[a], ALGORITMOS[i]) == 0) {
                    escolhidos[i] = achou = 1;
                }
            if (!achou) {
                fprintf(stderr, "ERRO: algoritmo desconhecido: %s\nOpcoes:", argv[a]);
                for (int i = 0; i < NUM_ALGORITMOS; i++)
                    fprintf(stderr, " %s", ALGORITMOS[i]);
                fprintf(stderr, "\n");
                return 1;
            }
        }
    }

    compilador = detectar_compilador();
    if (compilador < 0) {
        fprintf(stderr, "ERRO: nenhum compilador C encontrado.\n"
                        "Instale o gcc (MinGW-w64 ou MSYS2) ou o Visual Studio com a carga de trabalho\n"
                        "\"Desenvolvimento para desktop com C++\" (e rode a partir do Developer Command Prompt).\n");
        return 1;
    }

    CRIAR_PASTA(PASTA_BUILD);
    CRIAR_PASTA(PASTA_RESULTADOS);

    for (int i = 0; i < NUM_ALGORITMOS; i++) {
        if (!escolhidos[i])
            continue;
        printf("\n==================================================================\n"
               " %s\n"
               "==================================================================\n", ALGORITMOS[i]);
        fflush(stdout);
        if (compilar(compilador, ALGORITMOS[i]) != 0) {
            printf("ERRO: a compilacao de %s falhou.\n", ALGORITMOS[i]);
            snprintf(falhou[falhas++], sizeof falhou[0], "%s", ALGORITMOS[i]);
            continue;
        }
        printf("\n");
        fflush(stdout);
        if (rodar(ALGORITMOS[i]) != 0) {
            printf("\nO programa %s terminou com erro. Se apareceu \"bloqueado pela politica de\n"
                   "Controle de Aplicativo\", o Windows bloqueou o executavel recem-criado:\n"
                   "rode de novo para gerar um novo executavel.\n", ALGORITMOS[i]);
            snprintf(falhou[falhas++], sizeof falhou[0], "%s", ALGORITMOS[i]);
        }
    }

    printf("\n");
    if (falhas > 0) {
        printf("Terminou, mas com falha em:");
        for (int i = 0; i < falhas; i++)
            printf(" %s", falhou[i]);
        printf("\n");
        return 1;
    }
    printf("Terminou com sucesso. Resultados na pasta " PASTA_RESULTADOS ".\n");
    return 0;
}
