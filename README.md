# Most Pleasant Itineraries

Bibliothèque C++ de graphes pondérés avec parcours (DFS, BFS), arbre couvrant minimal (Kruskal, Prim) et maximum sur le chemin dans un arbre.

## Structure du projet

```
.
├── include/          # En-têtes
│   └── Graph.h
├── src/              # Sources C++
│   ├── Graph.cpp
│   └── main.cpp
├── doc/              # Documentation LaTeX
│   └── max_on_path_tree.tex
├── scripts/          # Scripts Python (visualisation)
│   ├── view_dot.py
│   └── requirements.txt
├── output/           # Binaire et fichiers générés (.dot, .png, .html)
├── Makefile
└── README.md
```

## Prérequis

- **Compilation :** `g++` avec support C++17
- **Visualisation (optionnel) :** Python 3, `networkx`, `matplotlib`

## Compilation et exécution

```bash
make          # Compile → exécutable dans output/main
make run      # Compile puis exécute
make clean    # Supprime .o, .d et l'exécutable
```

L’exécutable est généré dans `output/main`. Les commandes sont à lancer depuis la racine du projet (dossier contenant le `Makefile`).

## Fonctionnalités (classe `Graph`)

- **Graphe :** sommets (ajout/suppression), arêtes pondérées, orienté ou non.
- **Parcours :** `dfs(start)`, `bfs(start)` — ordre de découverte des sommets.
- **Arbre couvrant minimal (graphe non orienté) :**
  - `kruskal()` — retourne un `Graph` (MST).
  - `prim(start)` — idem à partir de `start`.
- **Arbre :** `max_on_path(u, v)` — maximum des poids d’arêtes sur l’unique chemin entre `u` et `v` (retourne `std::optional<Weight>`).
- **Export :** `write_dot_file(path, name)` — format DOT (Graphviz).

Types principaux : `Vertex`, `Weight`, `Edge` (tuple `(u, v, w)`).

## Visualisation des graphes

Le programme écrit des fichiers `.dot` dans `output/`. Pour les visualiser avec le script Python :

```bash
pip install -r scripts/requirements.txt   # ou: pip install networkx matplotlib
python3 scripts/view_dot.py                      # affiche output/graph.dot
python3 scripts/view_dot.py output/mst_prim.dot   # affiche le MST
python3 scripts/view_dot.py -o mst.png           # enregistre output/mst.png
```

Les fichiers `.dot` peuvent aussi être ouverts sur [graphviz.org](https://graphviz.org) ou convertis en PNG avec `dot -Tpng fichier.dot -o fichier.png`.

## Documentation

- **Maximum sur le chemin dans un arbre :** principe, pseudocode, preuve de correction, terminaison et complexité dans `doc/max_on_path_tree.tex`. Compilation PDF :
  ```bash
  cd doc && pdflatex max_on_path_tree.tex
  ```

## Licence

Projet à usage pédagogique / personnel.
