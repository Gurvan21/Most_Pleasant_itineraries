#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <functional>
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

    /** Maximum des poids d'arêtes sur l'unique chemin entre u et v (graphe supposé arbre).
        Retourne std::nullopt si u et v ne sont pas connectés ; si u == v, retourne 0 (chemin vide). */
    std::optional<Weight> max_on_path(Vertex u, Vertex v) const;

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
};

#endif