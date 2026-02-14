# Centre d’un arbre, tableau parent, et LCA — Pseudocode et remarques

## 1. Ton idée (résumée)

1. Partir d’un sommet quelconque, faire un DFS pour trouver un sommet **le plus éloigné** (en nombre d’arêtes).
2. Repartir de ce sommet, refaire un DFS pour trouver à nouveau un sommet le plus éloigné → on obtient un **diamètre** (longueur = nombre d’arêtes du plus long chemin).
3. Le **centre** = sommet (ou arête du milieu) de ce diamètre ; le stocker dans un attribut.
4. Stocker pour chaque sommet son **parent** dans un vecteur (arbre enraciné au centre).
5. **LCA(u, v)** : remonter de u et v vers le centre en stockant les ancêtres, puis trouver le premier ancêtre commun.

---

## 2. Avis global

- **Diamètre par deux BFS/DFS** : C’est la méthode classique et correcte. En partant d’un sommet arbitraire, un sommet à distance max est une extrémité d’un diamètre ; en repartant de cette extrémité, le plus éloigné donne l’autre bout et la longueur du diamètre. Donc ton idée est bonne.
- **Centre = milieu du diamètre** : C’est bien la définition usuelle du centre d’un arbre (minimiser la distance max, ou être au milieu d’un diamètre). Il faut seulement préciser : si le diamètre a un **nombre pair d’arêtes**, le “milieu” est une **arête** (donc 2 sommets centraux) ; si **impair**, c’est un **sommet** unique. Pour le parent et le LCA, on a besoin d’une **racine** unique : tu peux choisir un des 1 ou 2 centres (par ex. celui d’indice min) et le stocker comme attribut “centre” / racine.
- **Parent** : Une fois la racine (centre) fixée, un DFS depuis la racine remplit bien `parent[v]` pour tout `v`. Standard et correct.
- **LCA en remontant vers le centre** : Oui, remonter de u et v en suivant `parent[]` et comparer les ancêtres donne bien le LCA. C’est correct ; la complexité est O(hauteur) par requête, donc O(n) dans le pire cas. Pour beaucoup de requêtes on peut faire mieux (ex. binary lifting), mais pour commencer c’est très bien.

Points à bien faire dans l’implémentation :
- Lors des deux BFS/DFS, il faut **retourner le chemin** (séquence de sommets) jusqu’au sommet le plus éloigné, pas seulement ce sommet, pour pouvoir ensuite prendre le “milieu” du diamètre.
- Gérer proprement le cas “centre = arête du milieu” (2 sommets) en choisissant une racine unique.

---

## 3. Pseudocode

### 3.1 Trouver un sommet le plus éloigné et le chemin jusqu’à lui

On suppose que le graphe est un **arbre** (connexe, sans cycle). On utilise un BFS (ou DFS) depuis `start` ; on retourne un sommet à distance maximale **et** le chemin `start → ... → farthest`.

```
Fonction FARTEST_ET_CHEMIN(arbre, start):
    soit dist[0..n-1] = -1, parent_bfs[0..n-1] = -1
    file = [start], dist[start] = 0
    tant que file non vide:
        u = défiler(file)
        pour chaque voisin v de u (avec v ≠ parent pour éviter de remonter, ou utiliser dist[v] == -1):
            si dist[v] == -1:
                dist[v] = dist[u] + 1
                parent_bfs[v] = u
                enfiler(file, v)
    farthest = argmax_{sommets v} dist[v]   // un sommet avec dist max
    // Reconstruire le chemin start → farthest en remontant parent_bfs
    path = liste vide
    v = farthest
    tant que v ≠ -1:
        ajouter v au début de path
        v = parent_bfs[v]
    retourner (farthest, path)
```

En **DFS** (récursif), on peut faire une recherche du sommet le plus loin en propageant la profondeur et en gardant le meilleur sommet + le chemin (en remontant, on reconstruit le chemin). Équivalent en idée.

---

### 3.2 Diamètre et centre

