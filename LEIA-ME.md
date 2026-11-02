# Análise de algoritmos de ordenação em C

```
./
├── rodar_todos.c                 ← executor: compila e roda os algoritmos
├── src/
│   ├── medicao.h                 ← medição e relatório (único, usado pelos 4; não precisa mexer)
│   ├── selection_sort.c
│   ├── quick_sort.c              ← versão principal + variante de pior caso (pivô fixo)
│   ├── binary_insertion_sort.c
│   └── avl_tree_sort.c           ← Tree Sort com árvore AVL (balanceada)
├── build/                        ← executáveis (gerado)
├── resultados/                   ← relatórios .txt e planilhas .csv (gerado)
└── .vscode/                      ← tarefas do VS Code
```

Os quatro algoritmos usam o mesmo `src/medicao.h`, o que garante a mesma medição
e as mesmas entradas para todos. A análise teórica de cada algoritmo está no
comentário do início do seu arquivo `.c`.

---

## Como executar

Não há arquivo `.bat`: o próprio `rodar_todos.c` compila e executa os algoritmos,
um depois do outro (nunca em paralelo, para um não atrapalhar o tempo do outro).

1. Compile o executor (uma vez), na pasta do projeto:

   ```
   gcc rodar_todos.c -o rodar_todos
   ```

   Com o Visual Studio (MSVC), faça isso no **Developer Command Prompt**: `cl rodar_todos.c`.
2. Rode todos os algoritmos, ou só alguns:

   ```
   .\rodar_todos
   .\rodar_todos quick_sort avl_tree_sort
   ```

   Nomes válidos: `selection_sort`, `quick_sort`, `binary_insertion_sort`, `avl_tree_sort`.

No VS Code: abra a pasta do projeto e use **Terminal → Executar Tarefa…**
(ou Ctrl+Shift+B). Para o MSVC, abra o VS Code a partir do Developer Command Prompt.

O programa usa o gcc se ele estiver no PATH; senão, o `cl` (MSVC). Compila com
otimização, roda o experimento e mostra o relatório no terminal. Os executáveis
ficam em `build\` e os resultados em `resultados\`.

**Teste rápido:** no início de `src/<algoritmo>.c`, troque `#define TESTE_RAPIDO 0` por `1`
para uma execução de poucos segundos (n até 8.000, 3 repetições). Volte para `0`
antes do experimento que vai para o relatório.

> **Não use** o botão ▶ ("Run C/C++ File") nem o F6 da extensão *C/C++ Compile Run*.
> Eles compilam sem otimização e numa versão só, e os tempos saem errados. Se
> isso acontecer, o próprio relatório mostra um aviso.

### Tempo do experimento completo

Medido num Ryzen 7 5825U:

| Algoritmo | Tempo aproximado |
|---|---|
| Selection Sort | ~7 min (Θ(n²) em todas as entradas) |
| Quick Sort (com a variante de pior caso) | ~25 s |
| Binary Insertion Sort | ~10 s |
| AVL Tree Sort | ~6 s |

**Antes de rodar:** deixe o notebook **na tomada**, use o modo de energia "Melhor
desempenho" e feche os outros programas. Nos testes, o mesmo código variou até
~25% de uma sessão para outra por causa do gerenciamento de energia do notebook.
Por isso, rode os 4 algoritmos **na mesma sessão e nas mesmas condições**.

---

## O que é gerado (na pasta `resultados\`)

| Arquivo | Conteúdo |
|---|---|
| `resultado_<algoritmo>.txt` | **Relatório completo**, também mostrado no terminal |
| `dados_<algoritmo>.csv` | **Planilha**: uma linha por (distribuição, n) com todas as medidas, para gráficos |

O Quick Sort gera também `resultado_quick_sort_pivo_fixo.txt` e `dados_quick_sort_pivo_fixo.csv`.

O relatório tem 4 seções:

1. **Estabilidade:** exemplo pequeno e teste com chaves repetidas.
2. **Comportamento do algoritmo:** para cada distribuição, tempo médio
   (± intervalo de confiança de 95%), coeficiente de variação, comparações e movimentações.
3. **Comportamento na memória:** profundidade de recursão, pilha, memória
   auxiliar e memória total.
4. **Crescimento observado:** razão de dobra X(n)/X(n/2) e inclinação log-log
   para tempo, comparações, movimentações, pilha e memória auxiliar.
   - razão ≈ 4 e inclinação ≈ 2 indicam Θ(n²);
   - razão ≈ 2,1–2,2 e inclinação ≈ 1,1 indicam Θ(n log n).

A planilha usa `;` como separador e `,` como decimal, então abre direto no Excel
em português. As colunas `tempo_ns_por_n2` e `tempo_ns_por_nlog2n` servem para o
gráfico de normalização: a que ficar constante indica a complexidade.

### Conferir que as entradas foram idênticas

Cada relatório traz a linha `Assinatura das entradas: 0x…`. Ela é um *hash* de
todas as entradas usadas e deve ser **igual nos relatórios dos 4 algoritmos**.

---

## Como o programa mede (para a metodologia do trabalho)

- **Entradas:** n = 500, 1.000, 2.000, …, 64.000 (dobrando) × 5 distribuições:
  aleatória, crescente, decrescente, quase ordenada (crescente + n/100 trocas) e
  muitos repetidos (10 valores distintos). Gerador pseudoaleatório próprio
  (xorshift32); a repetição r usa a semente 2026 + r.
