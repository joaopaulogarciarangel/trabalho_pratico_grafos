# Roteiro de fala — Teoria dos Grafos, Parte 3

**Arquivo:** `apresentacao-grafos-P3.pptx` (ou `.pdf`). **Duração:** cerca de 7min30s.
**Gabriel:** slides 1–7. **João Paulo:** slides 8–14. Apêndice (A1–A3) só para perguntas.

---

## Slide 1 — Capa (0:00–0:20) · Gabriel

"Boa tarde. Somos Gabriel e João Paulo. Na terceira parte, a nossa biblioteca passou a tratar grafos
direcionados com pesos, inclusive negativos, com dois algoritmos novos: Bellman-Ford e Floyd-Warshall."

## Slide 2 — O que a Parte 3 acrescenta (0:20–0:50) · Gabriel

"Cada linha do arquivo agora pode ser um arco, com um sentido só. O Bellman-Ford calcula a distância de todos
os vértices até um destino; o Floyd-Warshall, entre todos os pares. Os dois detectam ciclos negativos. Medimos
tudo em cinco grafos, de 25 mil a 10 milhões de vértices. E nada das partes anteriores mudou: a direção é só um
parâmetro da leitura."

## Slide 3 — Direção (0:50–1:20) · Gabriel

"No vetor de adjacência, o mesmo das outras partes, o vértice v entra só na lista de saída de u; na matriz,
só a posição u, v. Por isso BFS, DFS e Dijkstra funcionaram sem alteração: eles já percorriam os vizinhos de
saída. Arco repetido fica com o menor peso, e laço é mantido, porque um laço negativo já é um ciclo negativo.
Com direção, a distância deixa de ser simétrica: no W_2, de 10 para 20 é 11,73; de 20 para 10, 3,84."

## Slide 4 — Bellman-Ford (1:20–2:20) · Gabriel

"O enunciado pede todos até um destino t. Então começamos de t, com distância zero, e olhamos os arcos que
chegam em cada vértice: se passar pelo arco u para v melhora a distância de u, atualizamos e guardamos o
sucessor, que reconstrói o caminho e forma a árvore.

As duas melhorias são as do slide da aula 14. Na memória, um vetor M[v] em vez da matriz M[i, v]. No tempo,
cada rodada só olha os vértices que mudaram na rodada anterior, e o algoritmo para quando nada muda. Na
prática isso faz diferença: em vez de n menos 1 rodadas, os grafos do estudo pararam entre 24 e 42."

## Slide 5 — Ciclos negativos (2:20–3:00) · Gabriel

"Um ciclo negativo pode existir e não afetar uma consulta, então separamos as perguntas. Para saber se existe,
usamos uma superorigem ligada a todos com custo zero. Para saber se afeta o destino, vemos quem ainda melhora
depois da última rodada: esse vértice e todos que chegam nele ficam com menos infinito. Sem caminho, mais
infinito. No Floyd-Warshall, o sinal é a diagonal negativa.

No W_1 encontramos um ciclo de peso −2,43, e o grafo é fortemente conexo: todo vértice alcança o ciclo e é
alcançado por ele. Então todas as distâncias são menos infinito."

## Slide 6 — Preparar uma vez (3:00–3:25) · Gabriel

"Os arcos de entrada e a detecção do ciclo não dependem do destino. A primeira versão refazia isso a cada
chamada, e essa cópia do grafo entrava no cronômetro. Agora preparamos uma vez e cada consulta mede só o
algoritmo. Isso importa no estudo de todos os pares, que faz n consultas no mesmo grafo."

## Slide 7 — Floyd-Warshall (3:25–3:55) · Gabriel

"k é o vértice intermediário permitido. Como na aula 13, atualizamos uma única matriz no lugar, em vez de uma
por valor de k. A segunda matriz guarda o próximo vértice e permite devolver o caminho. São 12n² bytes e n³
passos, qualquer que seja o grafo. Passo para o João."

## Slide 8 — Floyd nos grafos do estudo (3:55–4:30) · João Paulo

"Com 25 mil vértices, as duas matrizes dariam 7,5 GB, mais que a memória da máquina. O estudo só pede
distâncias, então usamos só a matriz de distâncias, 5 GB. O laço simples relê os 5 GB inteiros para cada k e
levaria horas. Por isso percorremos a matriz em blocos: é a mesma recorrência e a mesma matriz, mas cada pedaço
é reaproveitado no cache, e os blocos independentes rodam em paralelo. Testamos que o resultado é o mesmo.
A partir do W_3, a matriz teria 80 GB: aí só dá para estimar."

