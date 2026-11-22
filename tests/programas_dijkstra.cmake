# Testes de ponta a ponta da Parte 2: `grafos dijkstra` e `estudos_dijkstra` (executado por ctest).
# Variaveis: GRAFOS, ESTUDOS_DIJKSTRA, DADOS (tests/dados), TMP (pasta de trabalho).

file(REMOVE_RECURSE "${TMP}")
file(MAKE_DIRECTORY "${TMP}")

include("${CMAKE_CURRENT_LIST_DIR}/funcoes_programas.cmake")

# Figura 1 do enunciado com -9.5 trocado por 9.5.
file(WRITE "${TMP}/fig1_pos.txt" "5\n1 2 0.1\n2 5 0.2\n5 3 5\n3 4 9.5\n4 5 2.3\n1 5 1\n")
set(ARVORE_POS "# vertice pai distancia\n1 0 0\n2 1 0.1\n3 5 5.3\n4 5 2.5999999999999996\n5 2 0.30000000000000004\n")

foreach(repr lista matriz)
    foreach(fila heap vetor)
        rodar(0 "${GRAFOS}" dijkstra "${TMP}/fig1_pos.txt" --origem 1 --fila ${fila} --repr ${repr}
              --destinos 3,4,1 -o "${TMP}/dij_${repr}_${fila}.txt")
        exigir_texto("${SAIDA}" "origem: 1, fila: ${fila}, explorados: 5")
        exigir_texto("${SAIDA}" "destino 3: distancia 5.3, 3 arestas\n  caminho: 1 -> 2 -> 5 -> 3\n")
        exigir_texto("${SAIDA}" "destino 4: distancia 2.5999999999999996, 3 arestas")
        exigir_texto("${SAIDA}" "destino 1: distancia 0, 0 arestas\n  caminho: 1\n")
        comparar("${TMP}/dij_${repr}_${fila}.txt" "# Dijkstra origem=1 fila=${fila}\n${ARVORE_POS}")
    endforeach()
endforeach()

# Padrao: heap; destino inalcancavel
file(WRITE "${TMP}/desconexo.txt" "6\n1 2 2\n2 3 1.5\n4 5 1\n")
rodar(0 "${GRAFOS}" dijkstra "${TMP}/desconexo.txt" --origem 1 --destinos 3,4)
exigir_texto("${SAIDA}" "fila: heap")
exigir_texto("${SAIDA}" "destino 3: distancia 3.5, 2 arestas")
exigir_texto("${SAIDA}" "destino 4: infinito\n")

# Pesos negativos: recusa com a mensagem da especificacao
rodar(1 "${GRAFOS}" dijkstra "${DADOS}/figura1_pesos.txt" --origem 1)
exigir_texto("${ERRO}" "Dijkstra nao aceita pesos negativos; use Bellman-Ford")

# Erros de uso e de entrada
rodar(2 "${GRAFOS}" dijkstra "${TMP}/fig1_pos.txt")
rodar(2 "${GRAFOS}" dijkstra "${TMP}/fig1_pos.txt" --origem 1 --fila lista)
rodar(2 "${GRAFOS}" dijkstra "${TMP}/fig1_pos.txt" --origem 1 --destinos 3,x)
rodar(1 "${GRAFOS}" dijkstra "${TMP}/fig1_pos.txt" --origem 9)
exigir_texto("${ERRO}" "vertice 9 fora do intervalo 1..5")
rodar(1 "${GRAFOS}" dijkstra "${TMP}/fig1_pos.txt" --origem 1 --destinos 7)
exigir_texto("${ERRO}" "vertice 7 fora do intervalo 1..5")
rodar(0 "${GRAFOS}" --help)
exigir_texto("${SAIDA}" "grafos dijkstra")

# Matriz ponderada: a viabilidade conta os 8n^2 bytes dos pesos. Com n = 30000 o bitset
# cabe (112 MB), mas os pesos exigiriam 7,2 GB; sem peso, a mesma matriz e aceita.
file(WRITE "${TMP}/grande_pesos.txt" "30000\n1 2 0.5\n")
rodar(1 "${GRAFOS}" dijkstra "${TMP}/grande_pesos.txt" --origem 1 --repr matriz)
exigir_texto("${ERRO}" "matriz exigiria 6.8 GB")
file(WRITE "${TMP}/grande_sem_peso.txt" "30000\n1 2\n")
rodar(0 "${GRAFOS}" dijkstra "${TMP}/grande_sem_peso.txt" --origem 1 --repr matriz --destinos 2)
exigir_texto("${SAIDA}" "destino 2: distancia 1, 1 arestas")

# estudos_dijkstra, modo grafo: caminho de 70 vertices com pesos 0.5 (10 -> 60 = 25)
set(caminho70 "70\n")
foreach(i RANGE 1 69)
    math(EXPR j "${i} + 1")
    string(APPEND caminho70 "${i} ${j} 0.5\n")
