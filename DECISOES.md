# Decisões de projeto

## Representações

- **Matriz de adjacência em bitset** (`GrafoMatriz`): um único bloco contíguo de `n·W` palavras de 64 bits (`W = ⌈n/64⌉`), com índices em 64 bits. Ocupa `n·W·8 + 4n` bytes (1 bit por par). Teste de adjacência em O(1); vizinhos percorridos palavra a palavra com `ctz`/`clz`, em ordem crescente ou decrescente. m e graus contam só bits ligados pela primeira vez, o que descarta repetições.
- **Lista de adjacência** (`GrafoLista`): `vector<vector<int32_t>>`, cada lista reservada pelo grau bruto, ordenada e sem repetições (`sort` + `unique`; `shrink_to_fit` só se perdeu elementos). Adjacência por busca binária. Memória: `Σ capacidades·4 + n·sizeof(vector) + 4n`.
- As duas têm a **mesma interface não virtual**. `Grafo` guarda um `std::variant` das duas e cada função pública faz um único `std::visit`; os algoritmos são templates sobre o tipo concreto, sem chamada virtual nem `std::function` nos laços. A comparação de tempo mede só a estrutura de dados.
- API pública com vértices 1..n; índices internos 0..n−1. Erros por exceção (`ErroGrafo`).

## Leitura

- Arquivo inteiro lido com `fread` e parseado manualmente. Os `\n` são contados antes para reservar exatamente os arrays temporários, e o buffer é liberado antes de construir a estrutura.
- Aceita `\r\n`, tabulações, espaços extras e linhas vazias. `n` = primeiro número da primeira linha não vazia. Colunas a partir da terceira são ignoradas (peso, na parte 2).
- Erros citam a linha física: vértice fora de 1..n, linha com um só número, texto inválido, `n` ausente, `n ≤ 0` ou acima de `int32_t`.
- Laços e arestas repetidas são ignorados e contados; m é o número de arestas distintas. `linhas_aresta` inclui laços e repetições.
- A lista temporária é destruída e `malloc_trim(0)` devolve a memória ao sistema antes de retornar, para o RSS medido refletir só a estrutura.

## Buscas

- Vizinhos sempre em **ordem crescente**: árvores determinísticas e iguais nas duas representações.
- **BFS** (aula 4): marca ao **descobrir**; fila em array de tamanho n com dois índices; vetores `pai`/`nivel`/`ordem` inicializados dentro da busca.
- **DFS** (slides, iterativa): pilha de pares (vértice, pai); marca = **explorado**, feita ao remover da pilha; vizinhos empilhados em ordem decrescente (o menor sai primeiro), sem empilhar os já marcados. `nivel` = profundidade na árvore. Produz a mesma árvore e ordem da DFS recursiva (verificado nos testes).

## Estatísticas, componentes e distância

- Grau mínimo, máximo e mediana por **histograma de graus** (O(n + grau máximo), sem ordenar); grau médio = 2m/n.
- Componentes (aula 6): um array de rótulos e **uma única fila** reaproveitada por todas as BFS, percorrendo os vértices em ordem crescente (sem custo O(n) por componente). Listas montadas por *bucket* (já saem crescentes); componentes por tamanho decrescente, empate pelo menor vértice.
- Distância: BFS com parada antecipada ao descobrir o destino; componentes diferentes → "infinito".
- Números em arquivos com `std::to_chars` (ponto decimal independente de locale).

## Diâmetro

