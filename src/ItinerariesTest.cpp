#include "ItinerariesTest.h"
#include <fstream>
#include <iomanip>
#include <sstream>

ItinerariesTest::ItinerariesTest(Graph tree, std::vector<std::pair<Vertex, Vertex>> queries)
    : tree_(std::move(tree)), queries_(std::move(queries)) {}

std::optional<ItinerariesTest> ItinerariesTest::load_from_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) return std::nullopt;
    int n = 0, m = 0;
    if (!(f >> n >> m)) return std::nullopt;
    if (n < 1 || m < 0) return std::nullopt;

    Graph g;
    for (int i = 0; i < n; ++i) g.add_vertex();
    for (int i = 0; i < m; ++i) {
        int u = 0, v = 0;
        Weight c = 0;
        if (!(f >> u >> v >> c)) return std::nullopt;
        if (u < 1 || u > n || v < 1 || v > n) return std::nullopt;
        g.add_edge(u - 1, v - 1, c);
    }
    if (m != n - 1) {
        g = g.prim(0);
    }

    int Q = 0;
    if (!(f >> Q)) return std::nullopt;
    if (Q < 0) return std::nullopt;
    std::vector<std::pair<Vertex, Vertex>> queries;
    queries.reserve(static_cast<size_t>(Q));
    for (int i = 0; i < Q; ++i) {
        int u = 0, v = 0;
        if (!(f >> u >> v)) return std::nullopt;
        if (u < 1 || u > n || v < 1 || v > n) return std::nullopt;
        queries.emplace_back(u - 1, v - 1);
    }

    return ItinerariesTest(std::move(g), std::move(queries));
}

void ItinerariesTest::run_and_compare_times(std::ostream& out) const {
    if (queries_.empty()) {
        out << "Aucune requête.\n";
        return;
    }
    using Clock = std::chrono::high_resolution_clock;
    using Ms = std::chrono::duration<double, std::milli>;

    std::vector<std::optional<Weight>> res_v1(queries_.size()), res_v2(queries_.size()), res_v3(queries_.size());
    const int n = tree_.num_vertices();

    // --- v1 : pas de prétraitement, uniquement les requêtes ---
    Graph g1 = tree_;
    auto t0 = Clock::now();
    for (size_t i = 0; i < queries_.size(); ++i)
        res_v1[i] = g1.itineraries_v1(queries_[i].first, queries_[i].second);
    auto t1 = Clock::now();
    double ms_v1 = std::chrono::duration_cast<Ms>(t1 - t0).count();

    // --- v2 : prétraitement (centre + parent + binary lifting) puis requêtes ---
    Graph g2 = tree_;
    auto t2_start = Clock::now();
    g2.compute_center_and_parent();
    auto t2_pre_end = Clock::now();
    if (!g2.has_center()) {
        out << "Erreur : compute_center_and_parent a échoué (graphe non connexe ?).\n";
        return;
    }
    for (size_t i = 0; i < queries_.size(); ++i)
        res_v2[i] = g2.itineraries_v2(queries_[i].first, queries_[i].second);
    auto t2_end = Clock::now();
    double ms_pre_v2 = std::chrono::duration_cast<Ms>(t2_pre_end - t2_start).count();
    double ms_queries_v2 = std::chrono::duration_cast<Ms>(t2_end - t2_pre_end).count();
    double ms_v2_total = ms_pre_v2 + ms_queries_v2;

    // --- v3 : prétraitement (Tarjan + table) puis requêtes (réutilise le centre de g2) ---
    auto t3_pre_start = Clock::now();
    g2.preprocess_itineraries_v3(queries_);
    auto t3_pre_end = Clock::now();
    for (size_t i = 0; i < queries_.size(); ++i)
        res_v3[i] = g2.itineraries_v3(queries_[i].first, queries_[i].second);
    auto t3_end = Clock::now();
    double ms_pre_v3 = std::chrono::duration_cast<Ms>(t3_pre_end - t3_pre_start).count();
    double ms_queries_v3 = std::chrono::duration_cast<Ms>(t3_end - t3_pre_end).count();
    double ms_v3_total = ms_pre_v3 + ms_queries_v3;

    bool ok = true;
    for (size_t i = 0; i < queries_.size(); ++i) {
        if (res_v1[i] != res_v2[i] || res_v1[i] != res_v3[i]) ok = false;
    }

    out << "n = " << n << ", |P| = " << queries_.size() << "\n";
    out << std::fixed << std::setprecision(3);
    out << "  itineraries_v1 : " << ms_v1 << " ms (requêtes uniquement, pas de prétraitement)\n";
    out << "  itineraries_v2 : prétraitement " << ms_pre_v2 << " ms + requêtes " << ms_queries_v2 << " ms = total " << ms_v2_total << " ms\n";
    out << "  itineraries_v3 : prétraitement " << ms_pre_v3 << " ms + requêtes " << ms_queries_v3 << " ms = total " << ms_v3_total << " ms\n";
    out << "  Résultats identiques : " << (ok ? "oui" : "non") << "\n";
}