```
Fonction DIAMETRE_ET_CENTRE(arbre):
    s = un sommet quelconque (ex: 0)
    (u, path_s_u) = FARTEST_ET_CHEMIN(arbre, s)
    (v, path_diametre) = FARTEST_ET_CHEMIN(arbre, u)
    // path_diametre = chemin u → v = un diamètre (liste de sommets)
    L = longueur(path_diametre) - 1   // nombre d'arêtes = nombre de sommets - 1

    si L pair (ex: L = 2k):
        // centre = l'arête du milieu entre path_diametre[k] et path_diametre[k+1]
        // on choisit une racine unique, par ex. path_diametre[k]
        centre = path_diametre[k]
    sinon (L impair, ex: L = 2k+1):
        centre = path_diametre[k+1]   // sommet du milieu (indices 0..L, milieu = k+1)

    retourner (path_diametre, L, centre)
```

Tu peux stocker `centre` (un sommet) et éventuellement `diametre_path` / `diametre_longueur` comme attributs si tu en as besoin ailleurs.

---

### 3.3 Tableau parent (arbre enraciné au centre)

Une fois la racine `centre` fixée, un DFS (ou BFS) depuis `centre` remplit `parent` en évitant de remonter (comme pour le DFS “max sur chemin”) :

```
Fonction REMPLIR_PARENT(arbre, racine):
    parent[0..n-1] = -1   // -1 = pas de parent
    parent[racine] = racine   // ou -1, selon convention (racine n'a pas de parent)
    DFS_PARENT(arbre, racine, -1, parent)

Fonction DFS_PARENT(arbre, current, from, parent):
    pour chaque voisin v de current:
        si v ≠ from:
            parent[v] = current
            DFS_PARENT(arbre, v, current, parent)
```

Convention possible : `parent[racine] = -1` pour indiquer “pas de parent”.

---

### 3.4 LCA(u, v) par remontée des ancêtres

On remonte depuis u en enregistrant tous les ancêtres (u inclus), puis on remonte depuis v jusqu’à tomber sur un ancêtre de u ; ce sera le LCA.

```
Fonction LCA(u, v):
    ancetres_u = ensemble vide
    w = u
    tant que w ≠ -1 (ou w ≠ racine selon convention):
        ajouter w à ancetres_u
        w = parent[w]
    w = v
    tant que w ∉ ancetres_u:
        w = parent[w]
    retourner w   // premier ancêtre de v qui est aussi ancêtre de u = LCA(u,v)
```

Variante : construire la liste des ancêtres de u, puis pour v remonter en testant à chaque pas si le sommet courant est dans cette liste ; le premier trouvé est le LCA. Complexité : O(hauteur) par requête, donc O(n) dans le pire cas.

---

## 4. Résumé des attributs proposés

| Attribut | Type | Rôle |
|----------|------|------|
| `centre` | `Vertex` | Un sommet centre de l’arbre (racine pour parent / LCA). |
| `parent` | `std::vector<int>` ou `std::vector<Vertex>` | `parent[v]` = parent de v dans l’arbre enraciné en `centre`. `parent[centre] = -1`. |
| (optionnel) | `diametre_longueur`, `diametre_path` | Pour debug ou affichage du diamètre. |

---

## 5. Mise à jour des autres fonctions

- **Quand mettre à jour centre et parent ?**  
  Dès que le graphe (arbre) est fixé : après construction, ou après chaque modification si tu veux que centre/parent restent cohérents. Si le graphe peut changer (ajout/suppression d’arêtes), il faudra soit recalculer centre + parent, soit documenter que ces attributs ne sont valides que pour l’arbre au moment du dernier appel à “calcul centre / parent”.
- **Fonctions existantes** :  
  - `dfs`, `bfs`, `max_on_path`, etc. ne dépendent pas de `centre` ou `parent`.  
  - Les nouvelles fonctions seront du type : `compute_center_and_parent()` (ou `root_at_center()`), `get_center()`, `get_parent(v)`, `lca(u,v)`.  
  - Si tu ajoutes des sommets/arêtes après un calcul de centre, il faut soit recalculer, soit interdire les modifications après “enracinement”.

---

## 6. Convention sur “centre” quand le diamètre a un nombre pair d’arêtes

Pour LCA et parent, il faut **une seule** racine. Si le diamètre a 2k arêtes, il y a deux sommets “au milieu” (les deux extrémités de l’arête centrale). Tu peux :

- Stocker `centre` = l’un des deux (par ex. celui d’indice min dans le chemin), et enraciner l’arbre en ce sommet.  
- Ou stocker `centre1`, `centre2` si tu veux exposer les deux ; pour `parent` et LCA, tu choisis quand même une racine (ex. `centre1`).

Le pseudocode ci-dessus choisit une racine unique dans tous les cas.
