#include "Graph.h"
#include "ItinerariesTest.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>


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

int main(int argc, char** argv) {
    if (argc >= 2) {
        std::string path = argv[1];
        auto test = ItinerariesTest::load_from_file(path);
        if (test) {
            std::cout << "Fichier : " << path << "\n";
            test->run_and_compare_times();
            return 0;
        }
        std::cerr << "Échec chargement " << path << "\n";
        return 1;
    }

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

        // --- Test itineraries_v1 sur l'arbre MST (Prim) ---
        std::cout << "\n--- Max sur le chemin (arbre MST) ---\n";
        auto m1 = mst_p.itineraries_v1(0, 2);
        auto m2 = mst_p.itineraries_v1(0, 3);
        auto m3 = mst_p.itineraries_v1(1, 4);
        auto m_same = mst_p.itineraries_v1(0, 0);
        if (m1) std::cout << "itineraries_v1(0, 2) = " << *m1 << "\n";
        else    std::cout << "itineraries_v1(0, 2) = (non connectés)\n";
        if (m2) std::cout << "itineraries_v1(0, 3) = " << *m2 << "\n";
        else    std::cout << "itineraries_v1(0, 3) = (non connectés)\n";
        if (m3) std::cout << "itineraries_v1(1, 4) = " << *m3 << "\n";
        else    std::cout << "itineraries_v1(1, 4) = (non connectés)\n";
        if (m_same) std::cout << "itineraries_v1(0, 0) = " << *m_same << " (chemin vide)\n";
        else        std::cout << "itineraries_v1(0, 0) = (non connectés)\n";

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
            // max_on_path_to_ancestor (O(log n)) : u -> ancêtre a
            auto r1 = mst_p.max_on_path_to_ancestor(2, 0);  // chemin 2 -> 1 -> 0, max = 2
            auto r2 = mst_p.max_on_path_to_ancestor(4, 0);   // 4 -> 0, max = 1.5
            if (r1) std::cout << "max_on_path_to_ancestor(2, 0) = " << *r1 << "\n";
            if (r2) std::cout << "max_on_path_to_ancestor(4, 0) = " << *r2 << "\n";

            // --- Test itineraries_v2 : doit égaler itineraries_v1 pour toute paire ---
            std::cout << "\n--- Test itineraries_v2 (vs itineraries_v1) ---\n";
            bool ok = true;
            for (int u = 0; u < mst_p.num_vertices(); ++u) {
                if (!mst_p.is_alive(u)) continue;
                for (int v = 0; v < mst_p.num_vertices(); ++v) {
                    if (!mst_p.is_alive(v)) continue;
                    auto old_val = mst_p.itineraries_v1(u, v);
                    auto new_val = mst_p.itineraries_v2(u, v);
                    if (old_val != new_val) {
                        std::cout << "  Différence (" << u << "," << v << "): v1=";
                        if (old_val) std::cout << *old_val; else std::cout << "null";
                        std::cout << " v2=";
                        if (new_val) std::cout << *new_val; else std::cout << "null";
                        std::cout << "\n";
                        ok = false;
                    }
                }
            }
            if (ok) std::cout << "  OK : itineraries_v1 et itineraries_v2 coïncident.\n";

            // --- Test Tarjan LCA (hors-ligne) ---
            std::vector<std::pair<Vertex, Vertex>> qs = {{0, 2}, {1, 4}, {3, 3}, {2, 4}};
            auto tarjan_ans = mst_p.tarjan_lca(qs);
            std::cout << "\n--- Tarjan LCA (hors-ligne) ---\n";
            for (size_t i = 0; i < qs.size(); ++i) {
                auto ref = mst_p.lca(qs[i].first, qs[i].second);
                std::cout << "  LCA(" << qs[i].first << "," << qs[i].second << ") = ";
                if (tarjan_ans[i]) std::cout << *tarjan_ans[i]; else std::cout << "null";
                std::cout << (tarjan_ans[i] == ref ? " (OK)" : " [diff]") << "\n";
            }

            // --- Itineraries v3 : précalcul Tarjan + table (requêtes en O(1)) ---
            std::cout << "\n--- Itineraries v3 (most pleasant itineraries) ---\n";
            std::vector<std::pair<Vertex, Vertex>> P;
            for (int u = 0; u < mst_p.num_vertices(); ++u)
                if (mst_p.is_alive(u))
                    for (int v = u; v < mst_p.num_vertices(); ++v)
                        if (mst_p.is_alive(v))
                            P.emplace_back(u, v);
            mst_p.preprocess_itineraries_v3(P);
            std::cout << "Prétraitement pour |P| = " << P.size() << " requêtes (linéaire en n + |P|).\n";
            bool ok_final = true;
            for (const auto& [u, v] : P) {
                auto ref_w = mst_p.itineraries_v1(u, v);
                auto tab_w = mst_p.itineraries_v3(u, v);
                if (ref_w != tab_w) ok_final = false;
            }
            std::cout << "Réponses en O(1) en moyenne : " << (ok_final ? "toutes cohérentes avec itineraries_v1.\n" : "erreur.\n");
            std::cout << "Exemples itineraries_v3(0,2) = " << *mst_p.itineraries_v3(0, 2)
                      << ", itineraries_v3(1,4) = " << *mst_p.itineraries_v3(1, 4) << "\n";
        }
    }

    return 0;
}
