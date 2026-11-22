# Decisões de projeto

## Representações

- **Matriz de adjacência em bitset** (`GrafoMatriz`): um único bloco contíguo de `n·W` palavras de 64 bits (`W = ⌈n/64⌉`), com índices em 64 bits. Ocupa `n·W·8 + 4n` bytes (1 bit por par). Teste de adjacência em O(1); vizinhos percorridos palavra a palavra com `ctz`/`clz`, em ordem crescente ou decrescente. m e graus contam só bits ligados pela primeira vez, o que descarta repetições.
- **Lista de adjacência** (`GrafoLista`): `vector<vector<int32_t>>`, cada lista reservada pelo grau bruto, ordenada e sem repetições (`sort` + `unique`; `shrink_to_fit` só se perdeu elementos). Adjacência por busca binária. Memória: `Σ capacidades·4 + n·sizeof(vector) + 4n`.
- As duas têm a **mesma interface não virtual**. `Grafo` guarda um `std::variant` das duas e cada função pública faz um único `std::visit`; os algoritmos são templates sobre o tipo concreto, sem chamada virtual nem `std::function` nos laços. A comparação de tempo mede só a estrutura de dados.
- API pública com vértices 1..n; índices internos 0..n−1. Erros por exceção (`ErroGrafo`).

## Leitura

- Arquivo inteiro lido com `fread` e parseado manualmente. Os `\n` são contados antes para reservar exatamente os arrays temporários, e o buffer é liberado antes de construir a estrutura.
- Aceita `\r\n`, tabulações, espaços extras e linhas vazias. `n` = primeiro número da primeira linha não vazia. A terceira coluna é o peso (Parte 2, abaixo); a partir da quarta, colunas são ignoradas.
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

## Parte 2: pesos e Dijkstra

- **Leitura com peso:** a primeira linha de aresta decide se o arquivo é ponderado (3 ou mais colunas). Nesse caso toda linha precisa de peso `double` finito (`std::from_chars`, independente de locale); peso ausente, texto, `nan` ou `inf` dão erro com o número da linha. Arquivo sem peso continua como na Parte 1 (todos os pesos valem 1). **Aresta repetida fica com o menor peso**, o único que importa para caminho mínimo; laço é descartado mesmo com peso negativo.
- **Lista:** implementada como o **vetor de adjacência** da aula 3 (um vetor de vizinhos por vértice, contíguo na memória: mesma complexidade da lista encadeada, mas melhor uso do cache e sem um ponteiro por vizinho). Os pesos ficam em vetores **paralelos** às listas de vizinhos (`pesos[v][i]` é o peso da aresta até `adjacencia[v][i]`), ordenados junto com os vizinhos e alocados **só em grafo ponderado**: grafo sem peso tem a mesma memória e o mesmo desempenho da Parte 1. Optamos por isso em vez de mudar para pares (vizinho, peso), que dobraria o tamanho do laço interno da BFS/DFS mesmo sem peso.
- **Matriz:** o bitset continua dizendo se a aresta existe; em grafo ponderado ganha um bloco denso `n·n` de `double` (+`8n²` bytes). Já no W_3 (n = 100.000) seriam 80 GB, então a matriz ponderada só serve a grafos pequenos (testes); a checagem de viabilidade antes de alocar conta esses bytes.
- **Um único Dijkstra** (aula 8), template sobre a representação e sobre a **fronteira** — a estrutura que guarda as estimativas, as atualiza e extrai o mínimo, com a interface `inserir`, `diminuir_chave`, `vazia`, `extrair_minimo`:
  - **vetor:** um vetor de chaves com `+∞` fora da fronteira; cada extração varre os n vértices → O(n²) no total. `diminuir_chave` é O(1);
  - **heap:** heap binário mínimo próprio, em vetor (aula 9), **indexado** por `pos[v]`: achar `v` é O(1), então `diminuir_chave` sobe o elemento em O(log n) → O((n + m) log n). Só os vértices descobertos entram no heap, e cada vértice ocupa no máximo uma posição.
  - **Por que não a "saída fácil"** (reinserir o vértice a cada melhora e ignorar, ao extrair, quem já saiu — o que `std::priority_queue` obriga, pois não muda chaves): o heap passa a ter uma entrada por relaxamento bem-sucedido, até O(m) em vez de O(n). O custo continua O(m log n) (log m ≤ 2 log n), mas a memória do heap e o número de extrações crescem com m — no W_6 seriam até ~73 milhões de entradas contra 10 milhões de vértices — e cada entrada obsoleta ainda paga uma extração O(log m).
- **Desempate por (distância, id)** nas duas fronteiras e relaxamento estrito (`<`), com vizinhos em ordem crescente: vetor e heap produzem exatamente as mesmas distâncias, pais e ordem de exploração (conferido nos testes e em cada medição).
- **Pesos negativos em Dijkstra:** recusados antes de qualquer alocação; peso zero é aceito. Para pesos negativos, usar Bellman-Ford.
- **Resultado:** distância, pai (árvore de caminhos mínimos) e ordem de exploração por vértice; `caminho(destino)` sobe pelos pais. Distâncias gravadas na forma mais curta que reconstrói o `double`.
- **Rede de colaboração:** nomes lidos de `id,nome` cortando na primeira vírgula (há nomes com vírgula); busca exata em UTF-8 (`Éva Tardos`), recusando nome ausente ou ambíguo.
- **Tempo (P2-2):** 100 origens sorteadas como na Parte 1 (semente 42); uma execução de aquecimento por fila; cada chamada cronometrada com `steady_clock`. O vetor roda nas **primeiras k** dessas origens, com k fixo por grafo para caber em cerca de 1,5 h (W_3: 100, W_4: 10); a tabela compara com a média do heap **nas mesmas origens**. No W_5 e no W_6 (n = 10⁷), uma execução do vetor levaria cerca de um dia, e no W_7 (n = 5·10⁷) semanas: ela é interrompida após 1% dos vértices explorados (0,01% no W_7) e extrapolada linearmente (cada extração custa uma varredura de n), registrada como **estimativa**. No W_7 o heap usa 30 origens (~90 s cada).
- **Escalabilidade (W_7):** com 50 milhões de vértices e grau médio 2,4, a lista com um vetor de vizinhos e um de pesos por vértice custa ~110 bytes por vértice só em cabeçalhos de `vector` e na sobra mínima do alocador; o pico chega a ~6,4 GB (de 7,7 GB do WSL). Um formato compactado (CSR: vetores únicos de vizinhos, pesos e deslocamentos) reduziria a estrutura para ~1,7 GB; não foi adotado para manter a mesma lista da Parte 1.

