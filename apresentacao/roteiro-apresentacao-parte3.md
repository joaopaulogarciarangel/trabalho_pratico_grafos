# Roteiro de fala — Teoria dos Grafos, Parte 3

**Use com:** `Teoria-dos-Grafos-Parte-3-atualizado.pdf`. **Duração:** cerca de 7min40s. Gabriel: slides 1–4; João Paulo: slides 5–8. Apêndices A1/A2 somente para perguntas.

## Slide 1 — Abertura (0:00–0:35) · Gabriel

“Boa tarde. Somos Gabriel e João Paulo. Nesta terceira parte, estendemos nossa biblioteca para grafos dirigidos com pesos, implementamos Bellman–Ford e Floyd–Warshall e analisamos os cinco arquivos fornecidos. Vamos explicar as decisões, os resultados e os limites da execução.”

## Slide 2 — Representação (0:35–1:15) · Gabriel

“Cada linha `u v peso` representa um arco de `u` para `v`. Não criamos automaticamente o caminho de volta. Na lista de adjacência, `v` fica entre as saídas de `u`; na matriz, registramos a posição `u,v`. O modo não dirigido anterior continua disponível. Arcos paralelos ficam com o menor peso e laços dirigidos são preservados.”

## Slide 3 — Algoritmos anteriores (1:15–2:00) · Gabriel

“Adaptamos BFS, DFS e as duas versões de Dijkstra para percorrer somente arcos de saída. No exemplo do slide, não se chega a `c` partindo de `s`, pois o arco aponta no sentido contrário. Dijkstra rejeita pesos negativos. Conferimos caminhos reconstruídos e seus custos. A suíte passou em 70 casos e 26.998 verificações, incluindo grafos aleatórios comparados com uma referência independente.”

## Slide 4 — Bellman–Ford (2:00–3:20) · Gabriel

“O enunciado pede distâncias de todas as origens até um destino `t`. Começamos com `d[t]=0` e, para cada arco `u→v`, tentamos melhorar `d[u]` com o peso do arco mais `d[v]`. Guardamos o sucessor para reconstruir o caminho. Implementamos as duas melhorias da aula: guardar as distâncias num vetor, em vez de uma matriz por rodada; e atualizar a partir dos vértices alterados, parando se uma rodada não mudar nada.

“Detectar um ciclo negativo no grafo inteiro é diferente de saber se ele afeta uma consulta. A resposta é menos infinito se a origem alcança o ciclo e, a partir dele, alcança o destino. Sem caminho, é mais infinito.”

## Slide 5 — Floyd–Warshall (3:20–4:10) · João Paulo

“Floyd–Warshall calcula todos os pares. Para cada vértice intermediário `k`, perguntamos se passar por ele melhora o custo de `i` até `j`. Atualizamos uma única matriz de distâncias no lugar; outra matriz guarda os próximos vértices para reconstrução. Uma diagonal negativa indica ciclo negativo, e identificamos os pares afetados. O custo é cúbico em tempo e quadrático em memória.”

## Slide 6 — Destino 100 (4:10–5:25) · João Paulo

“Calculamos as distâncias de 10, 20 e 30 até 100 nos cinco grafos. Em W1, todas são menos infinito. Encontramos um ciclo de peso total menos 2,43 e verificamos que os 25 mil vértices estão na mesma componente fortemente conexa; o ciclo afeta qualquer par.

“W2 e W3 deram valores finitos. Em W4, as três distâncias são 9,04, 8,83 e 9,23; em W5, 37,85, 42,63 e 37,86. Para caber na memória, usamos uma variante compacta de Bellman–Ford. Dez execuções sem leitura de disco deram médias de 3,89 segundos em W4 e 18,06 em W5. Como a implementação difere da usada em W1–W3, esses tempos não devem ser comparados diretamente.”

## Slide 7 — Consultas 5×5 e comparação (5:25–6:55) · João Paulo

“O segundo estudo pede as distâncias entre 10, 20, 30, 40 e 50 usando Floyd. Calculamos as 25 distâncias em cada grafo, porém com cinco destinos de Bellman–Ford. As tabelas W4/W5 estão no apêndice A2. Em W1, todas as 25 entradas são menos infinito. São respostas às consultas, mas não representam uma execução de Floyd nos arquivos reais.

“Só as matrizes de Floyd precisariam de cerca de 7,5 gigabytes já em W1/W2, contra aproximadamente 1,06 gigabyte livre, e seriam 15,6 trilhões de etapas. Em W5, as matrizes chegariam a 1,2 petabyte. Também não rodamos Bellman–Ford para todos os milhões de destinos, principalmente pelo tempo; poderíamos cronometrá-lo sem armazenar todos os resultados. Num ensaio separado com 100 vértices, ambos rodaram dez vezes e Floyd foi 5,37 vezes mais rápido **naquela instância**, sem extrapolação para W1–W5.”

## Slide 8 — Conclusão (6:55–7:45) · João Paulo

“Entregamos os algoritmos dirigidos, a reconstrução de caminhos, os testes e as distâncias solicitadas nos cinco grafos. W4 e W5 também têm dez medições de Bellman–Ford. O achado mais marcante é W1: a forte conexidade faz um ciclo negativo afetar todos os pares.

“Ficaram sem execução nos arquivos reais as medições de Floyd, por memória e tempo, e a comparação completa de todos os pares, sobretudo por tempo. O relatório separa esses limites dos resultados medidos. Obrigado; os apêndices têm as tabelas para perguntas.”

## Se perguntarem pelos apêndices

- **A1:** matrizes W2/W3. Em W2, `d(10,20)=11,73` e `d(20,10)=3,84`: direção importa.
- **A2:** matrizes W4/W5 por Bellman–Ford; caminhos selecionados reconstruídos e custos conferidos.
