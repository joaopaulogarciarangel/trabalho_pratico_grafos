# Guia objetivo para apresentar e responder perguntas — Parte 3

## O que foi feito

- Biblioteca C++17 para grafos dirigidos e ponderados; BFS, DFS e Dijkstra adaptados para respeitar a direção. Dijkstra rejeita pesos negativos.
- Bellman–Ford de todas as origens até um destino e Floyd–Warshall para todos os pares, com caminhos reconstruídos e tratamento de ciclos negativos.
- 70 testes e 26.998 verificações, inclusive com referência independente.
- As três distâncias até 100 **e as 25 distâncias entre {10,20,30,40,50}** foram calculadas em W1–W5. As tabelas 5×5 vieram de Bellman–Ford, não de Floyd.

## Algoritmos

**Bellman–Ford por destino:** inicializar `d[t]=0` e, para cada arco `u→v`, fazer `d[u]←min(d[u],peso(u,v)+d[v])`. Sucessores reconstroem caminhos. As duas melhorias da Aula 14 são: guardar `M[v]` em um vetor em vez de `M[i,v]` em uma matriz por iteração; atualizar a partir dos vértices alterados na rodada anterior e parar quando não houver alterações. A biblioteca implementa ambas. Pior caso `O(nm)` por destino, memória `O(n+m)`.

**Floyd–Warshall:** com `k` no laço externo, `D[i][j]←min(D[i][j],D[i][k]+D[k][j])`. Uma matriz de distâncias é atualizada no lugar e outra guarda próximos vértices. Tempo `O(n³)`; memória `O(n²)`, aproximadamente `12n²` bytes para as matrizes.

**Ciclos:** um par `u→t` tem custo `−∞` se `u` alcança um ciclo negativo e o ciclo alcança `t`. Sem caminho, `+∞`. Em Floyd, uma diagonal negativa revela ciclos; os pares afetados são propagados.

## Números para memorizar

| Grafo | Vértices | 10→100 | 20→100 | 30→100 | BF→100, média de 10 chamadas |
|---|---:|---:|---:|---:|---:|
| W1 | 25 mil | −∞ | −∞ | −∞ | 0,060178 s |
| W2 | 25 mil | 10,47 | 8,48 | 8,71 | 0,079931 s |
| W3 | 100 mil | 4,63 | 5,42 | 4,67 | 0,782222 s |
| W4 | 1 milhão | 9,04 | 8,83 | 9,23 | 3,891435 s* |
| W5 | 10 milhões | 37,85 | 42,63 | 37,86 | 18,064925 s* |

\* W4/W5 foram medidos com variante compacta, arcos carregados antes do relógio. São dez execuções válidas para essa variante, **sem comparação direta** com os tempos W1–W3 da implementação original.

Em W1, o ciclo `15108→3965→15108` tem pesos `0,04` e `−2,47`, soma `−2,43`. Os 25 mil vértices estão na mesma componente fortemente conexa. Assim, **todas** as 25 entradas da tabela 5×5 são `−∞`. As tabelas W2–W5 estão no relatório e nos apêndices dos slides. Em W4, `d(10,50)=8,24`; em W5, `d(10,50)=44,91`.

## O que ficou sem execução por recursos

| Grafo | Memória aproximada só das matrizes de Floyd |
|---|---:|
| W1/W2 | 7,50 GB |
| W3 | 120 GB |
| W4 | 12 TB |
| W5 | 1,2 PB |

Havia cerca de **1,06 GB livre** na medição. A representação original também excedia essa memória na carga de W4/W5 (picos estimados em 2,30 e 4,13 GB). A variante compacta permitiu Bellman–Ford nesses grafos, com dez medições e as tabelas 5×5. Ela **não** torna Floyd viável: ele ainda precisa de matrizes quadráticas e tempo cúbico. Bellman–Ford para **todos os pares** reais também não foi executado, principalmente pelo tempo de processar tantos destinos. É possível cronometrá-lo sem guardar a matriz de resultados; armazenar a resposta completa exigiria espaço adicional enorme.

Comparação de todos os pares apenas em grafo sintético de 100 vértices e 581 arcos, dez rodadas: Floyd `0,001082 s`; BF `0,005809 s`; razão `5,37×` **nessa instância**. Não há resultado comparativo medido em W1–W5.

## Perguntas prováveis

**Por que W1 dá `−∞`?** Repetir o ciclo de soma negativa reduz o custo sem limite; a forte conexidade permite usá-lo entre quaisquer vértices.

**Todo ciclo negativo afeta todos os pares?** Não. A origem precisa alcançar o ciclo e o ciclo, o destino. W1 satisfaz isso para todos.

**Por que não usar só Dijkstra?** Ele exige pesos não negativos. Bellman–Ford aceita pesos negativos e identifica efeitos de ciclos negativos.

**As tabelas 5×5 comprovam Floyd?** Não. São distâncias por Bellman–Ford para cinco destinos. Floyd foi implementado e testado em instâncias pequenas, mas não rodou nos cinco arquivos reais.

**Como W4/W5 couberam?** Cada arco ocupou 12 bytes, com vetores de distâncias e sucessores. Os arcos foram carregados antes da medição; cada caminho selecionado teve seu peso conferido. Em W2, a variante reproduziu as 25 entradas da implementação original.

**A memória foi o único impedimento?** Não. Floyd também exige tempo cúbico. BF para todos os pares exigiria processar até 10 milhões de destinos em W5; para medir só o tempo, não é necessário guardar as `10¹⁴` respostas possíveis.

**As duas otimizações de BF são quais?** (1) Um vetor `M[v]` em vez de uma matriz `M[i,v]`; (2) atualizar a partir dos vértices alterados na rodada anterior e parar se não houver mudança. São as duas melhorias descritas nas páginas 11–12 da Aula 14 e implementadas na biblioteca.

**O estudo 2 e o 4 foram cumpridos literalmente?** Não. Respondemos às consultas de distância do estudo 2, mas sem executar Floyd nos arquivos reais; a comparação completa do estudo 4 não foi feita neles. O relatório registra os limites.

**Onde está o código?** A Parte 3 está no pacote e na branch `parte-3`. Antes de apresentar, abram `https://github.com/joaopaulogarciarangel/trabalho_pratico_grafos/tree/parte-3` e confirmem que a branch aparece online. O link da página inicial do repositório abre a branch `main`, que contém apenas as partes anteriores.