- **Contagem:** comparações entre elementos e movimentações (atribuições de
  elementos; uma troca conta 3). Usa 3 entradas por configuração, e 1 para
  crescente e decrescente, que não variam. Essa execução também serve de aquecimento.
- **Tempo:** 10 repetições (30 para n ≤ 4.000), cada uma com entrada nova, usando
  o relógio de alta resolução do sistema. Só a ordenação fica dentro do cronômetro.
- **Duas versões do algoritmo:** o `rodar_todos` compila o mesmo `.c` duas vezes,
  com contadores (para a contagem) e sem contadores (para o tempo). Motivo: nos
  testes, deixar os contadores no código, mesmo desligados, dobrou o tempo do
  Quick Sort.
- **Memória:**
  - *profundidade*: maior número de chamadas empilhadas (1 = sem recursão);
  - *pilha*: estimativa em bytes, pela distância entre endereços de variáveis locais;
  - *auxiliar*: bytes que o algoritmo aloca, medidos com exatidão;
  - *total*: vetor (4n bytes) + auxiliar + pilha.
- **Corretude:** toda ordenação é conferida contra o `qsort` da biblioteca padrão.
  Se algum resultado estiver errado, o programa para com erro.
- **Estabilidade:** cada elemento guarda a chave e a posição original; depois de
  ordenar, verifica-se se chaves iguais mantiveram a ordem original.

---

## Valores de referência para validar

Igualdades **exatas**, conferidas nos testes. Servem de prova de que a
implementação corresponde à teoria:

| Algoritmo | Grandeza | Valor teórico exato |
|---|---|---|
| Selection | comparações (qualquer entrada) | n(n−1)/2 |
| Selection | movimentações (qualquer entrada) | 3(n−1) |
| Binary Insertion | movimentações, crescente | 2(n−1) |
| Binary Insertion | movimentações, decrescente | 2(n−1) + n(n−1)/2 |
| Binary Insertion | movimentações, qualquer entrada | 2(n−1) + nº de inversões |
| AVL Tree | movimentações (qualquer entrada) | 2n (cópia para o nó + volta ao vetor) |
| AVL Tree | memória auxiliar | 24n bytes (n nós de 24 bytes) |
| Quick (pivô fixo), crescente/decrescente | profundidade | n |
| Selection e Binary Insertion | profundidade / memória auxiliar | 1 / 0 |

Valores aproximados esperados:
- Binary Insertion, entrada aleatória: ≈ n(n−1)/4 inversões e ≈ n·log₂n − 1,44n comparações;
- AVL Tree: ≤ n·log₂n comparações e profundidade ≤ 1,44·log₂(n+2) + 3 (altura da AVL
  mais as chamadas de `ordenar` e do percurso);
- Quick (principal): profundidade ≤ log₂n + 2.

---

## Decisões de implementação (citar na metodologia)

- **Selection:** a troca é feita sempre, mesmo quando o menor já está na posição i
  (como em Ziviani). Assim o número de movimentações é exato.
- **Binary Insertion:** a busca binária devolve a posição *após* os iguais, o que
  garante estabilidade. O deslocamento usa `memmove`.
- **Quick (principal):** partição de Hoare/Wirth, pivô por mediana de três e
  recursão só na menor parte, o que garante pilha O(log n).
- **Quick (pivô fixo):** pivô = primeiro elemento e recursão nas duas partes,
  para mostrar o pior caso de tempo, Θ(n²), e de pilha, O(n). O experimento roda
  numa *thread* com pilha de 128 MB para suportar a profundidade n.
- **AVL Tree:** cada elemento é inserido numa árvore AVL (inserção recursiva com
  rotações) e depois o percurso em-ordem devolve os elementos ordenados ao vetor.
  Os n nós vêm de um único bloco alocado de uma vez; a alocação está dentro do
  tempo medido. Em empate, desce para a direita (estável); as rotações só trocam
  ponteiros e preservam a ordem em-ordem, então não contam como movimentação.
- **Todos:** elementos `int` de 32 bits, mesmo compilador e mesmas opções (`/O2` ou `-O2`).

---

## Se o Windows bloquear o programa

Com o **Controle Inteligente de Aplicativos** (Smart App Control) do Windows 11
ligado, um programa recém-compilado pode ser bloqueado com a mensagem *"Uma
política de Controle de Aplicativo bloqueou este arquivo"*. Rode o `rodar_todos`
de novo: ele recompila e gera um executável novo, que costuma ser aceito. Se o bloqueio
persistir, rode em outro computador. Mudar essa proteção é decisão sua; em muitas
versões do Windows 11, desligá-la é irreversível sem reinstalar o sistema.

## Compilar um algoritmo à mão (opcional)

Na pasta do projeto, com `build\` e `resultados\` já criadas:

```
gcc -O2 -std=c11 -DDUAS_VERSOES -DCONTAR -c src/selection_sort.c -o build/selection_sort_contagem.o
gcc -O2 -std=c11 -DDUAS_VERSOES src/selection_sort.c build/selection_sort_contagem.o -o build/selection_sort.exe -lm
cd resultados
..\build\selection_sort.exe
```

Rodar de dentro de `resultados\` faz os arquivos serem gravados lá. No Linux,
acrescente `-pthread` no segundo comando, use `selection_sort` sem `.exe` e
`../build/selection_sort` para executar.
