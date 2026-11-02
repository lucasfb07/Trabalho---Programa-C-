/*
 * SELECTION SORT (ordenação por seleção)
 */
#define NOME_ALGORITMO "selection_sort"
#define TESTE_RAPIDO 0   /* 1 = teste rápido, 0 = experimento completo */
#include "medicao.h"

void ordenar(int *v, int n, Contadores *cont)
{
    ENTRA_CHAMADA();
    for (int i = 0; i < n - 1; i++) {
        int menor = i;
        for (int j = i + 1; j < n; j++)
            if (MENOR(v[j], v[menor]))
                menor = j;
        TROCA(v[i], v[menor]);
    }
    SAI_CHAMADA();
}
