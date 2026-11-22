#!/usr/bin/env bash
# Roda `estudos_parte3` (Parte 3) nos grafos dirigidos grafo_W_1..5 e na instancia de validacao.
# Uso: scripts/rodar_estudos_parte3.sh
# Variaveis: DADOS (padrao dados/parte3), RESULTADOS (resultados/parte3), FW_REP (rodadas do
# Floyd-Warshall, padrao 10), BF_TODOS_REP (rodadas completas de Bellman-Ford para todos os
# pares, padrao 10), AMOSTRA (destinos da estimativa do BF todos os pares, padrao 100).
# CSVs ja existentes sao pulados (permite retomar). Ordem: rapidos primeiro; W_2 e W_1 por
# ultimo, pois o Floyd-Warshall (25000^2 x 8 B = 5 GB) leva cerca de 20 min por rodada.

set -u
set -o pipefail

RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXECUTAVEL="${ESTUDOS_PARTE3:-$RAIZ/build/estudos_parte3}"
DADOS="${DADOS:-$RAIZ/dados/parte3}"
RESULTADOS="${RESULTADOS:-$RAIZ/resultados/parte3}"
FW_REP="${FW_REP:-10}"
BF_TODOS_REP="${BF_TODOS_REP:-10}"
AMOSTRA="${AMOSTRA:-100}"
LOGS="$RESULTADOS/logs"

if [ ! -x "$EXECUTAVEL" ]; then
    echo "erro: executavel '$EXECUTAVEL' nao encontrado." >&2
    echo "Compile antes:" >&2
    echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j" >&2
    exit 1
fi
if [ ! -d "$DADOS" ]; then
    echo "erro: pasta '$DADOS' nao existe; coloque nela os grafo_W_*.txt(.gz) da Parte 3." >&2
    exit 1
fi
mkdir -p "$LOGS"

ok=0
pulados=0
falhas=()

rodar() {  # rodar <nome> <argumentos do estudos_parte3...>
    local nome="$1"
    shift
    local csv="$RESULTADOS/${nome}.csv"
    if [ -e "$csv" ]; then
        echo "pulando $nome: $csv ja existe"
        pulados=$((pulados + 1))
        return
    fi
    local log="$LOGS/${nome}.log"
    echo "== $nome ($(date '+%H:%M:%S')) — log em $log"
    if "$EXECUTAVEL" "$@" > "$csv.tmp" 2> "$log"; then
        mv "$csv.tmp" "$csv"
        grep -E '^(dist_|bellman_destino100_media|floyd_media|floyd_status|bf_todos_(media|estimado|status)|conferencia)' "$csv"
        ok=$((ok + 1))
    else
        echo "FALHOU: $nome; veja $log"
        cat "$log"
        rm -f "$csv.tmp"
        falhas+=("$nome")
    fi
}

# 1. Descompacta (mantendo o original) os .gz sem o .txt correspondente.
for compactado in "$DADOS"/grafo_W_*.txt.gz; do
    [ -e "$compactado" ] || continue
    texto="${compactado%.gz}"
    [ -e "$texto" ] && continue
    echo "descompactando $(basename "$compactado")"
    gunzip -k "$compactado" || { rm -f "$texto"; falhas+=("descompactar $(basename "$compactado")"); }
done

# 2. Instancia pequena de validacao: todos os estudos completos.
VALIDACAO="$RESULTADOS/validacao_100.txt"
python3 "$RAIZ/scripts/gerar_validacao_parte3.py" "$VALIDACAO"
rodar validacao_100 "$VALIDACAO" --fw-repeticoes 10 --bf-todos-repeticoes 10
rm -f "$VALIDACAO"

# 3. Grafos grandes (Floyd-Warshall nao cabe na memoria; BF todos os pares por amostra).
for w in 3 4 5; do
    arquivo="$DADOS/grafo_W_$w.txt"
    [ -e "$arquivo" ] || { echo "aviso: $arquivo ausente" >&2; continue; }
    rodar "grafo_W_$w" "$arquivo" --fw-repeticoes "$FW_REP" --bf-todos-amostra "$AMOSTRA"
done

# 4. W_2 e W_1 (n = 25000): Floyd-Warshall e Bellman-Ford para todos os pares completos.
for w in 2 1; do
    arquivo="$DADOS/grafo_W_$w.txt"
    [ -e "$arquivo" ] || { echo "aviso: $arquivo ausente" >&2; continue; }
    rodar "grafo_W_$w" "$arquivo" --fw-repeticoes "$FW_REP" --bf-todos-repeticoes "$BF_TODOS_REP"
done

echo
echo "resumo: ok=$ok pulados=$pulados falhas=${#falhas[@]} ($(date '+%H:%M:%S'))"
for f in "${falhas[@]}"; do
    echo "  falhou: $f"
done
[ ${#falhas[@]} -eq 0 ]
