# Roteiro da apresentação — Teoria dos Grafos, Parte 3 (8 minutos)

**Dupla:** Gabriel dos Santos Percu Saldanha e João Paulo Garcia de Assis Rangel
**Arquivo:** `Teoria-dos-Grafos-Parte-3-corrigida.pdf` (ou `.pptx`) — 10 slides: 8 da apresentação + apêndices A1 e A2, só para perguntas
**Tempo planejado:** 7 min 25 s de fala, com 35 s de folga para a troca de apresentador e imprevistos.

---

## Como ler este roteiro

- **Texto entre aspas** é a fala sugerida. Não é para decorar palavra por palavra: entendam a ideia e falem com as próprias palavras, mantendo os números e os termos técnicos.
- **[Entre colchetes, em itálico]** está o que fazer naquele momento: apontar, pausar, olhar para a turma, avançar o slide.
- **Negrito dentro da fala** marca a ideia que precisa ficar. Diminuam o ritmo e deem ênfase.
- Cada slide traz o **tempo** e o **relógio** acumulado. A fala foi dimensionada para cerca de 130 palavras por minuto.
- **Nomes por extenso na primeira vez:** busca em largura (BFS), busca em profundidade (DFS), Bellman–Ford, Floyd–Warshall. Depois pode abreviar.
- **Como ler os números em voz alta:** “−∞” → “menos infinito”; “n³” → “n ao cubo”; “n²” → “n ao quadrado”; “W1” → “dáblio um”.
- **Postura geral:**
  - Falem para a turma, não para a tela. Olhem o slide só para apontar.
  - Fiquem ao lado da tela, do lado do desenho ou da tabela que vão comentar.
  - Ao apontar um desenho ou uma tabela, **primeiro digam o que é** (“esta tabela mostra…”) e só depois expliquem.
  - Em tabela, apontem **a linha inteira** com a mão aberta, da esquerda para a direita; em número isolado, com um dedo.

---

## 1. Divisão entre os integrantes

| Integrante | Bloco | Slides | Tempo | Relógio ao terminar |
|---|---|---|---|---|
| **Gabriel** | Abertura, direção, buscas e Bellman–Ford | 1 a 4 | 3 min 15 s | 3:15 |
| **João Paulo** | Floyd–Warshall, resultados, comparação e conclusão | 5 a 8 | 4 min 10 s | 7:25 |

O bloco do João Paulo é maior porque concentra os resultados medidos, que são a parte que o professor mais vai cobrar. O Gabriel abre e explica as decisões de implementação.

**Troca de apresentador:** no fim do slide 4, o Gabriel diz a frase de passagem e dá um passo para o lado; o João Paulo já está perto da tela e começa sem pausa.

**Apêndices A1 e A2:** não são apresentados. Ficam prontos para perguntas sobre as matrizes 5×5.

---

## 2. Cronograma slide a slide

| Slide | Título | Quem | Tempo | Relógio |
|---:|---|:---:|---:|---:|
| 1 | Grafos dirigidos e caminhos mínimos (capa) | Gabriel | 0:25 | 0:25 |
| 2 | Uma linha do arquivo, um arco: u → v | Gabriel | 0:40 | 1:05 |
| 3 | Buscas e Dijkstra seguem os arcos | Gabriel | 0:45 | 1:50 |
| 4 | Bellman–Ford: todos até um destino t | Gabriel | 1:25 | **3:15** |
| 5 | Floyd–Warshall: uma única matriz | João Paulo | 1:00 | 4:15 |
| 6 | Distâncias até o vértice 100 | João Paulo | 1:00 | 5:15 |
| 7 | Todos os pares e os limites de memória | João Paulo | 1:30 | 6:45 |
| 8 | O que entregamos e o que medimos | João Paulo | 0:40 | **7:25** |
| A1, A2 | Matrizes de pares | — | só em perguntas | — |

---

## 3. Falas slide a slide

### Gabriel — Abertura, direção, buscas e Bellman–Ford (0:00 → 3:15)

---

#### Slide 1 — Capa · 0:25 · relógio 0:25

