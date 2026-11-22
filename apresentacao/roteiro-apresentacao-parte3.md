# Roteiro de fala — Teoria dos Grafos, Parte 3

**Use com:** `Teoria-dos-Grafos-Parte-3.pdf`. **Duração:** cerca de 7min40s. Gabriel: slides 1–4; João Paulo: slides 5–8. Apêndices A1/A2 somente para perguntas.

## Slide 1 — Abertura (0:00–0:35) · Gabriel

“Boa tarde. Somos Gabriel e João Paulo. Nesta terceira parte, estendemos nossa biblioteca para grafos dirigidos com pesos, implementamos Bellman–Ford e Floyd–Warshall e analisamos os cinco arquivos fornecidos. Vamos explicar as decisões, os resultados e os limites da execução.”

## Slide 2 — Representação (0:35–1:15) · Gabriel

“Cada linha `u v peso` representa um arco de `u` para `v`. Não criamos automaticamente o caminho de volta. Na lista de adjacência, `v` fica entre as saídas de `u`; na matriz, registramos a posição `u,v`. O modo não dirigido anterior continua disponível. Arcos paralelos ficam com o menor peso e laços dirigidos são preservados.”

## Slide 3 — Algoritmos anteriores (1:15–2:00) · Gabriel

“Adaptamos BFS, DFS e as duas versões de Dijkstra para percorrer somente arcos de saída. No exemplo do slide, não se chega a `c` partindo de `s`, pois o arco aponta no sentido contrário. Dijkstra rejeita pesos negativos. Conferimos caminhos reconstruídos e seus custos. A suíte passou em 72 casos e 40.969 verificações, incluindo grafos aleatórios comparados com uma referência independente.”

## Slide 4 — Bellman–Ford (2:00–3:20) · Gabriel

“O enunciado pede distâncias de todas as origens até um destino `t`. Começamos com `d[t]=0` e, para cada arco `u→v`, tentamos melhorar `d[u]` com o peso do arco mais `d[v]`. Guardamos o sucessor para reconstruir o caminho. Implementamos as duas melhorias da aula: guardar as distâncias num vetor, em vez de uma matriz por rodada; e atualizar a partir dos vértices alterados, parando se uma rodada não mudar nada.

“Detectar um ciclo negativo no grafo inteiro é diferente de saber se ele afeta uma consulta. A resposta é menos infinito se a origem alcança o ciclo e, a partir dele, alcança o destino. Sem caminho, é mais infinito.”

## Slide 5 — Floyd–Warshall (3:20–4:10) · João Paulo

“Floyd–Warshall calcula todos os pares. Para cada vértice intermediário `k`, perguntamos se passar por ele melhora o custo de `i` até `j`. Atualizamos uma única matriz de distâncias no lugar; outra matriz guarda os próximos vértices para reconstrução. Uma diagonal negativa indica ciclo negativo, e identificamos os pares afetados. É o algoritmo melhorado da aula 13. O custo é cúbico em tempo e quadrático em memória. Nos grafos de 25 mil vértices, guardamos só a matriz de distâncias, 5 gigabytes, e a percorremos em blocos, em paralelo: a mesma recorrência, mas aproveitando a memória cache.”

## Slide 6 — Destino 100 (4:10–5:25) · João Paulo

“Calculamos as distâncias de 10, 20 e 30 até 100 nos cinco grafos. Em W1, todas são menos infinito. Encontramos um ciclo de peso total menos 2,43 e verificamos que os 25 mil vértices estão na mesma componente fortemente conexa; o ciclo afeta qualquer par.

“W2 e W3 deram valores finitos. Em W4, as três distâncias são 9,04, 8,83 e 9,23; em W5, 37,85, 42,63 e 37,86. Todos os grafos rodaram na própria biblioteca, na mesma máquina. A preparação, montar os arcos de entrada e detectar o ciclo, é feita uma vez e fica fora do cronômetro. Cada consulta levou de 16 milissegundos no W2 a 6,45 segundos no W5. No W1, 32 microssegundos, porque o ciclo já foi detectado na preparação.”

## Slide 7 — Todos os pares (5:25–6:55) · João Paulo

“O segundo estudo pede as distâncias entre 10, 20, 30, 40 e 50 usando Floyd. No W2, a matriz vem do Floyd–Warshall e foi conferida par a par com o Bellman–Ford. No W1, todas as 25 entradas são menos infinito. Do W3 em diante, a matriz do Floyd teria de 80 gigabytes a 800 terabytes, então usamos cinco execuções de Bellman–Ford; estão nos apêndices.

“Depois, comparamos os dois para todos os pares. No W2, o Floyd–Warshall levou 12 minutos por execução, em média de dez; o Bellman–Ford para os 25 mil destinos levou 6,7 minutos: 1,8 vez mais rápido, mesmo com o Floyd usando 12 threads e o Bellman–Ford uma. No W1, o Bellman–Ford respondeu todos os pares em 0,35 segundo, e o Floyd levou 14,5 minutos, média de três execuções. A razão: o Floyd faz n³ passos qualquer que seja o grafo; o Bellman–Ford, com as melhorias da aula, para em poucas rodadas e só toca os arcos dos vértices que mudaram. Do W3 ao W5 os tempos são estimativas, marcadas na tabela. No ensaio pequeno, com 100 vértices, os dois ficam na mesma ordem de grandeza.”

## Slide 8 — Conclusão (6:55–7:45) · João Paulo

“Entregamos os algoritmos dirigidos, a reconstrução de caminhos, os testes e as distâncias solicitadas nos cinco grafos. O achado mais marcante é W1: a forte conexidade faz um ciclo negativo afetar todos os pares.

“Medimos o Floyd–Warshall no W1 e no W2 e comparamos com o Bellman–Ford para todos os pares: em grafos grandes e esparsos, o Bellman–Ford repetido venceu. Do W3 em diante, o Floyd não cabe na memória, e os tempos ficam estimados. Obrigado; os apêndices têm as tabelas para perguntas.”

## Se perguntarem pelos apêndices

- **A1:** matrizes W2 (Floyd–Warshall) e W3 (Bellman–Ford). Em W2, `d(10,20)=11,73` e `d(20,10)=3,84`: direção importa.
- **A2:** matrizes W4/W5 por Bellman–Ford, na própria biblioteca; distâncias conferidas por verificador independente.
- **Por que só 3 execuções do Floyd no W1?** Cada uma leva cerca de 14,5 minutos (de 13,7 a 16,1); o relatório diz isso. As demais medidas têm 10.
