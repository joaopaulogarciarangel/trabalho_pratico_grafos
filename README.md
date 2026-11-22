# grafos

Biblioteca de grafos em C++17 (COS 242 — Teoria dos Grafos, Partes 1 a 3): representações por **lista** ou **matriz de adjacência**, grafos não dirigidos e dirigidos, BFS, DFS, Dijkstra, Bellman–Ford e Floyd–Warshall. Inclui o programa de linha de comando `grafos`, estudos de caso, testes e materiais de apresentação.

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

São gerados `build/libgrafos.a`, `build/grafos`, `build/estudos`, `build/estudos_dijkstra` e `build/testes`. O `ctest` roda os testes da biblioteca (doctest) e os testes de ponta a ponta dos programas.

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

**Com pesos (Parte 2):** uma terceira coluna com o peso da aresta (número real, ex.: `1 2 0.1`, `3 4 -9.5`). A primeira linha de aresta decide: se tiver peso, todas precisam ter. Aresta repetida fica com o menor peso.

## Programa `grafos`

```bash
build/grafos --help
build/grafos estatisticas dados/grafo_1.txt [--repr lista|matriz] -o estatisticas.txt
build/grafos bfs dados/grafo_1.txt --origem 1 [--repr lista|matriz] -o bfs.txt
build/grafos dfs dados/grafo_1.txt --origem 1 [--repr lista|matriz] -o dfs.txt
build/grafos distancia dados/grafo_1.txt 10 20 [--repr lista|matriz] [--caminho]
build/grafos diametro dados/grafo_1.txt [--repr lista|matriz] [--ingenuo | --aproximado | --exato] [--limite 600]
build/grafos dijkstra dados/grafo_W_3.txt --origem 10 [--fila heap|vetor] [--repr lista|matriz] \
    [--destinos 20,30,40,50,60] [-o dijkstra.txt]
```

- `dijkstra` usa o heap por padrão; imprime distância, número de arestas e caminho até cada destino, e com `-o` grava `vertice pai distancia` (não alcançados: `-1 infinito`). Grafo com peso negativo é recusado com mensagem. `distancia` continua contando arestas (ignora pesos).

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

grafos::Grafo w = grafos::carregar("dados/grafo_W_3.txt", grafos::Representacao::Lista);
grafos::ResultadoDijkstra r = grafos::dijkstra(w, 10, grafos::FilaDijkstra::Heap);  // ou Vetor
double d20 = r.dist[20 - 1];                         // +infinito se inalcancavel
std::vector<int32_t> c = grafos::caminho(r, 20);     // 10 ... 20; vazio se inalcancavel
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

O `rodar_estudos.sh` ignora os arquivos da Parte 2 (`grafo_W_*` e `rede_*`).

## Estudos de caso da Parte 2 (Dijkstra)

1. Coloque em `dados/` os grafos com pesos (`grafo_W_3.txt`, ... ou `.txt.gz`) e a rede de colaboração (`rede_colaboracao.txt` e `rede_colaboracao_vertices.txt`).
2. Compile em Release e feche programas pesados.
3. Rode (cerca de 5 h com W_3 a W_7; o W_7 chega a ~6,4 GB de RAM, então feche os outros programas):

   ```bash
   scripts/rodar_estudos_dijkstra.sh
   python3 scripts/consolidar_dijkstra.py
   ```

   - Resultados em `resultados/parte2/<grafo>_dijkstra.csv` e `resultados/parte2/rede_colaboracao_dijkstra.csv`; logs em `resultados/parte2/logs/`; tabelas em `resultados/parte2/tabelas_dijkstra.md`.
   - Por grafo: distância e caminho de 10 a 20, 30, 40, 50 e 60; tempo médio do Dijkstra com heap (100 origens sorteadas; 30 no W_7) e com vetor (as primeiras k delas: W_3 100, W_4 10; do W_5 ao W_7 o tempo do vetor é estimado por uma execução parcial). Os valores de cada grafo ficam nas tabelas no topo do script.
   - Rede: distância e caminho (por nome) de Edsger W. Dijkstra a Alan M. Turing, J. B. Kruskal, Jon M. Kleinberg, Éva Tardos e Daniel R. Figueiredo.
   - Como na Parte 1, CSVs existentes são pulados (dá para retomar).

Para um único grafo:

```bash
build/estudos_dijkstra --grafo dados/grafo_W_3.txt --saida resultados/parte2 [--semente 42] \
    [--k-heap 100] [--k-vetor 100] [--estimar-vetor 0.01]
build/estudos_dijkstra --rede dados/rede_colaboracao.txt --nomes dados/rede_colaboracao_vertices.txt \
    --saida resultados/parte2
```

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