**Objetivo:** apresentar a dupla e dar, logo de cara, a resposta curta do trabalho.

> *[Comece de frente para a turma, sem olhar a tela. Espere o silêncio antes de falar.]*
>
> “Boa tarde. Nós somos o Gabriel e o João Paulo *[indique o colega com a mão]*, e esta é a terceira parte do nosso trabalho de Teoria dos Grafos.
>
> *[Vire-se e passe o dedo pelo caminho azul do desenho, de ORIGEM até DESTINO.]*
> Nesta parte, a biblioteca passou a tratar **grafos dirigidos com pesos, inclusive negativos**, e ganhou dois algoritmos de caminho mínimo: o **Bellman–Ford** e o **Floyd–Warshall**.
>
> *[Volte a olhar para a turma.]*
> No fim, vamos mostrar qual dos dois foi **mais eficiente nos grafos do professor, e por quê**.”

---

#### Slide 2 — Uma linha do arquivo, um arco: u → v · 0:40 · relógio 1:05

**Objetivo:** mostrar que a direção é uma mudança **pequena e localizada** na leitura do grafo.

> *[Avance. Aponte a linha do tempo da esquerda: primeiro “PARTES 1 E 2”, depois “PARTE 3”.]*
> “A Parte 3 é uma continuação: a base das Partes 1 e 2 — lista e matriz de adjacências, BFS, DFS e Dijkstra — continua igual, e a Parte 3 acrescenta o **modo dirigido**, o Bellman–Ford e o Floyd–Warshall.
>
> *[Vá para o cartão da direita. Aponte “u v peso” no topo.]*
> Cada linha do arquivo traz dois vértices e um peso.
>
> *[Aponte o desenho “Não dirigido”: a linha sem seta entre u e v.]*
> No modo não dirigido, a aresta **vale nos dois sentidos**.
>
> *[Aponte o desenho “Dirigido”: a seta azul de u para v. Diminua o ritmo.]*
> No modo dirigido, ela vira **só o arco de u para v**.
>
> *[Aponte as duas linhas de baixo do cartão.]*
> Na prática, a mudança está só na leitura: na lista de adjacências, **v entra apenas na lista de u**; na matriz, **só a posição u, v** recebe o peso. Arco repetido fica com o menor peso, e laço é mantido, porque um laço negativo já é um ciclo negativo.”

---

#### Slide 3 — Buscas e Dijkstra seguem os arcos · 0:45 · relógio 1:50

**Objetivo:** mostrar o efeito da direção nos algoritmos anteriores e a confiabilidade dos testes.

> *[Avance. Aponte o desenho da esquerda e diga o que é antes de explicar.]*
> “Neste exemplo, *[passe o dedo de s até a e de s até b]* a partir de **s** chegamos a **a** e a **b**. *[Aponte o arco tracejado entre c e s]* Mas **não** chegamos a **c**, porque o arco aponta **de c para s**, no sentido contrário.
>
> *[Vá para a lista da direita, item por item.]*
> Por isso a busca em largura e a busca em profundidade *[primeiro item]* passaram a visitar **só os vizinhos de saída**. *[Segundo item]* O Dijkstra, nas duas versões, com vetor e com heap, segue a direção e **continua recusando pesos negativos** — para eles, existe o Bellman–Ford. *[Terceiro item]* E os caminhos são reconstruídos e conferidos somando os pesos.
>
> *[Desça a mão até a faixa “SUÍTE RELEASE” e aponte os três números, um por vez.]*
> Para garantir que tudo isso está certo, a suíte de testes tem **72 casos** e **40.969 verificações**, incluindo **100 grafos dirigidos aleatórios** comparados com uma implementação independente.”

---

#### Slide 4 — Bellman–Ford: todos até um destino t · 1:25 · relógio 3:15

**Objetivo:** explicar o algoritmo, as **duas otimizações da aula 14** e a ideia de ciclo negativo. É o slide mais conceitual do Gabriel: fale mais devagar.

