# Resultados dos estudos de caso

## Memória (MB = 1024² bytes)

| grafo | n | m | lista: RSS (MB) | lista: estimada (MB) | matriz: RSS (MB) | matriz: estimada (MB) |
|---|---|---|---|---|---|---|
| grafo_1 | 10.000 | 109.921 | 1,2 | 1,1 | 12,0 | 12,0 |
| grafo_2 | 49.948 | 1.298.710 | 11,9 | 11,2 | 297,8 | 297,8 |
| grafo_3 | 375.000 | 765.615 | 21,9 | 15,9 | inviável | 16.767,0 (estimada) |
| grafo_4 | 375.000 | 8.186.986 | 77,5 | 72,5 | inviável | 16.767,0 (estimada) |
| grafo_5 | 4.843.750 | 13.168.911 | 303,4 | 229,8 | inviável | 2.796.911,6 (estimada) |
| grafo_6 | 4.843.750 | 46.469.479 | 549,4 | 483,9 | inviável | 2.796.911,6 (estimada) |

RSS = VmRSS depois da carga − VmRSS antes. Estimada = `bytes_estrutura()`.

## Tempo médio de BFS e DFS (100 buscas; média ± desvio padrão)

| grafo | BFS lista | DFS lista | BFS matriz | DFS matriz |
|---|---|---|---|---|
| grafo_1 | 390,4 µs ± 20,5 µs | 1,43 ms ± 78,3 µs | 3,51 ms ± 166,4 µs | 4,81 ms ± 157,8 µs |
| grafo_2 | 2,32 ms ± 1,66 ms | 4,15 ms ± 2,85 ms | 23,20 ms ± 14,91 ms | 20,48 ms ± 13,25 ms |
| grafo_3 | 16,19 ms ± 6,07 ms | 46,48 ms ± 14,23 ms | inviável | inviável |
| grafo_4 | 74,03 ms ± 22,86 ms | 112,16 ms ± 43,60 ms | inviável | inviável |
| grafo_5 | 302,29 ms ± 152,86 ms | 799,96 ms ± 423,37 ms | inviável | inviável |
| grafo_6 | 977,18 ms ± 650,95 ms | 1,22 s ± 723,34 ms | inviável | inviável |

## Pais nas árvores de busca

| grafo | raiz | BFS: pai de 10 | BFS: pai de 20 | BFS: pai de 30 | DFS: pai de 10 | DFS: pai de 20 | DFS: pai de 30 | lista = matriz |
|---|---|---|---|---|---|---|---|---|
| grafo_1 | 1 | 2.042 | 8.382 | 2.394 | 709 | 666 | 86 | sim |
| grafo_1 | 2 | 8.935 | 9.071 | 3.555 | 709 | 666 | 86 | sim |
| grafo_1 | 3 | 7.685 | 9.543 | 5.783 | 709 | 666 | 86 | sim |
| grafo_2 | 1 | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | sim |
| grafo_2 | 2 | 1.351 | não alcançado | não alcançado | 3.946 | não alcançado | não alcançado | sim |
| grafo_2 | 3 | não alcançado | 46.738 | 12.999 | não alcançado | 217 | 3.513 | sim |
| grafo_3 | 1 | não alcançado | não alcançado | 141.597 | não alcançado | não alcançado | 141.597 | — |
| grafo_3 | 2 | 158.403 | 75.471 | não alcançado | 192.218 | 141.526 | não alcançado | — |
| grafo_3 | 3 | 158.403 | 319.691 | não alcançado | 106.718 | 141.526 | não alcançado | — |
| grafo_4 | 1 | 243.865 | 370.783 | 136.244 | 12.269 | 10.738 | 1.531 | — |
| grafo_4 | 2 | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | — |
| grafo_4 | 3 | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | — |
| grafo_5 | 1 | 1.888.350 | não alcançado | 2.502.539 | 1.888.350 | não alcançado | 191.713 | — |
| grafo_5 | 2 | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | — |
| grafo_5 | 3 | 1.888.350 | não alcançado | 191.713 | 1.888.350 | não alcançado | 2.502.539 | — |
| grafo_6 | 1 | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | — |
| grafo_6 | 2 | 1.677.854 | 3.607.226 | 3.898.629 | 381.031 | 431.008 | 446.011 | — |
| grafo_6 | 3 | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | não alcançado | — |

## Distâncias

| grafo | d(10, 20) | d(10, 30) | d(20, 30) | lista = matriz |
|---|---|---|---|---|
| grafo_1 | 3 | 3 | 4 | sim |
| grafo_2 | ∞ | ∞ | 3 | sim |
| grafo_3 | 9 | ∞ | ∞ | — |
| grafo_4 | 4 | 3 | 4 | — |
| grafo_5 | ∞ | 9 | ∞ | — |
| grafo_6 | 5 | 5 | 5 | — |

## Componentes conexas

| grafo | componentes | maior | menor | lista = matriz |
|---|---|---|---|---|
| grafo_1 | 1 | 10.000 | 10.000 | sim |
| grafo_2 | 10 | 25.000 | 48 | sim |
| grafo_3 | 2 | 250.000 | 125.000 | — |
| grafo_4 | 2 | 250.000 | 125.000 | — |
| grafo_5 | 5 | 2.500.000 | 156.250 | — |
| grafo_6 | 5 | 2.500.000 | 156.250 | — |

## Diâmetro (representação lista)

| grafo | aproximado | par | tempo | BFS | exato (iFUB) | par | tempo | BFS |
|---|---|---|---|---|---|---|---|---|
| grafo_1 | [4, 8] | (3919, 5721) | 2,31 ms | 5 | 5 | (1383, 6451) | 3,72 s | 9.489 |
| grafo_2 | [20, 24] | (26772, 47761) | 34,54 ms | 50 | 20 | (26772, 47761) | 34,89 ms | 57 |
| grafo_3 | [21, 30] | (211931, 183211) | 173,62 ms | 10 | 22 | (21977, 251813) | 229,47 s | 25.744 |
| grafo_4 | [5, 10] | (134822, 23550) | 725,27 ms | 10 | [5, 10] | (134822, 23550) | 1.800,05 s (esgotado) | 20.794 |
| grafo_5 | [58, 78] | (4495114, 1700852) | 5,09 s | 25 | 59 | (2936772, 2289520) | 77,12 s | 5.181 |
| grafo_6 | [19, 28] | (2954530, 2054046) | 12,46 s | 25 | [19, 20] | (2954530, 2054046) | 1.800,02 s (esgotado) | 95.545 |

Intervalo [inferior, superior] quando não exato. O iFUB tem limite de tempo; "esgotado" indica interrupção.