- Em grafo desconexo, **maior diâmetro entre as componentes**. Só BFS como primitiva.
- **BFS auxiliar** com área de trabalho alocada uma vez; ao terminar, desmarca só os vértices visitados (pela fila). O mais distante é o último vértice descoberto.
- **Ingênuo:** BFS de cada vértice (n BFS); só para testes e grafos pequenos.
- **Aproximado (4-sweep):** por componente, BFS a partir do vértice de maior grau r → a1; BFS(a1) → b1; m1 = meio do caminho b1–a1; BFS(m1) → a2; BFS(a2) → b2; m2 = meio de b2–a2; BFS(m2). Como ecc(x) ≤ D ≤ 2·ecc(x): limite inferior = maior excentricidade vista; limite superior = min(2·ecc(r), 2·ecc(m1), 2·ecc(m2), tamanho−1). 5 BFS por componente.
- **Exato (iFUB, Crescenzi et al., 2013):** usa u = m2 do 4-sweep, L = ecc(u) e as camadas F_i (distância i de u). Da camada mais distante para a mais próxima, calcula B_i = maior excentricidade em F_i. Vértices em camadas < i têm excentricidade ≤ 2(i−1); logo, se max(lb, B_i) > 2(i−1), o valor é exato, senão ub = min(ub, 2(i−1)) (limites superiores se combinam por mínimo: nunca piora o do 4-sweep).
- **Duas fases no exato.** Fase A: 4-sweep em todas as componentes, em ordem decrescente de tamanho, até a primeira com tamanho−1 ≤ melhor valor atual (as seguintes são menores). Fase B: laço do iFUB, na mesma ordem, só nas componentes cujo limite superior ainda supera o melhor valor global. Assim toda componente relevante tem um limite superior próprio antes do trecho caro. Na fase B, a BFS(m2) é refeita (1 BFS a mais por componente, contada em `num_bfs`) em vez de guardar as camadas de todas as componentes.
- O aproximado roda só a fase A. Grafo sem arestas: diâmetro 0.
- **Limite de tempo** (padrão 30 min): consultado antes de cada BFS. Se esgotar, devolve [inferior, superior] com superior = max(limites das componentes já varridas, refinados pelo iFUB quando houve, tamanho−1 da primeira componente não varrida). Assim, interromper o iFUB da maior componente não faz o superior saltar para o tamanho da segunda.

## Programas e metodologia de medição

- `grafos`: comandos `estatisticas`, `bfs`, `dfs`, `distancia`, `diametro` (padrão `--exato`, limite de 1800 s).
- **Viabilidade antes de alocar** (`grafos` e `estudos`): lê só o `n` e compara com 80% de `MemAvailable` a estimativa da matriz (`n·W·8 + 4n`) ou o mínimo da lista (`n·(sizeof(vector) + 4)`, só pelo n: o tamanho do arquivo daria um limite folgado demais). Matriz inviável é resultado de estudo (CSV `inviavel`); lista inviável é erro (sem CSV). Evita que um `n` absurdo esgote a memória do WSL.
- `estudos` processa **um grafo e uma representação por execução**, para a memória de uma não contaminar a outra.
- **Memória:** `VmRSS` (`/proc/self/status`) antes e depois da carga, já sem o temporário; registra a diferença e a estimativa teórica `bytes_estrutura()`. MB = 1024² bytes. Matriz inviável (estimativa > 80% de `MemAvailable`) não é carregada e fica registrada como `inviavel`.
- **Tempo:** 100 vértices distintos sorteados com `mt19937_64` (semente 42) e **rejeição própria** (sem `uniform_int_distribution`, para ser reprodutível entre compiladores), iguais para BFS e DFS nas duas representações. Uma BFS de aquecimento não contada; depois 100 BFS e 100 DFS, cada chamada cronometrada com `steady_clock` (inclui a inicialização interna da busca, exclui qualquer escrita). Média e desvio padrão amostral.
- Resultados em CSV `chave,valor`, gravado num `.tmp` e renomeado só ao final: uma execução interrompida não é tomada como concluída. `rodar_estudos.sh` pula combinações com CSV existente, registra falhas e segue; `consolidar.py` gera o CSV consolidado e as tabelas.

## Preparação para as partes 2 e 3

- A matriz ponderada será outra classe (`double`, com um valor especial para "aresta inexistente"), pois pesos não cabem em bitset; a lista ponderada guardará pares (vizinho, peso).
- As novas classes só precisam implementar a mesma interface para reaproveitar os algoritmos template.
- `carregar` já recebe `direcionado` (hoje recusa) e trabalha com arcos (cada aresta gera (u,v) e (v,u)); a terceira coluna do arquivo já é tolerada.

## Ferramentas

- C++17, g++, CMake ≥ 3.16; Release com `-O2 -DNDEBUG`, sem `-march=native`; preset `debug-asan` com `-fsanitize=address,undefined`.
- Testes com doctest 2.5.3: exemplos das aulas e do enunciado, comparação lista × matriz, referências independentes (DFS recursiva, BFS com `std::queue`, diâmetro ingênuo) e milhares de grafos aleatórios.