> *[Avance. Aponte o vértice azul t, à direita do desenho, e o rótulo “d[t] = 0”.]*
> “O enunciado pede a distância de **todos os vértices até um destino t**. Então começamos pelo destino: a distância de t até ele mesmo é zero.
>
> *[Siga com o dedo o arco azul de u para v, depois de v para t.]*
> Para cada arco de **u para v**, perguntamos: ir de u até v e, de lá, seguir até t sai mais barato do que o que já conhecíamos para u?
>
> *[Aponte a fórmula na caixa de baixo e leia devagar.]*
> É esta fórmula: a distância de u passa a ser o menor valor entre a atual e **o peso do arco mais a distância de v**. *[Aponte a linha pequena abaixo da fórmula]* E guardamos **v como sucessor de u**: seguindo os sucessores, reconstruímos o caminho de u até t e formamos a árvore de caminhos mínimos.
>
> *[Vá para a coluna da direita, bloco “DUAS OTIMIZAÇÕES”. Aponte o item 1.]*
> Implementamos as **duas melhorias da aula 14**. A primeira é de **memória**: um único vetor de distâncias, em vez de uma matriz com uma linha por iteração.
>
> *[Aponte o item 2. Ênfase: é a que mais pesa no tempo.]*
> A segunda é de **tempo**: cada rodada só olha os vértices **cujo valor mudou na rodada anterior**, e o algoritmo **para quando uma rodada não muda nada**. No pior caso continua n vezes m, mas nos grafos do trabalho ele parou entre **24 e 42 rodadas**, e não em n menos 1.
>
> *[Suba para o bloco “CICLOS NEGATIVOS”. Aponte “Ciclo global”.]*
> Com pesos negativos, pode haver ciclo negativo. **Existir** um ciclo no grafo é diferente de ele **afetar a consulta**: ele pode estar num canto que não leva a t. *[Aponte “Ciclo que afeta t”, em laranja]* Quando a origem alcança o ciclo e, dele, chega a t, dá para dar voltas no ciclo e baixar o custo sem fim: a distância é **menos infinito**. Sem caminho até t, é mais infinito.
>
> *[Volte a olhar para a turma.]*
> Um detalhe de implementação: os arcos de entrada e a detecção do ciclo **não dependem do destino**. Então preparamos isso **uma vez** e cada consulta mede só o algoritmo.”
>
> *Passagem para o João Paulo:* “Isso é o Bellman–Ford, que resolve um destino por vez. Para todos os pares de uma vez, o João mostra o Floyd–Warshall.” *[Dê um passo para o lado.]*

---

### João Paulo — Floyd–Warshall, resultados, comparação e conclusão (3:15 → 7:25)

---

#### Slide 5 — Floyd–Warshall: uma única matriz · 1:00 · relógio 4:15

**Objetivo:** explicar a recorrência, a otimização de memória da aula 13 e o custo.

> *[Avance. Aponte a fórmula no topo da coluna da direita.]*
> “O Floyd–Warshall calcula **todos os pares de uma vez**. A ideia é esta fórmula: a distância de i até j melhora se **passar por k** for mais barato.
>
> *[Vá para a grade da esquerda. Diga o que é: “esta grade é a matriz de distâncias”. Aponte a linha e a coluna azuis marcadas com k.]*
> O k é o vértice intermediário permitido, e fica no laço de fora. Em cada etapa, a linha e a coluna de k, em azul, servem para atualizar o resto. *[Siga a seta branca até a célula de destino]* Por exemplo, esta célula de i até j é atualizada usando a linha e a coluna de k.
>
> *[Volte à coluna da direita e aponte os três itens, um de cada vez.]*
> Usamos o **algoritmo melhorado da aula 13**: **uma única matriz**, atualizada no lugar, em vez de uma matriz por valor de k. Uma segunda matriz guarda o **próximo vértice**, para reconstruir caminhos e árvores. *[Aponte o item em laranja e a diagonal laranja da grade]* E, se a **diagonal fica negativa**, um vértice chega a si mesmo com custo negativo: é um **ciclo negativo**, e os pares afetados recebem menos infinito.
>
> *[Aponte TEMPO e MEMÓRIA, embaixo. Ênfase.]*
> O custo é **n ao cubo em tempo e n ao quadrado em memória**, qualquer que seja o grafo.
>
> *[Olhe para a turma: este é o detalhe técnico que viabilizou a medição.]*
> Nos grafos de 25 mil vértices, as duas matrizes dariam 7,5 gigabytes e não cabiam. Como o estudo só pede distâncias, **guardamos só a matriz de distâncias**, 5 gigabytes, e a percorremos **em blocos, em paralelo**. É a mesma recorrência, só que aproveitando a memória cache, e os testes confirmam que o resultado é idêntico.”

