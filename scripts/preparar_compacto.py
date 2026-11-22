"""Converte um grafo .txt.gz sem pesos negativos para arcos binarios compactos.

Formato por arco (little-endian, 12 bytes): origem uint32, destino uint32,
peso em centesimos uint16, reserva uint16. A primeira linha do .gz informa n;
os IDs dos vertices seguem o formato do trabalho (1 a n).
"""

import argparse
import gzip
import struct
from pathlib import Path

ARC = struct.Struct('<IIHH')


def cents(raw: bytes) -> int:
    parts = raw.split(b'.')
    if len(parts) == 1:
        return 100 * int(parts[0])
    if len(parts) != 2 or len(parts[1]) > 2:
        raise ValueError(f'peso fora da precisão centesimal: {raw!r}')
    return 100 * int(parts[0]) + int(parts[1].ljust(2, b'0'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('entrada', type=Path)
    parser.add_argument('saida', type=Path)
    args = parser.parse_args()
    count = 0
    buffer = bytearray()
    with gzip.open(args.entrada, 'rb') as src, args.saida.open('wb') as dst:
        n = int(next(src))
        for line in src:
            if not line.strip():
                continue
            u_raw, v_raw, w_raw = line.split()
            u, v, w = int(u_raw), int(v_raw), cents(w_raw)
            if not (1 <= u <= n and 1 <= v <= n and 0 <= w <= 1000):
                raise ValueError(f'arco inválido na linha {count + 2}')
            buffer.extend(ARC.pack(u, v, w, 0))
            count += 1
            if len(buffer) >= 12_000_000:
                dst.write(buffer)
                buffer.clear()
        if buffer:
            dst.write(buffer)
    print(f'n={n} arcos={count} bytes={args.saida.stat().st_size}')


if __name__ == '__main__':
    main()