## Parte 3: grafos direcionados, Bellman-Ford e Floyd-Warshall

O [relatório da Parte 3](relatorio-parte3-Grafos.pdf) tem cinco páginas. Os [slides, roteiro e guia de estudo](apresentacao/) estão na pasta `apresentacao/`; os resultados medidos estão em `resultados/parte3/`.

A Parte 3 preserva o comportamento não direcionado por padrão. Passe `--direcionado`
para interpretar cada linha `u v peso` como um único arco `u -> v`. A lista e a
matriz respeitam esse sentido; BFS, DFS e os dois Dijkstras percorrem apenas
arcos de saída. Dijkstra continua exigindo pesos não negativos. Arcos paralelos
ficam com o menor peso; laços em grafos direcionados são mantidos, inclusive
os negativos.

Bellman-Ford recebe um **destino** e devolve distâncias de todos os vértices até
ele, sucessores para os caminhos e dois indicadores de ciclo negativo: em
qualquer componente e afetando o destino. Ele percorre os arcos na direção
inversa durante o relaxamento. Floyd-Warshall usa uma única matriz de
distâncias e outra de próximos vértices, totalizando `12*n*n` bytes para esses
dois blocos. `+inf` indica ausência de caminho e `-inf` indica distância
indefinida por ciclo negativo alcançável. A rotina `arvore_origem` reconstrói
os pais a partir da matriz de próximos vértices.

As duas melhorias de Bellman-Ford da Aula 14 estão implementadas: um vetor
`M[v]` de distâncias em vez da matriz por iteração `M[i,v]`; e a atualização
a partir dos vértices alterados na rodada anterior, com parada quando não há
mudanças.

Exemplos após `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` e
`cmake --build build -j`:

```bash
build/grafos bfs dados/exemplo.txt --origem 10 --direcionado -o bfs.txt
build/grafos dijkstra dados/exemplo.txt --origem 10 --direcionado --destinos 20,30
build/grafos bellman-ford dados/exemplo.txt --destino 100 --direcionado --origem 10 -o arvore.txt
build/grafos floyd-warshall dados/pequeno.txt --direcionado --origem 10 --destino 20
ctest --test-dir build --output-on-failure
```

No Windows com MSYS2 UCRT64, `powershell -File scripts/build_windows.ps1 -Test`
gera os executáveis e executa os testes. Floyd-Warshall no CLI barra por padrão
instâncias acima de 10 bilhões de iterações cúbicas; `--forcar` permite uma
execução deliberada quando houver recursos. A checagem de memória permanece.

Os estudos da Parte 3 usam `estudos_parte3` (um CSV `chave,valor` por grafo) e o
script `scripts/rodar_estudos_parte3.sh`, que procura `grafo_W_1..5.txt` (ou `.gz`)
em `dados/parte3/` e grava em `resultados/parte3/`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
scripts/rodar_estudos_parte3.sh          # FW_REP=10 BF_TODOS_REP=10 AMOSTRA=100 por padrão
```

Por grafo: Bellman-Ford até 100 (distâncias de 10, 20, 30, caminhos e média de dez
chamadas), os 25 pares de `{10,20,30,40,50}` por Floyd-Warshall (conferidos com
cinco chamadas de Bellman-Ford), média do Floyd-Warshall e do Bellman-Ford para
todos os pares. A carga do grafo e a preparação do Bellman-Ford (arcos de entrada e
detecção global de ciclo negativo, feitas uma vez) ficam fora do cronômetro das
consultas e são registradas à parte. O Floyd-Warshall só roda se `8n²` bytes couberem
na memória (W_1 e W_2, 5 GB; 12 min por rodada no W_2 e 14,5 no W_1, medido com ); nos demais o CSV registra
`nao_cabe_na_memoria`. O Bellman-Ford para todos os pares é medido inteiro em W_1 e
W_2 e estimado por amostra de 100 destinos em W_3–W_5. O script também roda uma
instância sintética de 100 vértices (`validacao_100`), com todos os estudos completos.
CSVs existentes são pulados, o que permite retomar.

## Organização

```
include/grafos/   headers públicos (algoritmos template)
src/              leitura, representações, escrita, utilitários de sistema
app/              programa grafos
estudos/          programas estudos e estudos_dijkstra
scripts/          rodar_estudos(_dijkstra).sh e consolidar(_dijkstra).py
tests/            testes doctest, testes dos programas e grafos de teste
third_party/      doctest.h
DECISOES.md       decisões de projeto
```