---

#### Slide 6 — Distâncias até o vértice 100 · 1:00 · relógio 5:15

**Objetivo:** responder ao **estudo 1** e explicar o caso do W1.

> *[Avance. Diga o que é a tabela antes de explicar.]*
> “Esta tabela é o **primeiro estudo**: a distância dos vértices 10, 20 e 30 até o vértice 100, nos cinco grafos.
>
> *[Aponte com a mão aberta as linhas W2 a W5.]*
> Do W2 ao W5 as distâncias são finitas: no W2, por exemplo, **10,47, 8,48 e 8,71**. Conferimos todas com um verificador escrito fora da biblioteca e com um trabalho independente de 2025, e **bateram todas**.
>
> *[Aponte a coluna “Tempo médio”, de cima para baixo.]*
> O tempo é a média de 10 execuções: vai de **16 milissegundos no W2 a 6,45 segundos no W5**, que tem 10 milhões de vértices. A leitura do arquivo e a preparação ficam fora do cronômetro.
>
> *[Aponte a linha W1, destacada, e os três “menos infinito”.]*
> E o W1 é o caso interessante: **as três distâncias são menos infinito**.
>
> *[Vá para o desenho da direita. Siga o arco de 15108 para 3965, peso 0,04, e volte pelo arco laranja, peso −2,47.]*
> O motivo é este ciclo: de 15108 para 3965 custa 0,04, e de volta custa menos 2,47. *[Aponte “Soma: −2,43”]* A volta completa **custa menos 2,43**.
>
> *[Aponte o texto de baixo e olhe para a turma.]*
> E o W1 é **fortemente conexo**: todo vértice alcança esse ciclo e é alcançado por ele. Então qualquer caminho pode dar voltas no ciclo e baixar o custo indefinidamente: **não existe caminho mínimo finito**. Por isso o tempo no W1 é de 32 microssegundos: o ciclo já foi detectado na preparação, e a resposta sai na hora.”

---

#### Slide 7 — Todos os pares e os limites de memória · 1:30 · relógio 6:45

**Objetivo:** responder aos **estudos 2, 3 e 4**: as distâncias entre os pares, o tempo de cada algoritmo e **qual é mais eficiente, e por quê**. É o slide mais importante da apresentação.

