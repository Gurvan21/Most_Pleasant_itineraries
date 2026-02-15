# Documentation du projet Most Pleasant Itineraries

Documentation technique complète du projet : structure, compilation, et description détaillée des classes **Graph** et **ItinerariesTest**.

---

## 1. Vue d'ensemble

Le projet résout le problème **« itinéraire le plus agréable »** dans un graphe (ville) : pour chaque requête \((u, v)\), on renvoie le **minimum du maximum** des niveaux de bruit sur un chemin entre \(u\) et \(v\). On se ramène à un **arbre couvrant minimal (MST)** puis on propose trois implémentations :

| Version | Prétraitement | Requête | Usage |
|--------|----------------|---------|--------|
| **v1** | Aucun | \(O(n)\) DFS | Référence simple |
| **v2** | \(O(n \log n)\) (centre + binary lifting) | \(O(\log n)\) LCA + max | Requêtes en ligne |
| **v3** | \(O(n + \|P\|)\) Tarjan + \(O(\|P\| \log n)\) max | \(O(1)\) en moyenne (table) | Ensemble de requêtes connu à l'avance |

**Types principaux :** `Vertex` = `int`, `Weight` = `double`, `Edge` = `std::tuple<Vertex, Vertex, Weight>`.

---

## 2. Structure du projet

```
.
├── include/
│   ├── Graph.h           # Classe Graph (graphe, MST, centre, LCA, v1/v2/v3)
│   └── ItinerariesTest.h # Classe ItinerariesTest (chargement, bench, comparaison)
├── src/
│   ├── Graph.cpp         # Implémentation de Graph
│   ├── ItinerariesTest.cpp
│   └── main.cpp          # Point d'entrée (fichier .in → bench + .out)
├── doc/
│   ├── DOCUMENTATION.md  # Ce fichier
│   ├── projet.tex        # Rapport LaTeX
│   └── ...
├── scripts/
│   ├── run_itineraries_with_output.sh  # Lance tous les tests, Runtimes + outputItineraries
│   └── view_dot.py       # Visualisation .dot
├── tests/                # Fichiers .in (n, m, arêtes, Q, requêtes)
├── outputItineraries/    # Fichiers .out (une ligne par requête)
├── Runtimes/             # Par test : v1.txt, v2.txt, v3.txt, preprocessing_v2/3, summary.txt
├── Makefile
└── README.md
```

---

## 3. Compilation et exécution

```bash
make              # → output/main
make run          # Compile puis exécute (sans argument)
make clean        # Supprime .o, .d, exécutable
```

**Lancer un test :**
```bash
./output/main tests/itineraries.0.in                    # sortie .out dans outputItineraries/
./output/main tests/itineraries.0.in /chemin/sortie     # dossier de sortie personnalisé
```

**Variable d'environnement :**
- `SKIP_V1=1` — désactive la version v1 (utile pour les gros tests) ; les réponses écrites viennent de v2.

**Script de batch :**
```bash
./scripts/run_itineraries_with_output.sh                # Tous les tests, parallèle par défaut
START=4 END=9 NPROC=6 ./scripts/run_itineraries_with_output.sh  # Tests 4 à 9, 6 en parallèle
NPROC=1 ./scripts/run_itineraries_with_output.sh        # Séquentiel
```

---

## 4. Format des fichiers

### 4.1 Entrée (fichiers `.in`)

