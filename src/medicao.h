/*
 * medicao.h — infraestrutura de medição (contém o main), compartilhada pelos 4 algoritmos.
 *
 * Cada algoritmo define NOME_ALGORITMO, inclui este arquivo e implementa
 * void ordenar(int *v, int n, Contadores *cont) usando as macros MENOR, MOVE,
 * TROCA, ENTRA_CHAMADA e SAI_CHAMADA.
 *
 * O arquivo do algoritmo é compilado duas vezes (rodar_todos): com -DCONTAR, para
 * a contagem, e sem, para o tempo; as duas versões vão no mesmo executável.
 */
#ifndef MEDICAO_H
#define MEDICAO_H

#define _CRT_SECURE_NO_WARNINGS    /* MSVC: permite fopen, localtime etc. */
#define __USE_MINGW_ANSI_STDIO 1   /* MinGW: printf com suporte completo a %llu */
#ifndef _WIN32
#define _DEFAULT_SOURCE            /* Linux: clock_gettime, sysconf, realpath */
#endif

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NOME_ALGORITMO
#error "Defina NOME_ALGORITMO antes de incluir medicao.h"
#endif
#ifndef TESTE_RAPIDO
#define TESTE_RAPIDO 0
#endif

/* Contadores e macros usadas pelos algoritmos */

typedef struct {
    unsigned long long comparacoes, movimentacoes;
    int profundidade, profundidade_max;
    uintptr_t topo_pilha, menor_endereco;
    int deslocamento;
} Contadores;

static inline void registra_chamada(Contadores *cont, const char *marca)
{
    uintptr_t endereco = (uintptr_t)marca;
    if (++cont->profundidade > cont->profundidade_max)
        cont->profundidade_max = cont->profundidade;
    if (endereco < cont->menor_endereco)
        cont->menor_endereco = endereco;
}

/*
 * MENOR(a, b)       compara dois elementos (a < b)
 * MOVE(k)           registra k movimentações de elementos
 * TROCA(a, b)       troca dois elementos (3 movimentações)
 * ENTRA_CHAMADA()   no início de cada função de ordenação (mede profundidade e pilha)
 * SAI_CHAMADA()     no fim de cada função de ordenação
 */
#if defined(DUAS_VERSOES) && defined(CONTAR)
#define MENOR(a, b) \
    (cont->comparacoes++, ((a) >> cont->deslocamento) < ((b) >> cont->deslocamento))
#define MOVE(k)         (cont->movimentacoes += (unsigned long long)(k))
#define ENTRA_CHAMADA() char marca_pilha_ = 0; registra_chamada(cont, &marca_pilha_)
#define SAI_CHAMADA()   (cont->profundidade--)
#elif defined(DUAS_VERSOES)
#define MENOR(a, b)     ((void)cont, (a) < (b))
#define MOVE(k)         ((void)cont)
#define ENTRA_CHAMADA() ((void)cont)
#define SAI_CHAMADA()   ((void)cont)
#else
#define MENOR(a, b) \
    (cont ? (cont->comparacoes++, ((a) >> cont->deslocamento) < ((b) >> cont->deslocamento)) : (a) < (b))
#define MOVE(k)         do { if (cont) cont->movimentacoes += (unsigned long long)(k); } while (0)
#define ENTRA_CHAMADA() char marca_pilha_ = 0; if (cont) registra_chamada(cont, &marca_pilha_)
#define SAI_CHAMADA()   do { if (cont) cont->profundidade--; } while (0)
#endif
#define TROCA(a, b) do { int t_ = (a); (a) = (b); (b) = t_; MOVE(3); } while (0)

/* Memória auxiliar: todo vetor extra é alocado por aqui, para medir o pico de bytes. */
#if defined(DUAS_VERSOES) && defined(CONTAR)
extern size_t medicao_mem_atual, medicao_mem_pico;
#else
size_t medicao_mem_atual, medicao_mem_pico;
#endif

static inline void *mem_alocar(size_t bytes)
{
    void *p = malloc(bytes);
    if (p == NULL) {
        fprintf(stderr, "ERRO: falta de memoria\n");
        exit(EXIT_FAILURE);
    }
    medicao_mem_atual += bytes;
    if (medicao_mem_atual > medicao_mem_pico)
        medicao_mem_pico = medicao_mem_atual;
    return p;
}

static inline void mem_liberar(void *p, size_t bytes)
{
    free(p);
    medicao_mem_atual -= bytes;
}