> *[Avance. Aponte o cartão superior direito, “MATRIZ 5×5 · FLOYD–WARSHALL”.]*
> “O **segundo estudo** pede as distâncias entre os vértices 10, 20, 30, 40 e 50, pelo Floyd–Warshall. No W2, a matriz é do Floyd e foi **conferida par a par com o Bellman–Ford**. *[Aponte a linha laranja]* No W1, as 25 são menos infinito. *[Aponte a última linha do cartão]* As matrizes estão nos apêndices.
>
> *[Vá para a tabela da esquerda. Diga o que é.]*
> Esta tabela é a comparação dos **estudos 3 e 4**: o tempo para obter **todos os pares** com o Floyd–Warshall e com o Bellman–Ford repetido para cada destino.
>
> *[Aponte a coluna “Matriz FW”, de cima para baixo.]*
> Primeiro, a memória: a matriz do Floyd tem 5 gigabytes no W1 e no W2, mas **80 gigabytes no W3, 8 terabytes no W4 e 800 no W5**. Por isso o Floyd só rodou de verdade no W1 e no W2; do W3 em diante os tempos são **estimativas**, marcadas com asterisco.
>
> *[Aponte a linha W2 inteira, com a mão aberta, devagar.]*
> Agora o resultado. No W2, o Floyd–Warshall levou **12 minutos** por execução; o Bellman–Ford para os 25 mil destinos levou **6,7 minutos**. *[Aponte o destaque “Bellman–Ford repetido venceu”]* O Bellman–Ford foi **1,8 vez mais rápido**, e isso **com o Floyd usando 12 núcleos de processamento e o Bellman–Ford, um só**.
>
> *[Aponte a linha W1.]*
> No W1 a diferença é enorme: o Bellman–Ford respondeu todos os pares em **0,35 segundo**, porque detecta o ciclo uma vez; o Floyd levou **14,5 minutos** para chegar à mesma conclusão. *[Aponte “(3×)”]* No W1 o Floyd foi medido **três vezes**, porque cada execução leva quase 15 minutos; todo o resto é média de dez.
>
> *[Olhe para a turma. Ritmo mais lento: este é o “por quê” do estudo 4.]*
> **Por que o Bellman–Ford ganha?** O Floyd faz **n ao cubo** passos **sempre**: 15 trilhões e meio no W2, sem aproveitar que o grafo é esparso. Já o Bellman–Ford, com as melhorias da aula, **para em poucas rodadas e só toca os arcos dos vértices que mudaram**: para todos os pares, isso dá umas **30 vezes menos trabalho**.
>
> *[Aponte a caixa tracejada “ENSAIO SEPARADO”.]*
> O Floyd só volta a competir em grafo **pequeno e mais denso**: neste ensaio com 100 vértices, os dois ficaram na mesma ordem de grandeza.”

---

#### Slide 8 — O que entregamos e o que medimos · 0:40 · relógio 7:25

**Objetivo:** fechar com a mensagem central.

> *[Avance. Passe a mão de cima para baixo pela coluna “ENTREGUE”, sem ler item por item.]*
> “Resumindo o que entregamos: o modo dirigido nas duas representações, as buscas e o Dijkstra respeitando a direção, o **Bellman–Ford com as duas otimizações e a detecção de ciclos**, o Floyd–Warshall com matriz única, os testes e as distâncias nos cinco grafos.
>
> *[Vá para a coluna “RESULTADOS” e aponte os três itens.]*
> E o que medimos: o Floyd–Warshall rodou no W1 e no W2; na comparação de todos os pares, **o Bellman–Ford repetido venceu**; e, do W3 em diante, o Floyd não cabe na memória.
>
> *[Volte a olhar para a turma. Frase final, devagar.]*
> A lição é que **a complexidade de pior caso não conta a história toda**: o algoritmo que parece mais caro para todos os pares venceu, porque **para cedo e respeita o grafo esparso**. Obrigado. Ficamos à disposição para perguntas.”

---

## 4. Controle do tempo

| Marco | Deveria estar em | Se estiver atrasado |
|---|---|---|
| Fim do Gabriel | **3:15**, no fim do slide 4 | No slide 4, cortar o parágrafo final sobre a preparação feita uma vez; ele volta no slide 6. |
| Entrada no slide 7 | **5:15** | No slide 6, não ler as distâncias do W2: dizer só “finitas e conferidas” e ir direto ao W1. |
| Entrada no slide 8 | **6:45** | No slide 7, pular o ensaio separado e ir direto à conclusão. |

Combinem que **quem não está falando segura o relógio** e mostra a mão aberta quando faltar 1 minuto para o fim do bloco.

**Cortes de emergência, nesta ordem (economizam cerca de 1 minuto):**
1. Slide 3: dizer só “as buscas e o Dijkstra seguem os arcos de saída; a suíte tem 72 casos e 40.969 verificações”.
2. Slide 5: cortar o último parágrafo, sobre os blocos; se perguntarem, responde-se na hora.
3. Slide 7: cortar o ensaio separado.
4. Slide 2: dizer só “cada linha vira um arco de u para v, e a mudança fica só na leitura”.

