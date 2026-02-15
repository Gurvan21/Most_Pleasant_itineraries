#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <functional>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>

using Vertex = int;
using Weight = double;
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

    Vertex add_vertex();
    void remove_vertex(Vertex v);
    int num_vertices() const;
    bool is_alive(Vertex v) const;

    void add_edge(Vertex u, Vertex v, Weight w = 1);
    void delete_edge(Vertex u, Vertex v, std::optional<Weight> w = std::nullopt);
    const std::vector<std::pair<Vertex, Weight>>& neighbors(Vertex u) const;
    std::vector<Edge> get_edges() const;

    bool is_directed() const;
    int num_edges() const;

    std::vector<Vertex> dfs(Vertex start) const;
    std::vector<Vertex> bfs(Vertex start) const;

    Graph kruskal() const;
    Graph prim(Vertex start) const;

    std::optional<Weight> itineraries_v1(Vertex u, Vertex v) const;

    void compute_center_and_parent();
    bool has_center() const;
    Vertex get_center() const;
    int get_diameter_length() const;
    Vertex get_parent(Vertex v) const;
    std::optional<Vertex> lca(Vertex u, Vertex v) const;
    /** Tarjan LCA : O(n + |P|), réponses dans l'ordre des paires. */
    std::vector<std::optional<Vertex>> tarjan_lca(const std::vector<std::pair<Vertex, Vertex>>& queries) const;
    /** Max sur le chemin u → ancêtre a. Précondition : a ancêtre de u. O(log n). */
    std::optional<Weight> max_on_path_to_ancestor(Vertex u, Vertex a) const;
    std::optional<Weight> itineraries_v2(Vertex u, Vertex v) const;

    void preprocess_itineraries_v3(const std::vector<std::pair<Vertex, Vertex>>& queries);
    std::optional<Weight> itineraries_v3(Vertex u, Vertex v) const;

    void print_graph(std::ostream& out) const;
    void print_summary(std::ostream& out) const;
    void write_dot(std::ostream& out, const std::string& name = "G") const;
    void write_dot_file(const std::string& path, const std::string& name = "G") const;
    void write_html_file(const std::string& path, const std::string& title = "Graphe") const;

    friend std::ostream& operator<<(std::ostream& out, const Graph& graph);

private:
    std::vector<std::vector<std::pair<Vertex, Weight>>> cont;
    std::vector<char> alive;
    std::vector<int> free_vertices;
    bool directed;

    bool center_valid_ = false;
    Vertex centre_ = -1;
    std::vector<Vertex> parent_;
    std::vector<Weight> parent_edge_weight_;
    std::vector<int> depth_;
    std::vector<std::vector<Vertex>> up_;
    std::vector<std::vector<Weight>> max_up_;
    int diameter_length_ = -1;

    std::unordered_map<std::pair<Vertex, Vertex>, Weight, PairHash> max_path_table_;

    void build_binary_lifting(int n);
};

#endif