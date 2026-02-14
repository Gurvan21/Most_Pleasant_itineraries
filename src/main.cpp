#include "Graph.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <cmath>


static bool has_edge(const Graph& g, int u, int v, double w, double eps=1e-12) {
    const auto& nb = g.neighbors(u);
    for (const auto& [to, weight] : nb) {
        if (to == v && std::fabs(weight - w) <= eps) return true;
    }
    return false;
}

static int count_edges_to(const Graph& g, int u, int v) {
    int c = 0;
    for (const auto& [to, w] : g.neighbors(u)) {
        if (to == v) ++c;
    }
    return c;
}

int main() {
    std::cout << "Programme démarré.\n" << std::flush;

    /*{
        Graph g;
        for (int i = 0; i < 5; ++i) g.add_vertex();
        g.add_edge(0, 1, 2.0);
        g.add_edge(1, 2, 1.0);
        g.add_edge(2, 3, 4.0);
        g.add_edge(3, 0, 1.0);
        g.add_edge(0, 4, 1.5);
        g.add_edge(4, 2, 2.5);
        g.delete_edge(0,1);
        g.remove_vertex(0);
        g.add_vertex();

        std::cout << "--- Résumé ---\n";
        g.print_summary(std::cout);
        std::cout << "--- Liste d'adjacence ---\n" << g;

        g.write_dot_file("output/graph.dot", "Demo");
        std::cout << "Fichiers générés : output/graph.dot (Graphviz), output/graph.html (navigateur)\n";
    }*/

    // --- Exemples DFS, BFS, Kruskal, Prim (graphe non orienté) ---
    {
        Graph g;
        for (int i = 0; i < 5; ++i) g.add_vertex();
        g.add_edge(0, 1, 2.0);
        g.add_edge(1, 2, 1.0);
        g.add_edge(2, 3, 4.0);
        g.add_edge(3, 0, 1.0);
        g.add_edge(0, 4, 1.5);
        g.add_edge(4, 2, 2.5);

        std::cout << "\n--- Parcours (même graphe non orienté) ---\n";
        std::vector<Vertex> ordre_dfs = g.dfs(0);
        std::cout << "DFS depuis 0 : ";
        for (Vertex v : ordre_dfs) std::cout << v << " ";
        std::cout << "\n";

        std::vector<Vertex> ordre_bfs = g.bfs(0);
        std::cout << "BFS depuis 0 : ";
        for (Vertex v : ordre_bfs) std::cout << v << " ";
        std::cout << "\n";

        std::cout << "\n--- Arbre couvrant minimal (MST) ---\n";
        Graph mst_k = g.kruskal();
        std::cout << "Kruskal :\n" << mst_k;
        Weight total_k = 0;
        for (const Edge& e : mst_k.get_edges()) total_k += std::get<2>(e);
        std::cout << "Poids total Kruskal : " << total_k << "\n\n";

        Graph mst_p = g.prim(0);
        std::cout << "Prim(0) :\n" << mst_p;
        Weight total_p = 0;
        for (const Edge& e : mst_p.get_edges()) total_p += std::get<2>(e);
        std::cout << "Poids total Prim : " << total_p << "\n\n";

        g.write_dot_file("output/graph.dot", "Demo");
        mst_p.write_dot_file("output/mst_prim.dot", "MST_Prim");
        std::cout << "MST (Prim) exporté : output/mst_prim.dot\n";

        // --- Test max_on_path sur l'arbre MST (Prim) ---
        std::cout << "\n--- Max sur le chemin (arbre MST) ---\n";
        auto m1 = mst_p.max_on_path(0, 2);
        auto m2 = mst_p.max_on_path(0, 3);
        auto m3 = mst_p.max_on_path(1, 4);
        auto m_same = mst_p.max_on_path(0, 0);
        if (m1) std::cout << "max_on_path(0, 2) = " << *m1 << "\n";
        else    std::cout << "max_on_path(0, 2) = (non connectés)\n";
        if (m2) std::cout << "max_on_path(0, 3) = " << *m2 << "\n";
        else    std::cout << "max_on_path(0, 3) = (non connectés)\n";
        if (m3) std::cout << "max_on_path(1, 4) = " << *m3 << "\n";
        else    std::cout << "max_on_path(1, 4) = (non connectés)\n";
        if (m_same) std::cout << "max_on_path(0, 0) = " << *m_same << " (chemin vide)\n";
        else        std::cout << "max_on_path(0, 0) = (non connectés)\n";

        // --- Centre, parent, LCA (arbre MST) ---
        std::cout << "\n--- Centre, parent, LCA (arbre MST) ---\n";
        mst_p.compute_center_and_parent();
        if (mst_p.has_center()) {
            std::cout << "Centre (racine) : " << mst_p.get_center() << "\n";
            std::cout << "Diamètre (nombre d'arêtes) : " << mst_p.get_diameter_length() << "\n";
            std::cout << "Parent : ";
            for (int i = 0; i < mst_p.num_vertices(); ++i)
                if (mst_p.is_alive(i))
                    std::cout << "parent[" << i << "]=" << mst_p.get_parent(i) << " ";
            std::cout << "\n";
            auto l1 = mst_p.lca(0, 2);
            auto l2 = mst_p.lca(1, 4);
            auto l3 = mst_p.lca(3, 3);
            if (l1) std::cout << "LCA(0, 2) = " << *l1 << "\n";
            if (l2) std::cout << "LCA(1, 4) = " << *l2 << "\n";
            if (l3) std::cout << "LCA(3, 3) = " << *l3 << "\n";
        }
    }

    return 0;
}