**Nunca cortar:** o “por quê” do slide 7 e a explicação do ciclo do W1 no slide 6. São as respostas que o professor vai procurar.

---

## 5. Perguntas prováveis e respostas

Respostas curtas. Sempre que der, **apontem o slide** onde aquilo apareceu.

| Pergunta | Resposta | Slide de apoio | Quem responde |
|---|---|:---:|:---:|
| **Por que o Floyd usa 12 núcleos e o Bellman–Ford não? Não é injusto?** | O paralelismo foi necessário para o Floyd caber no tempo: o laço simples levaria horas por execução. Mesmo com essa vantagem ele perdeu; com um núcleo só, a diferença seria ainda maior. | 7 | João Paulo |
| **O Floyd em blocos ainda é o Floyd–Warshall?** | É: mesma recorrência, mesma matriz única. Muda só a ordem em que a matriz é visitada, para aproveitar a memória cache. Os testes comparam com o laço simples, inclusive com ciclo negativo. | 5 | João Paulo |
| **Por que só 3 execuções do Floyd no W1?** | Cada execução leva cerca de 14,5 minutos; as três ficaram entre 13,7 e 16,1. O relatório diz isso. Todas as outras medidas têm 10 execuções. | 7 | João Paulo |
| **Como vocês sabem que o menos infinito do W1 está certo?** | O ciclo 15108 → 3965 → 15108 pesa −2,43, e verificamos fora da biblioteca que o W1 é fortemente conexo. Bellman–Ford e Floyd–Warshall chegaram à mesma conclusão. | 6 | João Paulo |
| **Quais são as duas otimizações do Bellman–Ford?** | Memória: um vetor M[v] em vez da matriz M[i, v]. Tempo: só relaxar a partir dos vértices que mudaram na rodada anterior e parar quando nada muda. Ambas da aula 14. | 4 | Gabriel |
| **Qual a diferença entre ciclo global e ciclo que afeta t?** | O global existe em algum lugar do grafo; ele só afeta a consulta se a origem o alcança e, a partir dele, se chega a t. Detectamos o global com uma superorigem ligada a todos os vértices com custo zero. | 4 | Gabriel |
| **Como os tempos do W3 ao W5 foram obtidos?** | Floyd: pela escala n³ a partir do W2, porque a matriz teria de 80 GB a 800 TB. Bellman–Ford para todos os pares: n vezes a média de 100 destinos sorteados. Estão marcados como estimativa. | 7 | João Paulo |
| **Por que não usar Dijkstra para todos os pares?** | Sem pesos negativos, n execuções do Dijkstra com heap seriam ainda mais rápidas. Mas ele não aceita pesos negativos, e o W1 tem 4.998 deles. | 3 e 7 | qualquer um |
| **Por que o Bellman–Ford parou tão cedo?** | Pela segunda otimização: depois de 24 a 42 rodadas nenhuma distância muda mais, e o algoritmo para. O pior caso, n − 1 rodadas, praticamente não acontece nesses grafos. | 4 | Gabriel |
| **Qual é a matriz 5×5 do W4?** *(ou de outro grafo)* | Mostrar o apêndice A1 (W2 e W3) ou A2 (W4 e W5). Exemplo de direção: no W2, de 10 para 20 é 11,73, mas de 20 para 10 é 3,84. | A1, A2 | João Paulo |

---

## 6. Ensaio

- [ ] Ler este roteiro em voz alta uma vez, cada um no seu bloco, **com cronômetro**.
- [ ] Ensaiar os gestos de apontar no projetor ou numa TV: o dedo segue o arco no desenho e a linha inteira na tabela, devagar.
- [ ] Conferir se BFS, DFS, Bellman–Ford e Floyd–Warshall são ditos por extenso na primeira vez.
- [ ] Treinar a passagem do Gabriel para o João Paulo (fim do slide 4).
- [ ] Ensaiar as respostas das perguntas prováveis, principalmente as três primeiras.
- [ ] Saber ir direto aos apêndices A1 e A2.
- [ ] Ter o arquivo em pendrive e no OneDrive.
