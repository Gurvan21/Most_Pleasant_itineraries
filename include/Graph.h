#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <functional>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>

// --- Types pour clarifier l'API (BFS, DFS, Kruskal, etc.) ---
using Vertex = int;
using Weight = double;
/** Arête : (origine, destination, poids). Pour Kruskal / listes d'arêtes. */
using Edge = std::tuple<Vertex, Vertex, Weight>;

struct PairHash {
    std::size_t operator()(const std::pair<Vertex, Vertex>& p) const {
        const Vertex a = p.first < p.second ? p.first : p.second;
        const Vertex b = p.first < p.second ? p.second : p.first;
        return std::size_t(a) * 1000000007ULL + std::size_t(b);
    }
};

class Graph
{
public:
    Graph();
    explicit Graph(std::vector<std::vector<std::pair<Vertex, Weight>>> adj, bool directed);

    // --- Sommets ---
    Vertex add_vertex();
    void remove_vertex(Vertex v);
    int num_vertices() const;
    /** Les identifiants sont dans [0, num_vertices()-1]; certains peuvent être "mort" (supprimés). */
    bool is_alive(Vertex v) const;

    // --- Arêtes (liste d'adjacence : idéal pour BFS, DFS) ---
    void add_edge(Vertex u, Vertex v, Weight w = 1);
    void delete_edge(Vertex u, Vertex v, std::optional<Weight> w = std::nullopt);
    /** Voisins de u (référence directe). Peut contenir des sommets morts : les algorithmes doivent vérifier is_alive(v). */
    const std::vector<std::pair<Vertex, Weight>>& neighbors(Vertex u) const;
    /** Toutes les arêtes (u, v, w). Graphe non orienté : chaque arête une seule fois (u <= v). Pour Kruskal. */
    std::vector<Edge> get_edges() const;

    // --- Métadonnées ---
    bool is_directed() const;
    int num_edges() const;

    // --- Parcours ---
    /** DFS depuis start : ordre de découverte des sommets (vivants uniquement). */
    std::vector<Vertex> dfs(Vertex start) const;
    /** BFS depuis start : ordre de découverte des sommets (vivants uniquement). */
    std::vector<Vertex> bfs(Vertex start) const;

    // --- Arbre couvrant de poids minimum (graphe non orienté) ---
    /** Kruskal : graphe contenant uniquement les arêtes de l'arbre couvrant minimal. */
    Graph kruskal() const;
    /** Prim depuis start : graphe contenant uniquement les arêtes de l'arbre couvrant minimal. */
    Graph prim(Vertex start) const;

    /** Itineraries v1 : max des poids sur le chemin (u,v) par DFS. */
    std::optional<Weight> itineraries_v1(Vertex u, Vertex v) const;

    // --- Arbre : centre, parent, LCA (graphe supposé non orienté et connexe = arbre) ---
    /** Calcule le centre (milieu du diamètre), le diamètre et remplit le tableau parent.
        À appeler lorsque le graphe est un arbre. Invalide centre/parent en cas de modification du graphe. */
    void compute_center_and_parent();
    /** True si compute_center_and_parent() a été appelé et n'a pas été invalidé. */
    bool has_center() const;
    /** Centre (racine). Précondition : has_center(). */
    Vertex get_center() const;
    /** Nombre d'arêtes du diamètre. Précondition : has_center(). */
    int get_diameter_length() const;
    /** Parent de v dans l'arbre enraciné au centre ; -1 pour la racine. Précondition : has_center(). */
    Vertex get_parent(Vertex v) const;
    /** Plus bas ancêtre commun de u et v. Précondition : has_center(), u et v vivants. */
    std::optional<Vertex> lca(Vertex u, Vertex v) const;
    /** Tarjan LCA hors-ligne : répond à toutes les requêtes dans P en O(n + |P|). Retourne les réponses dans le même ordre que les paires (u,v). */
    std::vector<std::optional<Vertex>> tarjan_lca(const std::vector<std::pair<Vertex, Vertex>>& queries) const;
    /** Max des poids d'arêtes sur le chemin de u vers l'ancêtre a (a doit être ancêtre de u). O(log n). */
    std::optional<Weight> max_on_path_to_ancestor(Vertex u, Vertex a) const;
    /** Itineraries v2 : max sur le chemin (u,v) via LCA + max_on_path_to_ancestor. Précondition : has_center(). */
    std::optional<Weight> itineraries_v2(Vertex u, Vertex v) const;

    /** Précalcul Itineraries v3 (Tarjan + max) pour l'ensemble P. À appeler une fois quand P est connu à l'avance. */
    void preprocess_itineraries_v3(const std::vector<std::pair<Vertex, Vertex>>& queries);
    /** Itineraries v3 : réponse en O(1) en moyenne après preprocess_itineraries_v3. */
    std::optional<Weight> itineraries_v3(Vertex u, Vertex v) const;

    // --- Affichage / observation ---
    /** Liste d'adjacence (sommets vivants uniquement). */
    void print_graph(std::ostream& out) const;
    /** Résumé : nombre de sommets, d'arêtes, orienté ou non. */
    void print_summary(std::ostream& out) const;
    /** Export DOT (Graphviz) : écrire vers out, puis "dot -Tpng fichier.dot -o out.png" ou ouvrir sur graphviz.org. */
    void write_dot(std::ostream& out, const std::string& name = "G") const;
    /** Export DOT dans un fichier. */
    void write_dot_file(const std::string& path, const std::string& name = "G") const;
    /** Génère une page HTML avec dessin du graphe (SVG, layout circulaire). Ouvrable dans le navigateur. */
    void write_html_file(const std::string& path, const std::string& title = "Graphe") const;

    friend std::ostream& operator<<(std::ostream& out, const Graph& graph);

private:
    std::vector<std::vector<std::pair<Vertex, Weight>>> cont;
    std::vector<char> alive;
    std::vector<int> free_vertices;
    bool directed;

    // Centre et parent (arbre enraciné au centre)
    bool center_valid_ = false;
    Vertex centre_ = -1;
    std::vector<Vertex> parent_;
    std::vector<Weight> parent_edge_weight_;  // poids de l'arête (v, parent[v])
    std::vector<int> depth_;                   // profondeur depuis la racine (nombre d'arêtes)
    std::vector<std::vector<Vertex>> up_;      // up_[v][k] = 2^k-ième ancêtre de v
    std::vector<std::vector<Weight>> max_up_;  // max_up_[v][k] = max sur le chemin v -> up_[v][k]
    int diameter_length_ = -1;

    // Table des réponses prétraitées (most pleasant itineraries via Tarjan)
    std::unordered_map<std::pair<Vertex, Vertex>, Weight, PairHash> max_path_table_;

    void build_binary_lifting(int n);
};

#endif