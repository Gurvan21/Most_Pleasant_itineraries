#include "ItinerariesTest.h"
#include <cmath>
#include <cstdlib>
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

namespace {
constexpr double QUERY_TIMEOUT_MS = 30000.0;
}

void ItinerariesTest::run_and_compare_times(std::ostream& out,
                                             const std::optional<std::string>& answers_path,
                                             ItinerariesRuntimes* runtimes_out) const {
    if (queries_.empty()) {
        out << "Aucune requête.\n";
        return;
    }
    using Clock = std::chrono::high_resolution_clock;
    using Ms = std::chrono::duration<double, std::milli>;

    std::vector<std::optional<Weight>> res_v1(queries_.size()), res_v2(queries_.size()), res_v3(queries_.size());
    const int n = tree_.num_vertices();
    const bool skip_v1 = (std::getenv("SKIP_V1") && std::atoi(std::getenv("SKIP_V1")) != 0);

    double ms_v1_total = 0;
    out << "RUNTIME_V1_QUERIES_START\n";
    if (!skip_v1) {
        Graph g1 = tree_;
        for (size_t i = 0; i < queries_.size(); ++i) {
            auto t0 = Clock::now();
            res_v1[i] = g1.itineraries_v1(queries_[i].first, queries_[i].second);
            auto t1 = Clock::now();
            double ms = std::chrono::duration_cast<Ms>(t1 - t0).count();
            ms_v1_total += ms;
            if (ms >= QUERY_TIMEOUT_MS)
                out << "N/A\n";
            else
                out << std::fixed << std::setprecision(6) << ms << "\n";
        }
    }
    out << "RUNTIME_V1_QUERIES_END\n";

    Graph g2 = tree_;
    auto t2_pre0 = Clock::now();
    g2.compute_center_and_parent();
    auto t2_pre1 = Clock::now();
    double ms_pre_v2 = std::chrono::duration_cast<Ms>(t2_pre1 - t2_pre0).count();
    if (!g2.has_center()) {
        out << "Erreur : compute_center_and_parent a échoué (graphe non connexe ?).\n";
        return;
    }
    out << "RUNTIME_V2_PREPROCESSING " << std::fixed << std::setprecision(6) << ms_pre_v2 << "\n";
    double ms_v2_queries_total = 0;
    out << "RUNTIME_V2_QUERIES_START\n";
    for (size_t i = 0; i < queries_.size(); ++i) {
        auto t0 = Clock::now();
        res_v2[i] = g2.itineraries_v2(queries_[i].first, queries_[i].second);
        auto t1 = Clock::now();
        double ms = std::chrono::duration_cast<Ms>(t1 - t0).count();
        ms_v2_queries_total += ms;
        if (ms >= QUERY_TIMEOUT_MS)
            out << "N/A\n";
        else
            out << std::fixed << std::setprecision(6) << ms << "\n";
    }
    out << "RUNTIME_V2_QUERIES_END\n";
    double ms_v2_total = ms_pre_v2 + ms_v2_queries_total;

    double ms_pre_v3 = 0;
    {
        auto t0 = Clock::now();
        g2.preprocess_itineraries_v3(queries_);
        auto t1 = Clock::now();
        ms_pre_v3 = std::chrono::duration_cast<Ms>(t1 - t0).count();
    }
    out << "RUNTIME_V3_PREPROCESSING " << std::fixed << std::setprecision(6) << ms_pre_v3 << "\n";
    double ms_v3_queries_total = 0;
    out << "RUNTIME_V3_QUERIES_START\n";
    for (size_t i = 0; i < queries_.size(); ++i) {
        auto t0 = Clock::now();
        res_v3[i] = g2.itineraries_v3(queries_[i].first, queries_[i].second);
        auto t1 = Clock::now();
        double ms = std::chrono::duration_cast<Ms>(t1 - t0).count();
        ms_v3_queries_total += ms;
        if (ms >= QUERY_TIMEOUT_MS)
            out << "N/A\n";
        else
            out << std::fixed << std::setprecision(6) << ms << "\n";
    }
    out << "RUNTIME_V3_QUERIES_END\n";
    double ms_v3_total = ms_pre_v3 + ms_v3_queries_total;

    bool ok = true;
    if (skip_v1) {
        for (size_t i = 0; i < queries_.size(); ++i) {
            if (res_v2[i] != res_v3[i]) ok = false;
        }
    } else {
        for (size_t i = 0; i < queries_.size(); ++i) {
            if (res_v1[i] != res_v2[i] || res_v1[i] != res_v3[i]) ok = false;
        }
    }

    if (runtimes_out) {
        runtimes_out->v1_ms = ms_v1_total;
        runtimes_out->v2_total_ms = ms_v2_total;
        runtimes_out->v3_total_ms = ms_v3_total;
        runtimes_out->results_identical = ok;
    }

    if (answers_path) {
        std::ofstream f(*answers_path);
        if (f) {
            const auto& ref = skip_v1 ? res_v2 : res_v1;
            for (size_t i = 0; i < queries_.size(); ++i) {
                const auto& r = ref[i];
                if (r)
                    f << static_cast<int>(std::round(*r)) << "\n";
                else
                    f << "-1\n";
            }
        }
    }

    out << "n = " << n << ", |P| = " << queries_.size() << "\n";
    out << std::fixed << std::setprecision(3);
    if (skip_v1)
        out << "  itineraries_v1 : (ignoré, SKIP_V1=1)\n";
    else
        out << "  itineraries_v1 : " << ms_v1_total << " ms (requêtes uniquement, pas de prétraitement)\n";
    out << "  itineraries_v2 : prétraitement " << ms_pre_v2 << " ms + requêtes " << ms_v2_queries_total << " ms = total " << ms_v2_total << " ms\n";
    out << "  itineraries_v3 : prétraitement " << ms_pre_v3 << " ms + requêtes " << ms_v3_queries_total << " ms = total " << ms_v3_total << " ms\n";
    out << "  Résultats identiques : " << (ok ? "oui" : "non") << "\n";
}