## Preparação para a parte 3

- `carregar` já recebia `direcionado`; na Parte 3 esse modo passou a aceitar arcos apenas no sentido indicado.
- Novos algoritmos sobre grafos com peso só precisam de `para_cada_vizinho_com_peso`, disponível nas duas representações.

## Ferramentas

## Parte 3: direção, ciclos negativos e limites

- **Direção.** O parâmetro público `direcionado` vale para lista e matriz. Em modo dirigido cada linha insere só `u -> v`; em modo antigo continua inserindo ambos os sentidos. Duplicatas conservam o menor peso. Laços dirigidos são preservados para detectar ciclo negativo; o descarte de laços do modo não dirigido permanece por compatibilidade.
- **Bellman-Ford por destino.** Construímos adjacências de entrada para relaxar `d[u] > peso(u,v) + d[v]` a partir do destino. `sucessor[u] = v` compõe a árvore de caminhos até ele. Distância `+inf` significa inalcançável. A detecção global usa superorigem virtual de custo zero e fila de relaxamento; um ciclo no grafo de predecessores com soma negativa comprova ciclo negativo. Se o grafo é fortemente conexo, um único ciclo negativo torna todos os pares indefinidos. Nos demais casos, a rodada extra e uma busca reversa marcam com `-inf` as origens afetadas. O flag global é separado do flag que afeta o destino.
- **Duas otimizações do Bellman-Ford.** (1) Encerrar quando uma rodada não altera distâncias. (2) Relaxar apenas os vértices que mudaram na rodada anterior, evitando varrer arcos não relevantes. A primeira aparece em resumo antigo; a consulta aos slides disponíveis não permitiu confirmar que a segunda foi exatamente a ensinada em aula. A escolha é tecnicamente fundamentada e preserva as mesmas relações de relaxamento. O limite de pior caso continua `O(nm)` e o espaço adicional é `O(n+m)`.
- **Floyd-Warshall.** Distâncias são atualizadas no próprio bloco `n*n`, em vez de guardar uma matriz por estágio. Essa otimização está explícita na Aula 20 de Teoria dos Grafos (slides de 2013, seção “Algoritmo Melhorado”). Um segundo bloco `n*n` de inteiros guarda o próximo vértice para reconstruir caminhos e árvores de origem. São `12n²` bytes para os dois blocos, além do grafo. A diagonal negativa detecta ciclo; pares que podem atravessar tal ciclo recebem `-inf`. Tempo `O(n³)`.
- **Viabilidade.** A interface barra Floyd-Warshall acima de `10¹⁰` iterações por padrão, liberáveis com `--forcar`, e compara o bloco `12n²` com 80% da RAM disponível. O roteiro experimental também perfila o `.gz` antes de descompactar e registrar carregamentos inviáveis. O limite de trabalho é preventivo, não uma medição. Os casos W1–W5 exigem tratar resultados não calculados com marcação explícita.
- **Validação.** Os testes anteriores permanecem. Os novos testes verificam direção, laços, paralelos, pesos positivos/negativos/zero, caminhos, ciclos e 100 grafos dirigidos aleatórios pequenos contra Floyd de três matrizes como referência independente. As medições do estudo excluem leitura, escrita e descompactação, repetindo cada operação cronometrada dez vezes.
- **Complemento W4/W5.** A carga integral da representação geral excedeu a RAM livre, mas seus pesos não negativos de precisão centesimal permitem uma variante de Bellman–Ford com arcos compactos de 12 bytes. A conversão do `.gz` para binário ocorre antes da medição; o programa `estudo_compacto` carrega os arcos compactos e mede dez chamadas até o destino 100, sem I/O dentro do cronômetro. Também calcula as matrizes 5×5 por cinco chamadas adicionais. Cada chamada encerra após uma varredura sem melhoria; caminhos selecionados são reconstruídos e seus pesos conferidos. O método foi verificado contra as 25 distâncias de W2 já produzidas pela biblioteca. Esta variante faz varredura integral de arcos, portanto seus tempos não são diretamente comparáveis aos da versão com vértices ativos usada em W1–W3. Nenhuma matriz de W4/W5 foi gerada por Floyd–Warshall.

## Ferramentas

- C++17, g++, CMake ≥ 3.16; Release com `-O2 -DNDEBUG`, sem `-march=native`; preset `debug-asan` com `-fsanitize=address,undefined`.
- Testes com doctest 2.5.3: exemplos das aulas e do enunciado, comparação lista × matriz, referências independentes (DFS recursiva, BFS com `std::queue`, diâmetro ingênuo, Bellman-Ford para o Dijkstra, `std::set` para as fronteiras) e milhares de grafos aleatórios.
