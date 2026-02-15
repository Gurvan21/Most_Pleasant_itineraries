#ifndef ITINERARIESTEST_H_INCLUDED
#define ITINERARIESTEST_H_INCLUDED

#include "Graph.h"
#include <chrono>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

/** Résultats et runtimes après run_and_compare_times (pour rapport). */
struct ItinerariesRuntimes {
    double v1_ms = 0;
    double v2_total_ms = 0;
    double v3_total_ms = 0;
    bool timeout = false;
    bool results_identical = false;
};

/** Charge un graphe (arbre) et des requêtes depuis un fichier, exécute les 3 méthodes
 *  itineraries_v1, v2, v3 et compare les temps. */
class ItinerariesTest
{
public:
    ItinerariesTest() = default;
    /** Graphe (arbre) et liste des requêtes (paires de sommets, 0-indexés). */
    ItinerariesTest(Graph tree, std::vector<std::pair<Vertex, Vertex>> queries);

    /** Parse le fichier : ligne 1 = n m, lignes 2..m+1 = u v c (arête, indices 1..n), ligne m+2 = Q, lignes m+3.. = paires de requêtes. Retourne nullopt en cas d'erreur. */
    static std::optional<ItinerariesTest> load_from_file(const std::string& path);

    const Graph& graph() const { return tree_; }
    const std::vector<std::pair<Vertex, Vertex>>& queries() const { return queries_; }

    /** Exécute les 3 méthodes sur toutes les requêtes, vérifie que les résultats coïncident, affiche les temps (ms).
     *  Si answers_path est fourni, écrit une ligne par requête (entier = max sur le chemin, arrondi) pour itineraries.out.
     *  Si runtimes_out est fourni, y écrit v1_ms, v2_total_ms, v3_total_ms (pour parsing par script). */
    void run_and_compare_times(std::ostream& out = std::cout,
                               const std::optional<std::string>& answers_path = std::nullopt,
                               ItinerariesRuntimes* runtimes_out = nullptr) const;

private:
    Graph tree_;
    std::vector<std::pair<Vertex, Vertex>> queries_;
};

#endif