endforeach()
file(WRITE "${TMP}/caminho70.txt" "${caminho70}")
rodar(0 "${ESTUDOS_DIJKSTRA}" --grafo "${TMP}/caminho70.txt" --saida "${TMP}/res" --k-heap 8 --k-vetor 3)
file(READ "${TMP}/res/caminho70_dijkstra.csv" csv)
exigir_texto("${csv}" "chave,valor\n")
exigir_texto("${csv}" "ponderado,sim\n")
exigir_texto("${csv}" "pesos_negativos,nao\n")
exigir_texto("${csv}" "dist_10_20,5\n")
exigir_texto("${csv}" "dist_10_60,25\n")
exigir_texto("${csv}" "arestas_10_60,50\n")
exigir_texto("${csv}" "caminho_10_30,10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30\n")
exigir_texto("${csv}" "heap_execucoes,8\n")
exigir_texto("${csv}" "vetor_execucoes,3\n")
exigir_texto("${csv}" "conferencia_vetor_heap,ok\n")
exigir_texto("${csv}" "razao_vetor_heap,")
if(EXISTS "${TMP}/res/caminho70_dijkstra.csv.tmp")
    message(FATAL_ERROR "estudos_dijkstra deixou o .tmp")
endif()

# Estimativa do vetor: so com --k-vetor 0
rodar(0 "${ESTUDOS_DIJKSTRA}" --grafo "${TMP}/caminho70.txt" --saida "${TMP}/res_est" --k-heap 4 --k-vetor 0
      --estimar-vetor 0.5)
file(READ "${TMP}/res_est/caminho70_dijkstra.csv" csv)
exigir_texto("${csv}" "vetor_execucoes,0\n")
exigir_texto("${csv}" "vetor_estimado_explorados,35\n")
exigir_texto("${csv}" "vetor_estimado_explorados_total,70\n")
exigir_texto("${csv}" "vetor_estimado_s,")
rodar(2 "${ESTUDOS_DIJKSTRA}" --grafo "${TMP}/caminho70.txt" --saida "${TMP}/x" --estimar-vetor 0.5)
rodar(2 "${ESTUDOS_DIJKSTRA}" --grafo "${TMP}/caminho70.txt" --saida "${TMP}/x" --k-heap 5 --k-vetor 6)
rodar(2 "${ESTUDOS_DIJKSTRA}" --saida "${TMP}/x")

# Grafo pequeno (n < 10): estudo P2-1 registra inexistente
rodar(0 "${ESTUDOS_DIJKSTRA}" --grafo "${TMP}/desconexo.txt" --saida "${TMP}/res")
file(READ "${TMP}/res/desconexo_dijkstra.csv" csv)
exigir_texto("${csv}" "dist_10_20,inexistente\n")
exigir_texto("${csv}" "heap_execucoes,6\n")
exigir_texto("${csv}" "vetor_execucoes,6\n")

# Pesos negativos: CSV com status, sem medir
rodar(0 "${ESTUDOS_DIJKSTRA}" --grafo "${DADOS}/figura1_pesos.txt" --saida "${TMP}/res")
file(READ "${TMP}/res/figura1_pesos_dijkstra.csv" csv)
exigir_texto("${csv}" "pesos_negativos,sim\n")
exigir_texto("${csv}" "status,pesos_negativos_dijkstra_nao_aplicavel\n")

# Modo rede: mini-rede com os nomes do enunciado (Turing isolado; virgula e UTF-8 nos nomes)
file(WRITE "${TMP}/mini_rede.txt" "8\n1 2 0.5\n2 3 1\n3 4 0.25\n1 5 2\n5 6 1\n4 6 0.1\n7 8 1\n")
file(WRITE "${TMP}/mini_rede_vertices.txt"
     "1,Edsger W. Dijkstra\n2,Smith, John\n3,J. B. Kruskal\n4,Jon M. Kleinberg\n5,Éva Tardos\n"
     "6,Daniel R. Figueiredo\n7,Alan M. Turing\n8,Ana\n")
rodar(0 "${ESTUDOS_DIJKSTRA}" --rede "${TMP}/mini_rede.txt" --nomes "${TMP}/mini_rede_vertices.txt" --saida "${TMP}/res")
file(READ "${TMP}/res/mini_rede_dijkstra.csv" csv)
exigir_texto("${csv}" "origem_id,1\n")
exigir_texto("${csv}" "rede_turing_distancia,infinito\n")
exigir_texto("${csv}" "rede_kruskal_distancia,1.5\n")
exigir_texto("${csv}" "rede_kruskal_caminho,Edsger W. Dijkstra -> Smith  John -> J. B. Kruskal\n")
exigir_texto("${csv}" "rede_kleinberg_distancia,1.75\n")
exigir_texto("${csv}" "rede_tardos_distancia,2\n")
exigir_texto("${csv}" "rede_figueiredo_distancia,1.85\n")
exigir_texto("${csv}" "rede_figueiredo_caminho_ids,1 2 3 4 6\n")
rodar(2 "${ESTUDOS_DIJKSTRA}" --rede "${TMP}/mini_rede.txt" --saida "${TMP}/res")

file(REMOVE_RECURSE "${TMP}")
message(STATUS "programas_dijkstra: ok")
