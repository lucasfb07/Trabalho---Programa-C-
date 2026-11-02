/*
 * AVL TREE SORT (Tree Sort com árvore AVL, Adelson-Velsky e Landis, 1962)
 */
#define NOME_ALGORITMO "avl_tree_sort"
#define TESTE_RAPIDO 0   /* 1 = teste rápido, 0 = experimento completo */
#include "medicao.h"

typedef struct No {
    int valor;
    int altura;
    struct No *esq, *dir;
} No;

/* Altura de uma subárvore; a árvore vazia (NULL) tem altura 0. */
static int altura(const No *no)
{
    return no != NULL ? no->altura : 0;
}

static void atualiza_altura(No *no)
{
    int ae = altura(no->esq), ad = altura(no->dir);
    no->altura = 1 + (ae > ad ? ae : ad);
}

/* Rotações: só trocam ponteiros e preservam a ordem em-ordem. */
static No *rotacao_direita(No *raiz)
{
    No *nova = raiz->esq;
    raiz->esq = nova->dir;
    nova->dir = raiz;
    atualiza_altura(raiz);
    atualiza_altura(nova);
    return nova;
}

static No *rotacao_esquerda(No *raiz)
{
    No *nova = raiz->dir;
    raiz->dir = nova->esq;
    nova->esq = raiz;
    atualiza_altura(raiz);
    atualiza_altura(nova);
    return nova;
}

/* Recalcula a altura de 'raiz' e rotaciona se as alturas dos filhos diferem em mais de 1. */
static No *balanceia(No *raiz)
{
    atualiza_altura(raiz);
    int fator = altura(raiz->esq) - altura(raiz->dir);

    if (fator > 1) {
        if (altura(raiz->esq->esq) < altura(raiz->esq->dir))
            raiz->esq = rotacao_esquerda(raiz->esq);
        return rotacao_direita(raiz);
    }
    if (fator < -1) {
        if (altura(raiz->dir->dir) < altura(raiz->dir->esq))
            raiz->dir = rotacao_direita(raiz->dir);
        return rotacao_esquerda(raiz);
    }
    return raiz;
}

/* Insere o nó 'novo' na subárvore 'raiz' e devolve a nova raiz dessa subárvore. */
static No *insere(No *raiz, No *novo, Contadores *cont)
{
    ENTRA_CHAMADA();
    if (raiz == NULL) {
        raiz = novo;
    } else {
        if (MENOR(novo->valor, raiz->valor))
            raiz->esq = insere(raiz->esq, novo, cont);
        else
            raiz->dir = insere(raiz->dir, novo, cont);
        raiz = balanceia(raiz);
    }
    SAI_CHAMADA();
    return raiz;
}

/* Percurso em-ordem (esquerda, nó, direita): grava os elementos em v, já ordenados. */
static void em_ordem(const No *raiz, int *v, int *prox, Contadores *cont)
{
    ENTRA_CHAMADA();
    if (raiz != NULL) {
        em_ordem(raiz->esq, v, prox, cont);
        v[(*prox)++] = raiz->valor;
        MOVE(1);
        em_ordem(raiz->dir, v, prox, cont);
    }
    SAI_CHAMADA();
}

void ordenar(int *v, int n, Contadores *cont)
{
    ENTRA_CHAMADA();
    if (n > 1) {
        size_t bytes = (size_t)n * sizeof(No);
        No *nos = mem_alocar(bytes);
        No *raiz = NULL;
        for (int i = 0; i < n; i++) {
            nos[i].valor = v[i];
            MOVE(1);
            nos[i].altura = 1;
            nos[i].esq = nos[i].dir = NULL;
            raiz = insere(raiz, &nos[i], cont);
        }
        int prox = 0;
        em_ordem(raiz, v, &prox, cont);
        mem_liberar(nos, bytes);
    }
    SAI_CHAMADA();
}