#if defined(DUAS_VERSOES) && defined(CONTAR)
/* Na versão com contadores, as funções do algoritmo ganham outro nome. */
#define ordenar ordenar_contando
#define ordenar_variante ordenar_variante_contando
void ordenar(int *v, int n, Contadores *cont);
#ifdef VARIANTE
void ordenar_variante(int *v, int n, Contadores *cont);
#endif

#else

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#endif
#else
#include <pthread.h>
#include <unistd.h>
#endif

typedef void (*Ordenacao)(int *v, int n, Contadores *cont);

/* Implementados no arquivo do algoritmo */
void ordenar(int *v, int n, Contadores *cont);
#ifdef VARIANTE
void ordenar_variante(int *v, int n, Contadores *cont);
#endif
#ifdef DUAS_VERSOES
void ordenar_contando(int *v, int n, Contadores *cont);
#define VERSAO_CONTAGEM ordenar_contando
#ifdef VARIANTE
void ordenar_variante_contando(int *v, int n, Contadores *cont);
#define VERSAO_CONTAGEM_VARIANTE ordenar_variante_contando
#endif
#else
#define VERSAO_CONTAGEM ordenar
#define VERSAO_CONTAGEM_VARIANTE ordenar_variante
#endif

/* Configuração do experimento */

static const int TAMANHOS[] = {500, 1000, 2000, 4000, 8000, 16000, 32000, 64000};
#define NUM_TAMANHOS ((int)(sizeof TAMANHOS / sizeof TAMANHOS[0]))
#define MAX_N_TESTE_RAPIDO 8000

#define REPETICOES 10
#define REPETICOES_N_PEQUENO 30
#define LIMITE_N_PEQUENO 4000
#define REPETICOES_TESTE_RAPIDO 3
#define ENTRADAS_CONTAGEM 3
#define SEMENTE_BASE 2026u

/* O experimento roda numa thread com pilha grande (a variante de pior caso do Quick Sort recursa até n). */
#define PILHA_EXPERIMENTO ((size_t)128 * 1024 * 1024)

enum { ALEATORIA, CRESCENTE, DECRESCENTE, QUASE_ORDENADA, REPETIDOS, NUM_DIST };
static const char *const NOME_DIST[NUM_DIST] = {
    "aleatoria", "crescente", "decrescente", "quase_ordenada", "repetidos"
};

/* Geração das entradas */

static uint32_t estado_prng;

/* xorshift32 (Marsaglia, 2003). */
static uint32_t xorshift32(void)
{
    estado_prng ^= estado_prng << 13;
    estado_prng ^= estado_prng >> 17;
    estado_prng ^= estado_prng << 5;
    return estado_prng;
}

static void gerar_entrada(int *v, int n, int dist, uint32_t semente)
{
    estado_prng = semente;
    switch (dist) {
    case ALEATORIA:
        for (int i = 0; i < n; i++)
            v[i] = (int)(xorshift32() >> 1);
        break;
    case CRESCENTE:
        for (int i = 0; i < n; i++)
            v[i] = i;
        break;
    case DECRESCENTE:
        for (int i = 0; i < n; i++)
            v[i] = n - i;
        break;
    case QUASE_ORDENADA:
        for (int i = 0; i < n; i++)
            v[i] = i;
        for (int k = 0; k < n / 100; k++) {
            int a = (int)(xorshift32() % (uint32_t)n);
            int b = (int)(xorshift32() % (uint32_t)n);
            int t = v[a];
            v[a] = v[b];
            v[b] = t;
        }
        break;
    case REPETIDOS:
        for (int i = 0; i < n; i++)
            v[i] = (int)(xorshift32() % 10u);
        break;
    }
}

/* Utilidades */

static int num_tamanhos(void)
{
    int k = 0;
    while (k < NUM_TAMANHOS && (!TESTE_RAPIDO || TAMANHOS[k] <= MAX_N_TESTE_RAPIDO))
        k++;
    return k;
}

static int repeticoes_para(int n)
{
    if (TESTE_RAPIDO)
        return REPETICOES_TESTE_RAPIDO;
    return n <= LIMITE_N_PEQUENO ? REPETICOES_N_PEQUENO : REPETICOES;
}

static int entradas_contagem(int dist)
{
    return (dist == CRESCENTE || dist == DECRESCENTE) ? 1 : ENTRADAS_CONTAGEM;
}

