# grafos

Biblioteca de grafos em C++17 (COS 242 — Teoria dos Grafos, Trabalho Parte 1): leitura de grafos não direcionados, representação por **lista** ou **matriz de adjacência**, BFS, DFS, distância, componentes conexas, estatísticas de grau e diâmetro (ingênuo, aproximado e exato). Inclui o programa de linha de comando `grafos`, o programa `estudos` e scripts para os estudos de caso.

## Requisitos (WSL2 / Ubuntu)

```bash
sudo apt install build-essential cmake python3
```

> **Importante:**
> - Mantenha o projeto no sistema de arquivos do Linux (ex.: `~/grafos`), **não** em `/mnt/c/...`. O acesso ao disco do Windows pelo WSL é muito mais lento e distorce os tempos de leitura.
> - O WSL2 usa por padrão só parte da RAM do Windows. Confira com `free -h`. Para aumentar, crie ou edite `%UserProfile%\.wslconfig` no Windows (ex.: `[wsl2]` e `memory=12GB`) e rode `wsl --shutdown` no PowerShell.

## Compilar e testar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

São gerados `build/libgrafos.a`, `build/grafos`, `build/estudos` e `build/testes`. O `ctest` roda os testes da biblioteca (doctest) e os testes de ponta a ponta dos programas.

Com AddressSanitizer e UBSan (mais lento):

```bash
cmake --preset debug-asan
cmake --build --preset debug-asan -j
ctest --preset debug-asan
```

## Formato dos grafos

Primeira linha: número de vértices `n`. Cada linha seguinte: uma aresta `u v`, com vértices de 1 a n. Laços e arestas repetidas são ignorados (e contados); linhas vazias, `\r\n`, tabulações e colunas extras são aceitas.

```
5
1 2
2 5
5 3
4 5
1 5
```

## Programa `grafos`

```bash
build/grafos --help
build/grafos estatisticas dados/grafo_1.txt [--repr lista|matriz] -o estatisticas.txt
build/grafos bfs dados/grafo_1.txt --origem 1 [--repr lista|matriz] -o bfs.txt
build/grafos dfs dados/grafo_1.txt --origem 1 [--repr lista|matriz] -o dfs.txt
build/grafos distancia dados/grafo_1.txt 10 20 [--repr lista|matriz] [--caminho]
build/grafos diametro dados/grafo_1.txt [--repr lista|matriz] [--ingenuo | --aproximado | --exato] [--limite 600]
```

- Representação padrão: `lista`. `diametro` usa `--exato` por padrão, com limite de 1800 s.
- Antes de carregar, o programa lê só o `n` e recusa (com erro) se a matriz, ou o mínimo da lista, não couber em 80% da memória disponível.
- O terminal mostra `n`, `m`, laços e repetidas ignorados e o tempo de cálculo (sem leitura e escrita).
- Arquivo da árvore: `vertice pai nivel ordem` (raiz com pai 0; não alcançados com `-1 -1 -1`).

## Usando a biblioteca

```cpp
#include "grafos/grafos.hpp"

grafos::InfoLeitura info;
grafos::Grafo g = grafos::carregar("dados/grafo_1.txt", grafos::Representacao::Lista, false, &info);
grafos::ArvoreBusca arvore = grafos::bfs(g, 1);
int32_t d = grafos::distancia(g, 10, 20);           // -1 = infinito
grafos::Diametro diam = grafos::diametro_exato(g, 600.0);
grafos::escrever_arvore(arvore, "bfs.txt");
```

Link com `grafos_lib` (CMake) ou `build/libgrafos.a`, com `include/` no caminho de headers.

## Estudos de caso

1. Coloque os grafos em `dados/` (a pasta está no `.gitignore`). Podem ficar compactados: `grafo_1.txt.gz` ou `grafo_1_txt.gz`. O script descompacta mantendo o original.
2. Compile em Release (acima) e **feche programas pesados** (inclusive editores com assistentes), para não interferir nas medições.
3. Rode:

   ```bash
   scripts/rodar_estudos.sh                    # todos os grafos, lista e depois matriz
   scripts/rodar_estudos.sh --limite-ifub 600  # opções extras são repassadas ao estudos
   ```

   - Resultados em `resultados/<grafo>_<repr>.csv` (e o arquivo de estatísticas de cada grafo); logs em `resultados/logs/`.
   - Combinações que já têm CSV são **puladas**, então dá para interromper e retomar. Para refazer uma, apague o CSV dela.
   - Se uma execução falhar, o script registra e segue; no fim mostra o resumo e sai com código ≠ 0.
   - Matriz que não cabe na memória é registrada como `inviavel`, sem carregar.
4. Consolide:

   ```bash
   python3 scripts/consolidar.py
   ```

   Gera `resultados/consolidado.csv` (uma linha por grafo) e `resultados/tabelas.md` (memória, tempos de BFS/DFS, pais, distâncias, componentes e diâmetro).

Para rodar um único grafo e representação:

```bash
build/estudos --grafo dados/grafo_1.txt --repr lista --saida resultados [--semente 42] [--pausar] [--limite-ifub 1800]
```

`--pausar` imprime o PID e espera Enter depois da carga, para conferir a memória com `top` ou `ps`.

## Grafos sintéticos (`gerar_grafo`)

Gera um grafo aleatório uniforme no formato da disciplina (m arestas com extremos distintos sorteados em 1..n; a semente torna o arquivo reprodutível):

```bash
mkdir -p /tmp/grafos-sintetico
build/gerar_grafo --n 1000000 --m 5000000 --semente 1 -o /tmp/grafos-sintetico/g_1M_5M.txt
# subshell com memória virtual limitada a 4 GB: um erro vira bad_alloc, não OOM do WSL
(ulimit -v 4000000; build/estudos --grafo /tmp/grafos-sintetico/g_1M_5M.txt --repr lista \
    --saida /tmp/grafos-sintetico/resultados --limite-ifub 600)
```

**Não** grave grafos sintéticos em `dados/` nem os resultados deles em `resultados/`: o `rodar_estudos.sh` processa tudo o que houver em `dados/`.

## Organização

```
include/grafos/   headers públicos (algoritmos template)
src/              leitura, representações, escrita, utilitários de sistema
app/              programa grafos
estudos/          programa estudos
scripts/          rodar_estudos.sh e consolidar.py
tests/            testes doctest, testes dos programas e grafos de teste
third_party/      doctest.h
DECISOES.md       decisões de projeto
```
