#!/usr/bin/env python3
"""
Visualise un graphe au format .dot avec NetworkX et matplotlib.
Usage: python view_dot.py [fichier.dot] [-o IMAGE]
       python view_dot.py                    # affiche à l'écran, source: output/graph.dot
       python view_dot.py -o graph.png       # enregistre output/graph.png
       python view_dot.py -o output/demo1.png  # enregistre sous un nom personnalisé
"""

import argparse
import re
import sys
from pathlib import Path

try:
    import networkx as nx
    import matplotlib.pyplot as plt
except ImportError:
    print("Erreur: installez les dépendances avec:", file=sys.stderr)
    print("  pip install networkx matplotlib", file=sys.stderr)
    sys.exit(1)


def parse_dot(path: Path):
    """Parse un fichier .dot (généré par notre C++) et retourne (G, directed)."""
    text = path.read_text()
    directed = "digraph" in text.split("{")[0]
    G = nx.DiGraph() if directed else nx.Graph()

    # Sommets seuls: "  0;"
    for m in re.finditer(r"^\s*(\w+)\s*;", text, re.MULTILINE):
        G.add_node(m.group(1))

    # Arêtes: "  0 -- 1 [label="2"];" ou "  0 -> 1 [label="2"];"
    edge_pat = re.compile(
        r"^\s*(\w+)\s*(?:--|->)\s*(\w+)\s*(?:\[label=[\"']([^\"']*)[\"']\])?\s*;",
        re.MULTILINE,
    )
    for m in edge_pat.finditer(text):
        u, v, label = m.group(1), m.group(2), m.group(3) or ""
        G.add_edge(u, v, label=label)

    return G


def main():
    parser = argparse.ArgumentParser(description="Visualise un graphe .dot avec NetworkX")
    parser.add_argument(
        "dot_file",
        nargs="?",
        default="output/graph.dot",
        help="Fichier .dot à afficher (défaut: output/graph.dot)",
    )
    parser.add_argument(
        "-o", "--output",
        metavar="IMAGE",
        help="Enregistrer la figure dans output/IMAGE (ex: graph.png, demo1.png). Chemin complet possible (ex: output/mon_graphe.png).",
    )
    args = parser.parse_args()

    path = Path(args.dot_file)
    if not path.exists():
        print(f"Fichier introuvable: {path}", file=sys.stderr)
        sys.exit(1)

    G = parse_dot(path)

    # Labels des arêtes (poids)
    edge_labels = {(u, v): data.get("label", "") for u, v, data in G.edges(data=True)}

    # Layout
    pos = nx.spring_layout(G, seed=42, k=1.5)

    plt.figure(figsize=(10, 8))
    nx.draw(
        G,
        pos,
        with_labels=True,
        node_color="#6ab",
        node_size=800,
        font_size=14,
        font_color="white",
        edge_color="#369",
        width=2,
    )
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=10)

    plt.axis("off")
    plt.subplots_adjust(left=0, right=1, top=1, bottom=0)

    if args.output:
        out_path = Path(args.output)
        # Si pas de dossier indiqué, enregistrer dans output/
        if out_path.parent == Path(".") and not str(out_path).startswith("output"):
            out_path = Path("output") / out_path.name
        out_path.parent.mkdir(parents=True, exist_ok=True)
        plt.savefig(out_path, dpi=150, bbox_inches="tight")
        print(f"Figure enregistrée: {out_path}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
