#!/usr/bin/env bash
# Roda o programa `estudos_dijkstra` (Parte 2) nos grafos com pesos de dados/ e na rede de
# colaboracao. Uso: scripts/rodar_estudos_dijkstra.sh [opcoes extras, ex.: --semente 7]
# Execucoes cujo CSV ja existe em resultados/parte2/ sao puladas (permite retomar).

set -u
set -o pipefail

RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXECUTAVEL="${ESTUDOS_DIJKSTRA:-$RAIZ/build/estudos_dijkstra}"
DADOS="${DADOS:-$RAIZ/dados}"
RESULTADOS="${RESULTADOS:-$RAIZ/resultados/parte2}"
LOGS="$RESULTADOS/logs"

# Origens por grafo no estudo de tempo. O Dijkstra com vetor e O(n^2): k-vetor escolhido para
# caber em cerca de 1,5 h por grafo; nos grafos com 10^7 vertices ou mais (uma execucao levaria
# de um dia a semanas) o tempo do vetor e estimado por uma execucao parcial (fracao dos vertices
# explorados, menor no W_7 para a parcial durar poucos minutos). No W_7 (5x10^7 vertices, ~90 s
# por execucao com heap e memoria perto do limite do WSL) o heap usa 30 origens.
declare -A K_HEAP=([grafo_W_7]=30)
declare -A K_VETOR=([grafo_W_3]=100 [grafo_W_4]=10 [grafo_W_5]=0 [grafo_W_6]=0 [grafo_W_7]=0)
declare -A ESTIMAR=([grafo_W_5]=0.01 [grafo_W_6]=0.01 [grafo_W_7]=0.0001)
K_HEAP_PADRAO=100
K_VETOR_PADRAO=0       # grafos sem entrada na tabela: so a estimativa
ESTIMAR_PADRAO=0.01

if [ ! -x "$EXECUTAVEL" ]; then
    echo "erro: executavel '$EXECUTAVEL' nao encontrado." >&2
    echo "Compile antes:" >&2
    echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j" >&2
    exit 1
fi
if [ ! -d "$DADOS" ]; then
    echo "erro: pasta '$DADOS' nao existe; coloque os grafos grafo_W_*.txt(.gz) e a rede nela." >&2
    exit 1
fi
mkdir -p "$LOGS"
shopt -s nullglob

ok=0
pulados=0
falhas=()

rodar() {  # rodar <nome> <argumentos do estudos_dijkstra...>
    local nome="$1"
    shift
    local csv="$RESULTADOS/${nome}_dijkstra.csv"
    if [ -e "$csv" ]; then
        echo "pulando $nome: $csv ja existe"
        pulados=$((pulados + 1))
        return
    fi
    local log="$LOGS/${nome}.log"
    echo "== $nome — log em $log"
    if "$EXECUTAVEL" "$@" --saida "$RESULTADOS" 2>&1 | tee "$log"; then
        ok=$((ok + 1))
    else
        echo "FALHOU: $nome; veja $log"
        falhas+=("$nome")
    fi
}

# 1. Descompactar, mantendo o original, os grafo_W_*.txt.gz sem o .txt correspondente.
for compactado in "$DADOS"/grafo_W_*.txt.gz; do
    texto="${compactado%.gz}"
    [ -e "$texto" ] && continue
    echo "descompactando $(basename "$compactado") -> $(basename "$texto")"
    if ! gunzip -k "$compactado"; then
        rm -f "$texto"
        falhas+=("descompactar $(basename "$compactado")")
    fi
done

# 2. Grafos com pesos (P2-1 e P2-2), em ordem natural (W_3, W_4, ..., W_10).
arquivos=("$DADOS"/grafo_W_*.txt)
if [ ${#arquivos[@]} -eq 0 ]; then
    echo "aviso: nenhum grafo_W_*.txt em '$DADOS'." >&2
fi
mapfile -t arquivos < <(printf '%s\n' "${arquivos[@]}" | sort -V)
for arquivo in "${arquivos[@]}"; do
    [ -n "$arquivo" ] || continue
    nome="$(basename "$arquivo" .txt)"
    k_heap="${K_HEAP[$nome]:-$K_HEAP_PADRAO}"
    k_vetor="${K_VETOR[$nome]:-$K_VETOR_PADRAO}"
    extra=()
    if [ "$k_vetor" -eq 0 ]; then
        extra=(--estimar-vetor "${ESTIMAR[$nome]:-$ESTIMAR_PADRAO}")
    fi
    rodar "$nome" --grafo "$arquivo" --k-heap "$k_heap" --k-vetor "$k_vetor" "${extra[@]}" "$@"
done

# 3. Rede de colaboracao (P2-3).
if [ -e "$DADOS/rede_colaboracao.txt" ] && [ -e "$DADOS/rede_colaboracao_vertices.txt" ]; then
    rodar rede_colaboracao --rede "$DADOS/rede_colaboracao.txt" --nomes "$DADOS/rede_colaboracao_vertices.txt"
else
    echo "aviso: rede_colaboracao.txt e rede_colaboracao_vertices.txt nao encontrados em '$DADOS'." >&2
fi

echo
echo "resumo: ok=$ok pulados=$pulados falhas=${#falhas[@]}"
for f in "${falhas[@]}"; do
    echo "  falhou: $f"
done
[ ${#falhas[@]} -eq 0 ]
