/*
 * QUICK SORT (Hoare, 1962), com a partição de Hoare/Wirth usada em Ziviani
 */
#define NOME_ALGORITMO "quick_sort"
#define VARIANTE "quick_sort_pivo_fixo"
#define TESTE_RAPIDO 0   /* 1 = teste rápido, 0 = experimento completo */
#include "medicao.h"

/* Devolve o VALOR do pivô para o subvetor v[esq..dir]. */
static int escolhe_pivo(const int *v, int esq, int dir, int pivo_fixo, Contadores *cont)
{
    if (pivo_fixo)
        return v[esq];

    const int *a = &v[esq], *b = &v[esq + (dir - esq) / 2], *c = &v[dir];
    if (MENOR(*a, *b)) {
        if (MENOR(*b, *c))
            return *b;
        return MENOR(*a, *c) ? *c : *a;
    }
    if (MENOR(*a, *c))
        return *a;
    return MENOR(*b, *c) ? *c : *b;
}

/* Partição de Hoare/Wirth. Ao final: v[esq..*pj] <= x <= v[*pi..dir] e *pj < *pi. */
static void particao(int *v, int esq, int dir, int pivo_fixo, int *pi, int *pj, Contadores *cont)
{
    int i = esq, j = dir;
    int x = escolhe_pivo(v, esq, dir, pivo_fixo, cont);
    MOVE(1);
    do {
        while (MENOR(v[i], x))
            i++;
        while (MENOR(x, v[j]))
            j--;
        if (i <= j) {
            TROCA(v[i], v[j]);
            i++;
            j--;
        }
    } while (i <= j);
    *pi = i;
    *pj = j;
}

/* Versão principal: mediana de três + recursão na menor parte. */
static void quick_sort(int *v, int esq, int dir, Contadores *cont)
{
    ENTRA_CHAMADA();
    while (esq < dir) {
        int i, j;
        particao(v, esq, dir, 0, &i, &j, cont);
        if (j - esq < dir - i) {
            quick_sort(v, esq, j, cont);
            esq = i;
        } else {
            quick_sort(v, i, dir, cont);
            dir = j;
        }
    }
    SAI_CHAMADA();
}

/* Variante de pior caso: pivô = primeiro elemento e recursão nas duas partes. */
static void quick_sort_pivo_fixo(int *v, int esq, int dir, Contadores *cont)
{
    ENTRA_CHAMADA();
    if (esq < dir) {
        int i, j;
        particao(v, esq, dir, 1, &i, &j, cont);
        if (esq < j)
            quick_sort_pivo_fixo(v, esq, j, cont);
        if (i < dir)
            quick_sort_pivo_fixo(v, i, dir, cont);
    }
    SAI_CHAMADA();
}

void ordenar(int *v, int n, Contadores *cont)
{
    ENTRA_CHAMADA();
    quick_sort(v, 0, n - 1, cont);
    SAI_CHAMADA();
}

void ordenar_variante(int *v, int n, Contadores *cont)
{
    ENTRA_CHAMADA();
    quick_sort_pivo_fixo(v, 0, n - 1, cont);
    SAI_CHAMADA();
}