- **Ligne 1 :** `n m` (nombre de sommets, nombre d'arêtes).
- **Lignes 2 à m+1 :** `u v c` (arête entre `u` et `v`, poids `c`), indices **1-indexés**.
- **Ligne m+2 :** `Q` (nombre de requêtes).
- **Lignes suivantes :** `u v` (paire pour chaque requête), 1-indexés.

Si \(m \neq n-1\), le graphe est transformé en MST via Prim depuis le sommet 0 avant de lancer les requêtes.

### 4.2 Sortie (fichiers `.out`)

Une ligne par requête : entier (max sur le chemin optimal, arrondi) ou `-1` si pas de chemin.

### 4.3 Runtimes (script)

Pour chaque test, le script produit dans `Runtimes/itineraries.X/` :

- `v1.txt`, `v2.txt`, `v3.txt` : une ligne par requête (temps en ms ; `N/A` si > 30 s).
- `preprocessing_v2.txt`, `preprocessing_v3.txt` : une ligne = temps de prétraitement (ms).
- `summary.txt` : résumé texte (n, |P|, temps totaux, « Résultats identiques »).

---

## 5. Classe `Graph`

### 5.1 Rôle

Représente un **graphe pondéré** (orienté ou non) en liste d'adjacence, avec :

- gestion des sommets (ajout/suppression, « vivants ») ;
- parcours DFS/BFS ;
- calcul d’un MST (Kruskal, Prim) ;
- pour un **arbre** : centre, parent, LCA (binary lifting), max sur un chemin, et les trois versions d’itinéraires (v1, v2, v3).

Les indices de sommets sont en **0-indexés** dans toute l’API.

### 5.2 Constructeurs

| Signature | Description |
|-----------|-------------|
| `Graph()` | Graphe vide, non orienté. |
| `Graph(adj, directed)` | Graphe construit à partir d’une liste d’adjacence `vector<vector<pair<Vertex,Weight>>>` et d’un booléen `directed`. |

### 5.3 Sommets

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `Vertex add_vertex()` | Ajoute un sommet ; retourne son indice. Réutilise un indice « libéré » si possible. | \(O(1)\) amorti |
| `void remove_vertex(Vertex v)` | Marque \(v\) comme mort (libère l’indice). Invalide centre / parent / table v3. | \(O(\deg(v))\) |
| `int num_vertices() const` | Nombre total d’indices (vivants + morts). | \(O(1)\) |
| `bool is_alive(Vertex v) const` | True si \(v\) est valide et non supprimé. | \(O(1)\) |

### 5.4 Arêtes

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `void add_edge(Vertex u, Vertex v, Weight w)` | Ajoute l’arête \((u,v)\) de poids \(w\). En non orienté, ajoute aussi \((v,u)\). Invalide centre / table v3. | \(O(1)\) |
| `void delete_edge(Vertex u, Vertex v, optional<Weight> w)` | Supprime l’arête (optionnellement avec poids pour cibler une arête précise). | \(O(\deg(u)+\deg(v))\) |
| `const vector<pair<Vertex,Weight>>& neighbors(Vertex u) const` | Référence sur la liste des voisins de \(u\) (poids inclus). Peut contenir des sommets morts ; les algorithmes doivent utiliser `is_alive`. | \(O(1)\) |
| `vector<Edge> get_edges() const` | Toutes les arêtes (chaque arête une fois en non orienté, \(u \leq v\)). | \(O(n+m)\) |

### 5.5 Métadonnées et parcours

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `bool is_directed() const` | Graphe orienté ou non. | \(O(1)\) |
| `int num_edges() const` | Nombre d’arêtes (comptage). | \(O(n+m)\) |
| `vector<Vertex> dfs(Vertex start) const` | Ordre de découverte DFS à partir de `start` (sommets vivants). | \(O(n+m)\) |
| `vector<Vertex> bfs(Vertex start) const` | Ordre de découverte BFS. | \(O(n+m)\) |

### 5.6 Arbre couvrant minimal

Le graphe doit être **non orienté**.

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `Graph kruskal() const` | Retourne un nouveau graphe contenant uniquement les arêtes d’un MST (Kruskal). | \(O(m \log m)\) |
| `Graph prim(Vertex start) const` | Idem avec l’algorithme de Prim depuis `start`. | \(O(m \log n)\) avec file de priorité |

### 5.7 Itinéraires v1 (référence)

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `optional<Weight> itineraries_v1(Vertex u, Vertex v) const` | Maximum des poids sur un chemin \(u\)–\(v\) par **DFS** (graphe quelconque ; en pratique utilisé sur un arbre). Retourne `0` si \(u = v\), `nullopt` si pas de chemin. | \(O(n)\) par requête |

**Implémentation :** DFS depuis \(u\) vers \(v` en propageant le max des poids le long du chemin ; dès que \(v\) est atteint, on renvoie ce max.

### 5.8 Centre, parent et LCA (arbre)

Le graphe est supposé **non orienté et connexe** (arbre). Toutes ces méthodes exigent d’avoir appelé `compute_center_and_parent()` et de ne pas avoir modifié le graphe depuis.

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `void compute_center_and_parent()` | Calcule le **centre** (milieu du diamètre), remplit `parent_`, `parent_edge_weight_`, `depth_`, et les tables de **binary lifting** (`up_`, `max_up_`). | \(O(n \log n)\) |
| `bool has_center() const` | True si le centre est valide. | \(O(1)\) |
| `Vertex get_center() const` | Sommet centre (racine de l’arbre). | \(O(1)\) |
| `int get_diameter_length() const` | Nombre d’arêtes du diamètre. | \(O(1)\) |
| `Vertex get_parent(Vertex v) const` | Parent de \(v` dans l’arbre enraciné au centre ; \(-1\) pour la racine. | \(O(1)\) |
| `optional<Vertex> lca(Vertex u, Vertex v) const` | Plus bas ancêtre commun (binary lifting). | \(O(\log n)\) |
| `vector<optional<Vertex>> tarjan_lca(queries) const` | LCA **hors-ligne** pour toutes les paires dans `queries` (Tarjan). Retourne les LCA dans le même ordre que les paires. | \(O(n + \|P\|)\) |
| `optional<Weight> max_on_path_to_ancestor(Vertex u, Vertex a) const` | Maximum des poids sur le chemin de \(u\) vers l’ancêtre \(a\) (\(a\) doit être ancêtre de \(u\)). Utilise `max_up_`. | \(O(\log n)\) |
| `optional<Weight> itineraries_v2(Vertex u, Vertex v) const` | LCA de \(u,v\) puis `max_on_path_to_ancestor(u, LCA)` et `max_on_path_to_ancestor(v, LCA)` ; renvoie le max des deux. | \(O(\log n)\) |

**Détail de `compute_center_and_parent()` :**

1. **Diamètre :** deux BFS (farthest depuis un sommet, puis farthest depuis ce sommet) → chemin diamètre.
2. **Centre :** sommet au milieu de ce chemin (indice \(L/2\) ou \((L+1)/2\)).
3. **Parent / poids :** un DFS depuis le centre remplit `parent_` et `parent_edge_weight_`.
4. **Binary lifting :** BFS pour `depth_` ; puis tables `up_[v][k]` = \(2^k\)-ième ancêtre de \(v\), et `max_up_[v][k]` = max des poids sur le chemin \(v \to up_[v][k]\). Formule :  
   `up_[v][k] = up_[ up_[v][k-1] ][k-1]`, et `max_up_` = max des deux segments.

### 5.9 Itinéraires v3 (requêtes prétraitées)

| Méthode | Description | Complexité |
|---------|-------------|------------|
| `void preprocess_itineraries_v3(queries)` | Précalcule les réponses pour toutes les paires dans `queries` : Tarjan LCA, puis pour chaque paire deux `max_on_path_to_ancestor`, stocke le max dans `max_path_table_`. À appeler **une fois** quand l’ensemble des requêtes est connu. | \(O(n + \|P\| \log n)\) |
| `optional<Weight> itineraries_v3(Vertex u, Vertex v) const` | Lookup dans `max_path_table_` (paire normalisée \((min(u,v), max(u,v))\)). | \(O(1)\) en moyenne |

### 5.10 Affichage et export

| Méthode | Description |
|---------|-------------|
| `void print_graph(ostream& out) const` | Liste d’adjacence (sommets vivants). |
| `void print_summary(ostream& out) const` | Nombre de sommets, d’arêtes, orienté ou non. |
| `void write_dot(ostream& out, string name) const` | Export au format DOT (Graphviz). |
| `void write_dot_file(path, name) const` | Écrit le DOT dans un fichier. |
| `void write_html_file(path, title) const` | Page HTML avec rendu du graphe. |
| `ostream& operator<<(ostream&, const Graph&)` | Équivalent à `print_graph`. |

### 5.11 Membres privés (résumé)

| Membre | Type | Rôle |
|--------|------|------|
| `cont` | `vector<vector<pair<Vertex,Weight>>>` | Liste d’adjacence. |
| `alive` | `vector<char>` | Sommet vivant ou non. |
| `free_vertices` | `vector<int>` | Indices libérés pour réutilisation. |
| `directed` | `bool` | Graphe orienté ou non. |
| `center_valid_` | `bool` | Centre / parent / binary lifting valides. |
| `centre_` | `Vertex` | Racine (centre de l’arbre). |
| `parent_` | `vector<Vertex>` | Parent dans l’arbre enraciné. |
| `parent_edge_weight_` | `vector<Weight>` | Poids de l’arête vers le parent. |
| `depth_` | `vector<int>` | Profondeur (nombre d’arêtes depuis la racine). |
| `up_` | `vector<vector<Vertex>>` | `up_[v][k]` = \(2^k\)-ième ancêtre de \(v\). |
| `max_up_` | `vector<vector<Weight>>` | Max sur le chemin \(v \to up_[v][k]\). |
| `diameter_length_` | `int` | Longueur du diamètre (nombre d’arêtes). |
| `max_path_table_` | `unordered_map<pair<Vertex,Vertex>, Weight, PairHash>` | Table des réponses v3. |
| `build_binary_lifting(int n)` | fonction privée | Remplit `depth_`, `up_`, `max_up_` après que `centre_`, `parent_`, `parent_edge_weight_` soient remplis. |

**PairHash :** hash pour les paires \((u,v)\) tel que \((u,v)\) et \((v,u)\) aient le même hash (pour la clé de `max_path_table_`).

---

## 6. Classe `ItinerariesTest`

### 6.1 Rôle

- **Charger** un graphe et des requêtes depuis un fichier `.in`.
- **Exécuter** les trois méthodes (v1, v2, v3) sur toutes les requêtes, **chronométrer** (par requête et prétraitements), **comparer** les résultats et **écrire** les réponses dans un fichier `.out` et (via le script) les runtimes dans `Runtimes/`.

### 6.2 Structure `ItinerariesRuntimes`

Utilisée pour retourner des temps agrégés (par exemple pour un script ou un rapport) :

```cpp
struct ItinerariesRuntimes {
    double v1_ms = 0;           // Temps total v1 (requêtes seules)
    double v2_total_ms = 0;    // Prétraitement v2 + requêtes v2
    double v3_total_ms = 0;    // Prétraitement v3 + requêtes v3
    bool timeout = false;
    bool results_identical = false;  // v1 == v2 == v3 (ou v2 == v3 si SKIP_V1)
};
```

### 6.3 Constructeur et chargement

| Méthode | Description |
|---------|-------------|
| `ItinerariesTest()` | Objet vide (défaut). |
| `ItinerariesTest(Graph tree, vector<pair<Vertex,Vertex>> queries)` | Stocke une copie de l’arbre et de la liste de requêtes (paires 0-indexées). |
| `static optional<ItinerariesTest> load_from_file(string path)` | Parse le fichier au format décrit en 4.1. Si \(m \neq n-1\), applique Prim pour obtenir un MST. Retourne `nullopt` en cas d’erreur de lecture ou de format. |

### 6.4 Accesseurs

| Méthode | Description |
|---------|-------------|
| `const Graph& graph() const` | Référence sur l’arbre chargé. |
| `const vector<pair<Vertex,Vertex>>& queries() const` | Référence sur la liste des requêtes. |

### 6.5 Exécution et comparaison

| Méthode | Description |
|---------|-------------|
| `void run_and_compare_times(ostream& out, optional<string> answers_path, ItinerariesRuntimes* runtimes_out) const` | Exécute v1, v2, v3 dans cet ordre, mesure les temps (par requête et prétraitements), affiche un résumé sur `out`. Si `answers_path` est fourni, écrit une ligne par requête (entier arrondi ou -1) dans ce fichier. Si `runtimes_out` est non nul, remplit la structure avec les temps totaux et `results_identical`. |

**Comportement détaillé :**

1. **v1 :** pour chaque requête, appelle `itineraries_v1`, enregistre le temps (et affiche `RUNTIME_V1_QUERIES_START` / une ligne par requête / `RUNTIME_V1_QUERIES_END`). Si **`SKIP_V1=1`** (variable d’environnement), la boucle v1 est sautée (aucune ligne de temps v1 entre START et END).
2. **v2 :** copie de l’arbre, `compute_center_and_parent()`, puis pour chaque requête `itineraries_v2`. Affiche `RUNTIME_V2_PREPROCESSING` (temps du prétraitement), puis `RUNTIME_V2_QUERIES_START` / une ligne par requête / `RUNTIME_V2_QUERIES_END`.
3. **v3 :** sur le même graphe (déjà centre/parent), appelle `preprocess_itineraries_v3(queries_)`, puis pour chaque requête `itineraries_v3`. Affiche `RUNTIME_V3_PREPROCESSING`, puis `RUNTIME_V3_QUERIES_START` / une ligne par requête / `RUNTIME_V3_QUERIES_END`.
4. **Comparaison :** si `SKIP_V1=1`, on compare uniquement v2 et v3 ; sinon v1, v2 et v3. `results_identical` indique si toutes les réponses coïncident.
5. **Fichier de réponses :** si `answers_path` est fourni, les réponses écrites sont celles de v1 (ou de v2 si v1 a été sauté).

**Marqueurs de sortie (pour le script) :**  
Le script `run_itineraries_with_output.sh` parse la sortie standard pour extraire les blocs entre `RUNTIME_V1_QUERIES_START`/`END`, `RUNTIME_V2_PREPROCESSING`, `RUNTIME_V2_QUERIES_START`/`END`, `RUNTIME_V3_PREPROCESSING`, `RUNTIME_V3_QUERIES_START`/`END`, et le résumé après `n = `.

### 6.6 Membres privés

| Membre | Type | Rôle |
|--------|------|------|
| `tree_` | `Graph` | Copie de l’arbre (MST) chargé. |
| `queries_` | `vector<pair<Vertex,Vertex>>` | Liste des paires (0-indexées). |

---

## 7. Point d’entrée (`main.cpp`)

- **Usage :** `./output/main [fichier.in] [dossier_sortie]`
  - Si **au moins un argument** : charge `fichier.in` avec `ItinerariesTest::load_from_file`, déduit le nom du fichier `.out` (ex. `itineraries.0.out`), et appelle `run_and_compare_times(std::cout, out_path, nullptr)`. Le dossier de sortie par défaut est `outputItineraries`.
  - Si **aucun argument** : exécute un bloc de démo (graphe minimal, etc.) si décommenté.
- **Retour :** 0 en cas de succès, 1 si le chargement échoue.

---

## 8. Récapitulatif des complexités

| Opération | Complexité |
|-----------|------------|
| v1 : une requête | \(O(n)\) |
| v2 : prétraitement | \(O(n \log n)\) |
| v2 : une requête | \(O(\log n)\) |
| v3 : prétraitement | \(O(n + \|P\| \log n)\) |
| v3 : une requête | \(O(1)\) en moyenne |
| Tarjan LCA (toutes les paires \(P`) | \(O(n + \|P\|)\) |
| LCA une paire (binary lifting) | \(O(\log n)\) |
| max_on_path_to_ancestor | \(O(\log n)\) |

---

## 9. Références

- **Rapport détaillé :** `doc/projet.tex` (preuves, algorithmes, tâches 1–4).
- **Binary lifting / LCA :** [GeeksforGeeks – Binary Lifting](https://www.geeksforgeeks.org/competitive-programming/binary-lifting-guide-for-competitive-programming/).
- **Sujet et cours :** voir la section Bibliographie dans `projet.tex`.