static int *aloca_int(int n)
{
    int *p = malloc((size_t)n * sizeof(int));
    if (p == NULL) {
        fprintf(stderr, "ERRO: falta de memoria\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

/* Relógio de alta resolução, em milissegundos. */
static double agora_ms(void)
{
#ifdef _WIN32
    static LARGE_INTEGER freq;
    LARGE_INTEGER t;
    if (freq.QuadPart == 0)
        QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e3 + (double)t.tv_nsec / 1e6;
#endif
}

static int compara_int(const void *pa, const void *pb)
{
    int a = *(const int *)pa, b = *(const int *)pb;
    return (a > b) - (a < b);
}

/* Confere se 'resultado' é 'original' ordenado (referência: qsort). */
static void verificar(const int *resultado, const int *original, int *referencia, int n,
                      const char *nome, const char *contexto)
{
    memcpy(referencia, original, (size_t)n * sizeof(int));
    qsort(referencia, (size_t)n, sizeof(int), compara_int);
    if (memcmp(resultado, referencia, (size_t)n * sizeof(int)) != 0) {
        fprintf(stderr, "\nERRO: %s produziu um vetor incorreto (%s, n = %d)\n", nome, contexto, n);
        exit(EXIT_FAILURE);
    }
}

/* Formata um número com vírgula decimal (padrão brasileiro). */
static const char *br(const char *formato, double x)
{
    static char buffers[24][64];
    static int proximo = 0;
    char *s = buffers[proximo];
    proximo = (proximo + 1) % 24;
    if (isnan(x))
        return "-";
    snprintf(s, sizeof buffers[0], formato, x);
    char *ponto = strchr(s, '.');
    if (ponto != NULL)
        *ponto = ',';
    return s;
}

static FILE *criar_arquivo(const char *nome)
{
    FILE *f = fopen(nome, "w");
    if (f == NULL) {
        fprintf(stderr, "ERRO: nao foi possivel criar %s\n", nome);
        exit(EXIT_FAILURE);
    }
    return f;
}

static void caminho_completo(const char *arquivo, char *saida, size_t tamanho)
{
#ifdef _WIN32
    if (_fullpath(saida, arquivo, tamanho) == NULL)
        snprintf(saida, tamanho, "%s", arquivo);
#else
    char *r = realpath(arquivo, NULL);
    snprintf(saida, tamanho, "%s", r != NULL ? r : arquivo);
    free(r);
#endif
}

/* Assinatura (hash FNV-1a de 64 bits) de todas as entradas usadas. */
static unsigned long long assinatura_entradas(int ntam)
{
    uint64_t h = 1469598103934665603ULL;
    int *v = aloca_int(TAMANHOS[ntam - 1]);
    for (int d = 0; d < NUM_DIST; d++)
        for (int t = 0; t < ntam; t++)
            for (int r = 0; r < repeticoes_para(TAMANHOS[t]); r++) {
                gerar_entrada(v, TAMANHOS[t], d, SEMENTE_BASE + (uint32_t)r);
                for (int i = 0; i < TAMANHOS[t]; i++) {
                    h ^= (uint32_t)v[i];
                    h *= 1099511628211ULL;
                }
            }
    free(v);
    return (unsigned long long)h;
}

/* Estatística */

typedef struct {
    double media, desvio, mediana, minimo, maximo, cv, ic95;
} Estatisticas;

static int compara_double(const void *pa, const void *pb)
{
    double a = *(const double *)pa, b = *(const double *)pb;
    return (a > b) - (a < b);
}

/* Valor crítico t de Student bicaudal para 95% de confiança. */
static double t_student_95(int graus_liberdade)
{
    static const double t[] = {0, 12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262,
                               2.228, 2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093,
                               2.086, 2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042};
    return (graus_liberdade >= 1 && graus_liberdade <= 30) ? t[graus_liberdade] : 1.960;
}

/* Atenção: reordena x (para calcular a mediana). */
static Estatisticas calcular_estatisticas(double *x, int r)
{
    Estatisticas e;
    double soma = 0, soma_quadrados = 0;
    for (int i = 0; i < r; i++)
        soma += x[i];
    e.media = soma / r;
    for (int i = 0; i < r; i++)
        soma_quadrados += (x[i] - e.media) * (x[i] - e.media);
    e.desvio = r > 1 ? sqrt(soma_quadrados / (r - 1)) : 0;
    qsort(x, (size_t)r, sizeof(double), compara_double);
    e.minimo = x[0];
    e.maximo = x[r - 1];
    e.mediana = (r % 2) ? x[r / 2] : (x[r / 2 - 1] + x[r / 2]) / 2;
    e.cv = e.media > 0 ? 100.0 * e.desvio / e.media : 0;
    e.ic95 = r > 1 ? t_student_95(r - 1) * e.desvio / sqrt((double)r) : 0;
    return e;
}

/* Inclinação da reta de regressão de log(X) contra log(n), nos índices [ini, fim). */
static double inclinacao_loglog(const double valores[], int ini, int fim)
{
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    int k = 0;
    for (int t = ini; t < fim; t++) {
        if (valores[t] <= 0)
            continue;
        double x = log((double)TAMANHOS[t]), y = log(valores[t]);
        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
        k++;
    }
    if (k < 2)
        return NAN;
    return (k * sxy - sx * sy) / (k * sxx - sx * sx);
}

/* Ambiente (vai para o cabeçalho do relatório) */

static void descrever_ambiente(FILE *f)
{
    char data[64];
    time_t agora = time(NULL);
    strftime(data, sizeof data, "%Y-%m-%d %H:%M:%S", localtime(&agora));
    fprintf(f, "Data/hora:    %s\n", data);
#if defined(__clang__)
    fprintf(f, "Compilador:   clang %s\n", __clang_version__);
#elif defined(__GNUC__)
    fprintf(f, "Compilador:   gcc %s\n", __VERSION__);
#elif defined(_MSC_VER)
    fprintf(f, "Compilador:   Microsoft C/C++ (MSVC) %d\n", _MSC_FULL_VER);
#else
    fprintf(f, "Compilador:   desconhecido\n");
#endif
#ifdef DUAS_VERSOES
    fprintf(f, "Compilacao:   duas versoes do algoritmo (com contadores para a contagem,\n"
               "              sem contadores para o tempo)\n");
#else
    fprintf(f, "ATENCAO:      compilado sem o rodar_todos: os contadores ficaram no codigo\n"
               "              medido e podem deixar o algoritmo ate 2x mais lento.\n"
               "              Os TEMPOS deste relatorio NAO sao confiaveis.\n");
#endif
#if defined(__GNUC__) && !defined(__OPTIMIZE__)
    fprintf(f, "ATENCAO:      compilado SEM otimizacao (-O0): os tempos nao sao representativos.\n");
#endif

#ifdef _WIN32
    char cpu[256] = "desconhecida";
    HKEY chave;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &chave) == ERROR_SUCCESS) {
        DWORD tam = (DWORD)sizeof cpu - 1;
        RegQueryValueExA(chave, "ProcessorNameString", NULL, NULL, (LPBYTE)cpu, &tam);
        RegCloseKey(chave);
    }
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof ms;
    GlobalMemoryStatusEx(&ms);
    fprintf(f, "CPU:          %s (%lu processadores logicos)\n", cpu, (unsigned long)si.dwNumberOfProcessors);
    fprintf(f, "RAM:          %s GB\n", br("%.1f", (double)ms.ullTotalPhys / 1073741824.0));
#else
    char cpu[256] = "desconhecida", linha[512];
    FILE *info = fopen("/proc/cpuinfo", "r");
    if (info != NULL) {
        while (fgets(linha, sizeof linha, info) != NULL) {
            if (strncmp(linha, "model name", 10) == 0 && strchr(linha, ':') != NULL) {
                snprintf(cpu, sizeof cpu, "%s", strchr(linha, ':') + 2);
                cpu[strcspn(cpu, "\n")] = '\0';
                break;
            }
        }
        fclose(info);
    }
    fprintf(f, "CPU:          %s (%ld processadores logicos)\n", cpu, sysconf(_SC_NPROCESSORS_ONLN));
#endif
}

/* 1. Teste de estabilidade (usa a versão com contadores) */

static void imprime_registros(FILE *f, const int *v, int n)
{
    for (int i = 0; i < n; i++)
        fprintf(f, " %d.%d", v[i] >> 16, v[i] & 0xFFFF);
    fprintf(f, "\n");
}

static void testar_estabilidade(FILE *rel, const char *nome, Ordenacao ordena_contando)
{
    static const struct {
        const char *descricao;
        int n;
        int chaves_distintas;
    } casos[] = {
        {"2 chaves distintas", 1000, 2},
        {"10 chaves distintas", 1000, 10},
        {"100 chaves distintas", 10000, 100},
    };
    static const int chaves_exemplo[] = {2, 1, 2, 1, 0, 2, 0, 1};
    int exemplo[8], instavel = 0;
    char topo = 0;
    Contadores cont;

    fprintf(rel, "\n------------------------------------------------------------------\n"
                 " 1. ESTABILIDADE\n"
                 "------------------------------------------------------------------\n"
                 "Estavel = elementos de chave igual mantem, apos a ordenacao, a mesma\n"
                 "ordem relativa da entrada.\n\n");

    for (int i = 0; i < 8; i++)
        exemplo[i] = (chaves_exemplo[i] << 16) | i;
    memset(&cont, 0, sizeof cont);
    cont.deslocamento = 16;
    cont.topo_pilha = cont.menor_endereco = (uintptr_t)&topo;
    fprintf(rel, "Exemplo (chave.posicao_original):\n  antes: ");
    imprime_registros(rel, exemplo, 8);
    ordena_contando(exemplo, 8, &cont);
    fprintf(rel, "  depois:");
    imprime_registros(rel, exemplo, 8);

    fprintf(rel, "\n%-22s %7s %11s   %s\n", "caso", "n", "violacoes", "resultado");
    for (size_t c = 0; c < sizeof casos / sizeof casos[0]; c++) {
        int n = casos[c].n;
        int *v = aloca_int(n);
        char *visto = calloc((size_t)n, 1);
        if (visto == NULL) {
            fprintf(stderr, "ERRO: falta de memoria\n");
            exit(EXIT_FAILURE);
        }
        estado_prng = SEMENTE_BASE;
        for (int i = 0; i < n; i++)
            v[i] = (int)(((xorshift32() % (uint32_t)casos[c].chaves_distintas) << 16) | (uint32_t)i);

        memset(&cont, 0, sizeof cont);
        cont.deslocamento = 16;
        cont.topo_pilha = cont.menor_endereco = (uintptr_t)&topo;
        ordena_contando(v, n, &cont);

        int correto = 1;
        long violacoes = 0;
        for (int i = 0; i < n; i++) {
            int p = v[i] & 0xFFFF;
            if (p >= n || visto[p])
                correto = 0;
            else
                visto[p] = 1;
            if (i > 0) {
                if ((v[i] >> 16) < (v[i - 1] >> 16))
                    correto = 0;
                else if ((v[i] >> 16) == (v[i - 1] >> 16) && p < (v[i - 1] & 0xFFFF))
                    violacoes++;
            }
        }
        if (!correto) {
            fprintf(stderr, "ERRO: %s produziu um resultado incorreto no teste de estabilidade\n", nome);
            exit(EXIT_FAILURE);
        }
        if (violacoes > 0)
            instavel = 1;
        fprintf(rel, "%-22s %7d %11ld   %s\n", casos[c].descricao, n, violacoes,
                violacoes > 0 ? "NAO ESTAVEL" : "nenhuma violacao");
        free(v);
        free(visto);
    }

    if (instavel)
        fprintf(rel, "\nConclusao: %s NAO e estavel.\n", nome);
    else
        fprintf(rel, "\nConclusao: nenhuma violacao encontrada. Isso e evidencia (nao prova)\n"
                     "de que %s e estavel; a prova vem da analise do codigo.\n", nome);
}

/* 2. Medições: contagem + tempo */

typedef struct {
    int repeticoes[NUM_DIST][NUM_TAMANHOS], entradas[NUM_DIST][NUM_TAMANHOS];
    Estatisticas estat[NUM_DIST][NUM_TAMANHOS];
    double tempo[NUM_DIST][NUM_TAMANHOS];
    double comparacoes[NUM_DIST][NUM_TAMANHOS];
    double movimentacoes[NUM_DIST][NUM_TAMANHOS];
    double pilha[NUM_DIST][NUM_TAMANHOS];
    double aux[NUM_DIST][NUM_TAMANHOS];
    int prof[NUM_DIST][NUM_TAMANHOS];
} Resultados;

static Resultados res;

static void medir(Ordenacao ordena_tempo, Ordenacao ordena_contando, const char *nome, int ntam, FILE *csv)
{
    double tempos[REPETICOES_N_PEQUENO > REPETICOES ? REPETICOES_N_PEQUENO : REPETICOES];
    int n_max = TAMANHOS[ntam - 1];
    int *original = aloca_int(n_max), *v = aloca_int(n_max), *referencia = aloca_int(n_max);

    fprintf(csv, "algoritmo;distribuicao;n;repeticoes;tempo_medio_ms;desvio_padrao_ms;mediana_ms;"
                 "minimo_ms;maximo_ms;cv_pct;ic95_ms;tempo_ns_por_n2;tempo_ns_por_nlog2n;"
                 "entradas_contagem;comparacoes;movimentacoes;profundidade_max;pilha_bytes;"
                 "mem_aux_bytes;memoria_total_bytes\n");

    for (int d = 0; d < NUM_DIST; d++) {
        for (int t = 0; t < ntam; t++) {
            int n = TAMANHOS[t], repeticoes = repeticoes_para(n), entradas = entradas_contagem(d);
            double soma_c = 0, soma_m = 0, pilha_max = 0, aux_max = 0;
            int prof_max = 0;

            /* Contagem */
            for (int e = 0; e < entradas; e++) {
                Contadores cont;
                char topo = 0;
                memset(&cont, 0, sizeof cont);
                cont.topo_pilha = cont.menor_endereco = (uintptr_t)&topo;
                gerar_entrada(original, n, d, SEMENTE_BASE + (uint32_t)e);
                memcpy(v, original, (size_t)n * sizeof(int));
                medicao_mem_atual = medicao_mem_pico = 0;
                ordena_contando(v, n, &cont);
                verificar(v, original, referencia, n, nome, NOME_DIST[d]);
                soma_c += (double)cont.comparacoes;
                soma_m += (double)cont.movimentacoes;
                if (cont.profundidade_max > prof_max)
                    prof_max = cont.profundidade_max;
                if ((double)(cont.topo_pilha - cont.menor_endereco) > pilha_max)
                    pilha_max = (double)(cont.topo_pilha - cont.menor_endereco);
                if ((double)medicao_mem_pico > aux_max)
                    aux_max = (double)medicao_mem_pico;
            }

            /* Tempo */
            for (int r = 0; r < repeticoes; r++) {
                gerar_entrada(original, n, d, SEMENTE_BASE + (uint32_t)r);
                memcpy(v, original, (size_t)n * sizeof(int));
                double inicio = agora_ms();
                ordena_tempo(v, n, NULL);
                tempos[r] = agora_ms() - inicio;
                verificar(v, original, referencia, n, nome, NOME_DIST[d]);
            }
            Estatisticas e = calcular_estatisticas(tempos, repeticoes);

            double nn = n;
            res.repeticoes[d][t] = repeticoes;
            res.entradas[d][t] = entradas;
            res.estat[d][t] = e;
            res.tempo[d][t] = e.media;
            res.comparacoes[d][t] = soma_c / entradas;
            res.movimentacoes[d][t] = soma_m / entradas;
            res.pilha[d][t] = pilha_max;
            res.aux[d][t] = aux_max;
            res.prof[d][t] = prof_max;

            fprintf(csv, "%s;%s;%d;%d;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%s;%s;%d;%s;%s;%s\n", nome,
                    NOME_DIST[d], n, repeticoes, br("%.6f", e.media), br("%.6f", e.desvio),
                    br("%.6f", e.mediana), br("%.6f", e.minimo), br("%.6f", e.maximo), br("%.2f", e.cv),
                    br("%.6f", e.ic95), br("%.6f", e.media * 1e6 / (nn * nn)),
                    br("%.6f", e.media * 1e6 / (nn * log2(nn))), entradas, br("%.1f", soma_c / entradas),
                    br("%.1f", soma_m / entradas), prof_max, br("%.0f", pilha_max), br("%.0f", aux_max),
                    br("%.0f", 4.0 * nn + aux_max + pilha_max));
            printf("  %-15s n = %6d   tempo medio %12s ms  (%2d rep., CV %5s%%)\n", NOME_DIST[d], n,
                   br("%.4f", e.media), repeticoes, br("%.1f", e.cv));
            fflush(stdout);
        }
    }
    free(original);
    free(v);
    free(referencia);
}

/* 3. Relatório */

static void secao(FILE *f, const char *titulo)
{
    fprintf(f, "\n------------------------------------------------------------------\n"
               " %s\n"
               "------------------------------------------------------------------\n", titulo);
}

static void tabela_crescimento(FILE *f, const char *titulo, double x[][NUM_TAMANHOS], int ntam)
{
    fprintf(f, "\n%s\n%-15s", titulo, "distribuicao");
    for (int t = 1; t < ntam; t++)
        fprintf(f, " %7d", TAMANHOS[t]);
    fprintf(f, " | %7s %9s\n", "todos", "4 maiores");
    for (int d = 0; d < NUM_DIST; d++) {
        fprintf(f, "%-15s", NOME_DIST[d]);
        for (int t = 1; t < ntam; t++)
            fprintf(f, " %7s", br("%.2f", x[d][t - 1] > 0 ? x[d][t] / x[d][t - 1] : NAN));
        fprintf(f, " | %7s %9s\n", br("%.2f", inclinacao_loglog(x[d], 0, ntam)),
                br("%.2f", ntam >= 6 ? inclinacao_loglog(x[d], ntam - 4, ntam) : NAN));
    }
}

static void escrever_relatorio(FILE *rel, int ntam)
{
    secao(rel, "2. COMPORTAMENTO DO ALGORITMO: TEMPO E OPERACOES");
    fprintf(rel, "tempo  = media das repeticoes, em ms (+-IC95 = intervalo de confianca de 95%%)\n"
                 "CV     = coeficiente de variacao (desvio/media); acima de ~5%% indica ruido\n"
                 "comparacoes e movimentacoes = media por entrada\n");
    for (int d = 0; d < NUM_DIST; d++) {
        fprintf(rel, "\n[%s]\n%9s %4s %14s %11s %6s %16s %16s\n", NOME_DIST[d], "n", "rep", "tempo(ms)",
                "+-IC95", "CV(%)", "comparacoes", "movimentacoes");
        for (int t = 0; t < ntam; t++) {
            Estatisticas e = res.estat[d][t];
            fprintf(rel, "%9d %4d %14s %11s %6s %16s %16s\n", TAMANHOS[t], res.repeticoes[d][t],
                    br("%.4f", e.media), br("%.4f", e.ic95), br("%.1f", e.cv),
                    br("%.0f", res.comparacoes[d][t]), br("%.0f", res.movimentacoes[d][t]));
        }
    }

    secao(rel, "3. COMPORTAMENTO NA MEMORIA");
    fprintf(rel, "prof   = maior profundidade de chamadas (1 = sem recursao)\n"
                 "pilha  = estimativa da pilha usada pelas chamadas (bytes)\n"
                 "aux    = memoria auxiliar alocada pelo algoritmo (bytes)\n"
                 "total  = vetor de entrada (4n bytes) + aux + pilha\n");
    for (int d = 0; d < NUM_DIST; d++) {
        fprintf(rel, "\n[%s]\n%9s %8s %8s %12s %12s %12s %12s\n", NOME_DIST[d], "n", "prof", "log2(n)",
                "pilha(B)", "aux(B)", "vetor(B)", "total(B)");
        for (int t = 0; t < ntam; t++) {
            double n = TAMANHOS[t];
            fprintf(rel, "%9d %8d %8s %12s %12s %12s %12s\n", TAMANHOS[t], res.prof[d][t],
                    br("%.1f", log2(n)), br("%.0f", res.pilha[d][t]), br("%.0f", res.aux[d][t]),
                    br("%.0f", 4.0 * n), br("%.0f", 4.0 * n + res.aux[d][t] + res.pilha[d][t]));
        }
    }

    secao(rel, "4. CRESCIMENTO OBSERVADO (complexidade na pratica)");
    fprintf(rel, "Colunas com n: razao de dobra X(n)/X(n/2)\n"
                 "   ~4 => n^2  |  ~2,1 a 2,2 => n log n  |  ~2 => n  |  ~1 => constante ou log n\n"
                 "Colunas 'todos' e '4 maiores': inclinacao em escala log-log (expoente)\n"
                 "   ~2 => n^2  |  ~1,1 => n log n  |  ~1 => n  |  ~0 => constante\n"
                 "No tempo, os menores n sofrem mais com ruido; prefira a coluna '4 maiores'.\n");
    tabela_crescimento(rel, "TEMPO", res.tempo, ntam);
    tabela_crescimento(rel, "COMPARACOES", res.comparacoes, ntam);
    tabela_crescimento(rel, "MOVIMENTACOES", res.movimentacoes, ntam);
    tabela_crescimento(rel, "PILHA (bytes)", res.pilha, ntam);
    tabela_crescimento(rel, "MEMORIA AUXILIAR (bytes)", res.aux, ntam);
}

/* Programa principal */

static void executar(const char *nome, Ordenacao ordena_tempo, Ordenacao ordena_contando, int ntam,
                     unsigned long long assinatura)
{
    char arq_rel[128], arq_csv[128], caminho[1024];
    snprintf(arq_rel, sizeof arq_rel, "resultado_%s.txt", nome);
    snprintf(arq_csv, sizeof arq_csv, "dados_%s.csv", nome);
    FILE *rel = criar_arquivo(arq_rel), *csv = criar_arquivo(arq_csv);

    fprintf(rel, "==================================================================\n"
                 " RESULTADO: %s\n"
                 "==================================================================\n", nome);
    if (TESTE_RAPIDO)
        fprintf(rel, "*** TESTE RAPIDO: serve so para conferir se funciona. NAO use no relatorio. ***\n");
    descrever_ambiente(rel);
    fprintf(rel, "Tamanhos (n): ");
    for (int t = 0; t < ntam; t++)
        fprintf(rel, " %d", TAMANHOS[t]);
    fprintf(rel, "\nDistribuicoes: aleatoria, crescente, decrescente, quase_ordenada (n/100 trocas),\n"
                 "              repetidos (10 valores distintos)\n");
    if (TESTE_RAPIDO)
        fprintf(rel, "Tempo:        %d repeticoes por (distribuicao, n)\n", REPETICOES_TESTE_RAPIDO);
    else
        fprintf(rel, "Tempo:        %d repeticoes (n <= %d) ou %d (n > %d) por (distribuicao, n)\n",
                REPETICOES_N_PEQUENO, LIMITE_N_PEQUENO, REPETICOES, LIMITE_N_PEQUENO);
    fprintf(rel, "Contagem:     %d entradas (1 para crescente e decrescente, que nao variam)\n"
                 "Sementes:     repeticao r usa a semente %u + r\n"
                 "Assinatura das entradas: 0x%016llx\n"
                 "  (deve ser IGUAL nos relatorios dos 4 algoritmos: prova que as entradas foram identicas)\n",
            ENTRADAS_CONTAGEM, SEMENTE_BASE, assinatura);

    printf("\n=== %s ===\n", nome);
    printf("Teste de estabilidade...\n");
    testar_estabilidade(rel, nome, ordena_contando);
    printf("Medicoes (contagem + tempo):\n");
    medir(ordena_tempo, ordena_contando, nome, ntam, csv);
    escrever_relatorio(rel, ntam);
    fclose(rel);
    fclose(csv);

    printf("\n");
    FILE *f = fopen(arq_rel, "r");
    if (f != NULL) {
        char linha[512];
        while (fgets(linha, sizeof linha, f) != NULL)
            fputs(linha, stdout);
        fclose(f);
    }
    printf("\nArquivos gravados:\n");
    caminho_completo(arq_rel, caminho, sizeof caminho);
    printf("  relatorio: %s\n", caminho);
    caminho_completo(arq_csv, caminho, sizeof caminho);
    printf("  planilha:  %s\n", caminho);
}

static void experimento(void)
{
    int ntam = num_tamanhos();
    double inicio = agora_ms();

    printf("%s - %s\n", NOME_ALGORITMO, TESTE_RAPIDO ? "TESTE RAPIDO" : "EXPERIMENTO COMPLETO");
#ifndef DUAS_VERSOES
    printf("ATENCAO: compilado sem o rodar_todos; os tempos nao serao confiaveis.\n");
#endif
    printf("Calculando a assinatura das entradas...\n");
    unsigned long long assinatura = assinatura_entradas(ntam);

    executar(NOME_ALGORITMO, ordenar, VERSAO_CONTAGEM, ntam, assinatura);
#ifdef VARIANTE
    executar(VARIANTE, ordenar_variante, VERSAO_CONTAGEM_VARIANTE, ntam, assinatura);
#endif
    printf("\nTempo total: %s min\n", br("%.1f", (agora_ms() - inicio) / 60000.0));
}

#ifdef _WIN32
static DWORD WINAPI thread_experimento(LPVOID arg)
{
    (void)arg;
    experimento();
    return 0;
}

int main(void)
{
    HANDLE h = CreateThread(NULL, PILHA_EXPERIMENTO, thread_experimento, NULL,
                            STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
    if (h == NULL) {
        fprintf(stderr, "ERRO: nao foi possivel criar a thread do experimento\n");
        return EXIT_FAILURE;
    }
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
    return 0;
}
#else
static void *thread_experimento(void *arg)
{
    (void)arg;
    experimento();
    return NULL;
}

int main(void)
{
    pthread_t thread;
    pthread_attr_t atributos;
    pthread_attr_init(&atributos);
    pthread_attr_setstacksize(&atributos, PILHA_EXPERIMENTO);
    if (pthread_create(&thread, &atributos, thread_experimento, NULL) != 0) {
        fprintf(stderr, "ERRO: nao foi possivel criar a thread do experimento\n");
        return EXIT_FAILURE;
    }
    pthread_join(thread, NULL);
    return 0;
}
#endif

#endif /* versão de tempo + main */
#endif /* MEDICAO_H */
