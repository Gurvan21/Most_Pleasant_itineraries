#ifndef ITINERARIESTEST_H_INCLUDED
#define ITINERARIESTEST_H_INCLUDED

#include "Graph.h"
#include <chrono>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

struct ItinerariesRuntimes {
    double v1_ms = 0;
    double v2_total_ms = 0;
    double v3_total_ms = 0;
    bool timeout = false;
    bool results_identical = false;
};

class ItinerariesTest
{
public:
    ItinerariesTest() = default;
    ItinerariesTest(Graph tree, std::vector<std::pair<Vertex, Vertex>> queries);

    /** Format : n m, arêtes u v c (1-indexés), Q, paires de requêtes. */
    static std::optional<ItinerariesTest> load_from_file(const std::string& path);

    const Graph& graph() const { return tree_; }
    const std::vector<std::pair<Vertex, Vertex>>& queries() const { return queries_; }

    void run_and_compare_times(std::ostream& out = std::cout,
                               const std::optional<std::string>& answers_path = std::nullopt,
                               ItinerariesRuntimes* runtimes_out = nullptr) const;

private:
    Graph tree_;
    std::vector<std::pair<Vertex, Vertex>> queries_;
};

#endif
