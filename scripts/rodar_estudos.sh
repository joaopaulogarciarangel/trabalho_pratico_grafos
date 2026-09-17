#!/usr/bin/env bash
# Roda o programa `estudos` em todos os grafos de dados/, nas representacoes lista e matriz.
# Uso: scripts/rodar_estudos.sh [opcoes extras para estudos, ex.: --limite-ifub 600]
# Combinacoes cujo CSV ja existe em resultados/ sao puladas (permite retomar).

set -u
set -o pipefail

RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXECUTAVEL="${ESTUDOS:-$RAIZ/build/estudos}"
DADOS="$RAIZ/dados"
RESULTADOS="$RAIZ/resultados"
LOGS="$RESULTADOS/logs"

if [ ! -x "$EXECUTAVEL" ]; then
    echo "erro: executavel '$EXECUTAVEL' nao encontrado." >&2
    echo "Compile antes:" >&2
    echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j" >&2
    exit 1
fi
if [ ! -d "$DADOS" ]; then
    echo "erro: pasta '$DADOS' nao existe; coloque os grafos (.txt, .txt.gz ou _txt.gz) nela." >&2
    exit 1
fi
mkdir -p "$LOGS"
shopt -s nullglob

ok=0
pulados=0
falhas=()

# 1. Descompactar, mantendo o original, os .gz que ainda nao tem o .txt correspondente.
for compactado in "$DADOS"/*.txt.gz "$DADOS"/*_txt.gz; do
    case "$compactado" in
        *.txt.gz) texto="${compactado%.gz}" ;;
        *) texto="${compactado%_txt.gz}.txt" ;;
    esac
    [ -e "$texto" ] && continue
    echo "descompactando $(basename "$compactado") -> $(basename "$texto")"
    if [[ "$compactado" == *.txt.gz ]]; then
        gunzip -k "$compactado"
    else
        gunzip -c "$compactado" > "$texto"
    fi
    if [ $? -ne 0 ]; then
        rm -f "$texto"
        falhas+=("descompactar $(basename "$compactado")")
    fi
done

arquivos=("$DADOS"/*.txt)
if [ ${#arquivos[@]} -eq 0 ]; then
    echo "erro: nenhum arquivo .txt em '$DADOS'." >&2
    exit 1
fi

# 2. Estudos: para cada grafo, lista e depois matriz.
for arquivo in "${arquivos[@]}"; do
    nome="$(basename "$arquivo" .txt)"
    for repr in lista matriz; do
        csv="$RESULTADOS/${nome}_${repr}.csv"
        if [ -e "$csv" ]; then
            echo "pulando $nome ($repr): $csv ja existe"
            pulados=$((pulados + 1))
            continue
        fi
        log="$LOGS/${nome}_${repr}.log"
        echo "== $nome ($repr) — log em $log"
        if "$EXECUTAVEL" --grafo "$arquivo" --repr "$repr" --saida "$RESULTADOS" "$@" 2>&1 | tee "$log"; then
            ok=$((ok + 1))
        else
            echo "FALHOU: $nome ($repr); veja $log"
            falhas+=("$nome ($repr)")
        fi
    done
done

echo
echo "resumo: ok=$ok pulados=$pulados falhas=${#falhas[@]}"
for f in "${falhas[@]}"; do
    echo "  falhou: $f"
done
[ ${#falhas[@]} -eq 0 ]
