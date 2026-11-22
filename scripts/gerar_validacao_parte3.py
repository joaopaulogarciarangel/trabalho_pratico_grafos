"""Gera a pequena instancia dirigida usada para validar a comparacao de tempos."""

import argparse
import random
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("saida", type=Path)
    args = parser.parse_args()
    rng = random.Random(242)
    with args.saida.open("w", encoding="ascii", newline="\n") as f:
        f.write("100\n")
        for _ in range(600):
            f.write(f"{rng.randrange(1, 101)} {rng.randrange(1, 101)} {rng.randrange(1, 11)}\n")


if __name__ == "__main__":
    main()
