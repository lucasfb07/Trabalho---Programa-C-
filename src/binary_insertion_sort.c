/*
 * BINARY INSERTION SORT (inserção com busca binária)
 */
#define NOME_ALGORITMO "binary_insertion_sort"
#define TESTE_RAPIDO 0   /* 1 = teste rápido, 0 = experimento completo */
#include "medicao.h"

void ordenar(int *v, int n, Contadores *cont)
{
    ENTRA_CHAMADA();
    for (int i = 1; i < n; i++) {
        int x = v[i];
        MOVE(1);

        /* busca binária */
        int esq = 0, dir = i;
        while (esq < dir) {
            int meio = esq + (dir - esq) / 2;
            if (MENOR(x, v[meio]))
                dir = meio;
            else
                esq = meio + 1;
        }

        memmove(&v[esq + 1], &v[esq], (size_t)(i - esq) * sizeof(int));
        MOVE(i - esq);
        v[esq] = x;
        MOVE(1);
    }
    SAI_CHAMADA();
}