## Slide 9 — Os grafos (4:30–4:50) · João Paulo

"Todos os grafos foram lidos na própria biblioteca. O Floyd-Warshall cabe no W_1 e no W_2; nos outros, a matriz
iria de 80 GB a 800 TB. Só o W_1 tem pesos negativos e ciclo."

## Slide 10 — Estudo 1 (4:50–5:20) · João Paulo

"As distâncias até o vértice 100. No W_1, tudo menos infinito, e a resposta sai em 32 microssegundos, porque o
ciclo já foi detectado na preparação. Nos outros, de 16 milissegundos no W_2 a 6,5 segundos no W_5, com 10 milhões
de vértices. Conferimos tudo com um verificador que escrevemos fora da biblioteca e com um trabalho de 2025."

## Slide 11 — Estudo 2 (5:20–5:45) · João Paulo

"No W_2 a matriz é do Floyd-Warshall, conferida par a par com o Bellman-Ford. No W_1 tudo é menos infinito. Do
W_3 ao W_5 a matriz não cabe, então usamos cinco execuções do Bellman-Ford; estão no apêndice."

## Slide 12 — Estudos 3 e 4 (5:45–6:25) · João Paulo

"Os tempos para todos os pares. No W_2, o Floyd-Warshall levou 12 minutos por execução, e o Bellman-Ford para
os 25 mil destinos, 6,7 minutos: 1,8 vez mais rápido, mesmo o Floyd usando 12 threads e o Bellman-Ford uma.
No W_1 a diferença é enorme: o Bellman-Ford responde todos os pares em 0,35 segundo, e o Floyd leva 14,5 minutos.
No W_1 o Floyd foi medido 3 vezes, porque cada execução leva quase 15 minutos; o resto são médias de 10.
Do W_3 em diante, marcamos como estimativas."

## Slide 13 — Por quê (6:25–7:05) · João Paulo

"O Floyd-Warshall faz n³ passos sempre: 1,6 vezes 10¹³ no W_2. Ele não aproveita o grafo ser esparso.
O Bellman-Ford, com as melhorias da aula, parou em 24 rodadas no W_2, cada uma tocando só os arcos dos
vértices que mudaram: no máximo 4,9 vezes 10¹¹ passos para todos os pares, 32 vezes menos. Com ciclo negativo,
o Bellman-Ford detecta uma vez e responde na hora, enquanto o Floyd precisa dos n³ passos para ver a diagonal.
Em grafo denso o Floyd volta a competir: no grafo de validação, com 100 vértices, os dois ficaram na mesma
ordem. E, sem pesos negativos, n execuções do Dijkstra seriam ainda melhores."

## Slide 14 — Conclusões (7:05–7:30) · João Paulo

"Resumindo: a biblioteca trata direção, Bellman-Ford e Floyd-Warshall, com ciclos negativos. As melhorias da
aula fazem o Bellman-Ford parar muito antes de n rodadas. O Floyd é limitado por n³ e n², e para todos os pares
em grafos esparsos o Bellman-Ford repetido venceu. Obrigado."

---

## Perguntas prováveis — respostas curtas

- **Por que o Floyd usa 12 threads e o Bellman-Ford não?** O paralelismo foi necessário para o Floyd caber no
  tempo (o laço simples levaria horas). Mesmo com essa vantagem, ele perdeu; sem ela, a diferença seria maior.
- **O Floyd em blocos ainda é o Floyd-Warshall?** Sim: mesma recorrência, mesma matriz única. Muda só a ordem
  de visita, e os testes conferem o resultado com o laço simples.
- **Por que o W_1 só tem 3 execuções do Floyd?** Cada uma leva ~14,5 min; as três ficaram entre 13,7 e 16,1 min.
  Está dito no relatório.
- **Como sabem que o −∞ do W_1 está certo?** Ciclo 15108 → 3965 → 15108 de peso −2,43, e o W_1 é fortemente
  conexo (verificado fora da biblioteca); Bellman-Ford e Floyd concordam.
- **Por que estimar W_3 a W_5?** A matriz do Floyd teria 80 GB a 800 TB; o Bellman-Ford para todos os pares
  levaria de 4,6 horas a 2 anos. As estimativas estão marcadas.
