# Resultados complementares definitivos — W4 e W5

Estes valores são **calculados**, não estimativas. Use os arquivos de relatório e slides com `atualizado` no nome.

| Grafo | 10→100 | 20→100 | 30→100 | Média de 10 chamadas BF→100 |
|---|---:|---:|---:|---:|
| W4 | 9,04 | 8,83 | 9,23 | 3,891435 s |
| W5 | 37,85 | 42,63 | 37,86 | 18,064925 s |

As tabelas 5×5 de W4/W5 também foram calculadas com cinco destinos de Bellman–Ford por grafo. Elas aparecem no relatório e no apêndice A2 dos slides. Todos os caminhos selecionados foram reconstruídos e tiveram seus pesos conferidos. Em W2, a variante compacta reproduziu as 25 entradas da implementação original.

Cada arco foi convertido para registro compacto de 12 bytes. Conversão e carga terminaram **antes** de iniciar o cronômetro. A média inclui dez execuções do algoritmo até o destino 100, sem leitura nem escrita. Os tempos pertencem à variante compacta e não são diretamente comparáveis aos tempos W1–W3 da implementação original.

O experimento anterior em fluxo calculava apenas as três distâncias até 100 e não tinha médias. Ele foi superado pela medição compacta completa. `bf_fluxo_w4_w5.cpp` permanece como registro histórico, não como fonte dos tempos finais.

**Limites restantes:** Floyd–Warshall não foi executado nos arquivos reais por memória e tempo. Bellman–Ford para todos os pares também não foi executado neles, principalmente pelo tempo necessário para processar todos os destinos; a medição poderia descartar os resultados sem armazenar uma matriz completa. As tabelas 5×5 são respostas às consultas, não uma execução de Floyd.
