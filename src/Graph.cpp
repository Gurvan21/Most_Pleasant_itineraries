#include "Graph.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <sstream>
#include <utility>
#include <vector>

void Graph::add_edge(Vertex u, Vertex v, Weight w) {
    assert(0 <= u && u < num_vertices() && 0 <= v && v < num_vertices());
    assert(alive[u] && alive[v]);
    cont[u].push_back({v, w});
    if (!directed) cont[v].push_back({u, w});
    center_valid_ = false;
}

void Graph::print_graph(std::ostream& out) const {
    for (Vertex u = 0; u < num_vertices(); ++u) {
        if (!alive[u]) continue;
        out << u << " → ";
        bool first = true;
        for (const auto& [v, w] : cont[u]) {
            if (!alive[v]) continue;
            if (!first) out << ", ";
            out << "(" << v << ", " << w << ")";
            first = false;
        }
        out << '\n';
    }
}

void Graph::print_summary(std::ostream& out) const {
    int n = 0;
    for (Vertex v = 0; v < num_vertices(); ++v)
        if (alive[v]) ++n;
    out << "sommets (vivants): " << n
        << " | arêtes: " << num_edges()
        << " | " << (directed ? "orienté" : "non orienté") << '\n';
}

void Graph::write_dot(std::ostream& out, const std::string& name) const {
    out << (directed ? "digraph" : "graph") << " " << name << " {\n";
    for (Vertex u = 0; u < num_vertices(); ++u)
        if (alive[u])
            out << "  " << u << ";\n";
    const char* edge_op = directed ? " -> " : " -- ";
    for (Vertex u = 0; u < num_vertices(); ++u) {
        if (!alive[u]) continue;
        for (const auto& [v, w] : cont[u]) {
            if (!alive[v]) continue;
            if (directed || u <= v) {
                out << "  " << u << edge_op << v << " [label=\"" << w << "\"];\n";
            }
        }
    }
    out << "}\n";
}

void Graph::write_dot_file(const std::string& path, const std::string& name) const {
    std::ofstream f(path);
    if (f) write_dot(f, name);
}
/* 
void Graph::write_html_file(const std::string& path, const std::string& title) const {
    std::vector<Vertex> nodes;
    for (Vertex v = 0; v < num_vertices(); ++v)
        if (alive[v]) nodes.push_back(v);
    if (nodes.empty()) return;

    const double cx = 250, cy = 250, r = 200;
    const int n = static_cast<int>(nodes.size());
    auto px = [&](int i) {
        return cx + r * std::cos(2 * 3.14159265359 * i / n - 3.14159265359 / 2);
    };
    auto py = [&](int i) {
        return cy + r * std::sin(2 * 3.14159265359 * i / n - 3.14159265359 / 2);
    };

    std::ostringstream svg;
    svg << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 500 500' width='500' height='500'>\n";
    for (const auto& e : get_edges()) {
        Vertex u = std::get<0>(e), v = std::get<1>(e);
        Weight w = std::get<2>(e);
        int iu = static_cast<int>(std::find(nodes.begin(), nodes.end(), u) - nodes.begin());
        int iv = static_cast<int>(std::find(nodes.begin(), nodes.end(), v) - nodes.begin());
        double x1 = px(iu), y1 = py(iu), x2 = px(iv), y2 = py(iv);
        double mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
        svg << "  <line x1='" << x1 << "' y1='" << y1 << "' x2='" << x2 << "' y2='" << y2
            << "' stroke='#369' stroke-width='2'/>\n";
        svg << "  <text x='" << mx << "' y='" << my << "' fill='#333' font-size='10' text-anchor='middle'>" << w << "</text>\n";
    }
    for (int i = 0; i < n; ++i) {
        double x = px(i), y = py(i);
        svg << "  <circle cx='" << x << "' cy='" << y << "' r='18' fill='#6ab' stroke='#248' stroke-width='2'/>\n";
        svg << "  <text x='" << x << "' y='" << y << "' fill='white' font-size='12' text-anchor='middle' dominant-baseline='central'>" << nodes[i] << "</text>\n";
    }
    svg << "</svg>\n";

    std::ofstream f(path);
    if (!f) return;
    f << "<!DOCTYPE html><html><head><meta charset='utf-8'><title>" << title << "</title></head><body>";
    f << "<h1>" << title << "</h1>";
    f << "<p>Sommets: " << n << " — Arêtes: " << num_edges() << " — " << (directed ? "Orienté" : "Non orienté") << "</p>";
    f << svg.str();
    f << "</body></html>\n";
}*/
void Graph::remove_vertex(Vertex v) {
    assert(0 <= v && v < num_vertices() && alive[v]);
    cont[v].clear();
    alive[v] = 0;
    free_vertices.push_back(v);
    center_valid_ = false;
}

int Graph::num_vertices() const { return static_cast<int>(cont.size()); }

bool Graph::is_alive(Vertex v) const {
    return v >= 0 && v < num_vertices() && alive[v] != 0;
}

const std::vector<std::pair<Vertex, Weight>>& Graph::neighbors(Vertex u) const {
    assert(0 <= u && u < num_vertices());
    return cont[u];
}

std::vector<Edge> Graph::get_edges() const {
    std::vector<Edge> out;
    for (Vertex u = 0; u < num_vertices(); ++u) {
        if (!alive[u]) continue;
        for (const auto& [v, w] : cont[u]) {
            if (!alive[v]) continue;
            if (directed || u <= v)
                out.emplace_back(u, v, w);
        }
    }
    return out;
}

int Graph::num_edges() const {
    int n = 0;
    for (Vertex u = 0; u < num_vertices(); ++u) {
        if (!alive[u]) continue;
        for (const auto& p : cont[u])
            if (alive[p.first]) ++n;
    }
    if (!directed) n /= 2;
    return n;
}

bool Graph::is_directed() const { return directed; }

std::vector<Vertex> Graph::dfs(Vertex start) const {
    assert(0 <= start && start < num_vertices() && is_alive(start));
    std::vector<Vertex> order;
    std::vector<char> visited(static_cast<size_t>(num_vertices()), 0);
    std::stack<Vertex> st;
    st.push(start);
    visited[static_cast<size_t>(start)] = 1;
    while (!st.empty()) {
        Vertex u = st.top();
        st.pop();
        order.push_back(u);
        for (const auto& [v, w] : neighbors(u)) {
            (void)w;
            if (!is_alive(v) || visited[static_cast<size_t>(v)]) continue;
            visited[static_cast<size_t>(v)] = 1;
            st.push(v);
        }
    }
    return order;
}

std::vector<Vertex> Graph::bfs(Vertex start) const {
    assert(0 <= start && start < num_vertices() && is_alive(start));
    std::vector<Vertex> order;
    std::vector<char> visited(static_cast<size_t>(num_vertices()), 0);
    std::queue<Vertex> q;
    q.push(start);
    visited[static_cast<size_t>(start)] = 1;
    while (!q.empty()) {
        Vertex u = q.front();
        q.pop();
        order.push_back(u);
        for (const auto& [v, w] : neighbors(u)) {
            (void)w;
            if (!is_alive(v) || visited[static_cast<size_t>(v)]) continue;
            visited[static_cast<size_t>(v)] = 1;
            q.push(v);
        }
    }
    return order;
}

namespace {
struct UnionFind {
    std::vector<int> parent, rank;
    explicit UnionFind(int n) : parent(n), rank(n, 0) {
        for (int i = 0; i < n; ++i) parent[static_cast<size_t>(i)] = i;
    }
    int find(int x) {
        if (parent[static_cast<size_t>(x)] != x)
            parent[static_cast<size_t>(x)] = find(parent[static_cast<size_t>(x)]);
        return parent[static_cast<size_t>(x)];
    }
    void unite(int x, int y) {
        x = find(x), y = find(y);
        if (x == y) return;
        if (rank[static_cast<size_t>(x)] < rank[static_cast<size_t>(y)]) std::swap(x, y);
        parent[static_cast<size_t>(y)] = x;
        if (rank[static_cast<size_t>(x)] == rank[static_cast<size_t>(y)]) ++rank[static_cast<size_t>(x)];
    }
};
}  // namespace

static Graph build_mst_graph(int n_vertices, const std::vector<Edge>& mst_edges) {
    Graph mst;
    for (int i = 0; i < n_vertices; ++i) mst.add_vertex();
    for (const Edge& e : mst_edges) {
        Vertex u = std::get<0>(e), v = std::get<1>(e);
        Weight w = std::get<2>(e);
        mst.add_edge(u, v, w);
    }
    return mst;
}

Graph Graph::kruskal() const {
    assert(!directed && "Kruskal exige un graphe non orienté");
    std::vector<Edge> edges = get_edges();
    std::sort(edges.begin(), edges.end(),
              [](const Edge& a, const Edge& b) { return std::get<2>(a) < std::get<2>(b); });
    UnionFind uf(num_vertices());
    std::vector<Edge> mst;
    for (const Edge& e : edges) {
        Vertex u = std::get<0>(e), v = std::get<1>(e);
        if (!is_alive(u) || !is_alive(v)) continue;
        if (uf.find(u) != uf.find(v)) {
            uf.unite(u, v);
            mst.push_back(e);
        }
    }
    return build_mst_graph(num_vertices(), mst);
}

Graph Graph::prim(Vertex start) const {
    assert(!directed && "Prim exige un graphe non orienté");
    assert(0 <= start && start < num_vertices() && is_alive(start));
    using E = std::tuple<Weight, Vertex, Vertex>;
    std::priority_queue<E, std::vector<E>, std::greater<E>> pq;
    std::vector<char> in_mst(static_cast<size_t>(num_vertices()), 0);
    std::vector<Edge> mst;
    in_mst[static_cast<size_t>(start)] = 1;
    for (const auto& [v, w] : neighbors(start)) {
        if (is_alive(v)) pq.emplace(w, start, v);
    }
    while (!pq.empty()) {
        auto [w, from, to] = pq.top();
        pq.pop();
        if (in_mst[static_cast<size_t>(to)]) continue;
        in_mst[static_cast<size_t>(to)] = 1;
        mst.emplace_back(from, to, w);
        for (const auto& [v, vw] : neighbors(to)) {
            if (is_alive(v) && !in_mst[static_cast<size_t>(v)]) pq.emplace(vw, to, v);
        }
    }
    return build_mst_graph(num_vertices(), mst);
}

namespace {
// Helper récursif : max des poids sur le chemin de current à target, en ne repassant pas par from.
std::optional<Weight> max_on_path_dfs(const Graph& g, Vertex current, Vertex target,
                                      Vertex from, Weight path_max) {
    if (current == target) return path_max;
    for (const auto& [neighbor, w] : g.neighbors(current)) {
        if (!g.is_alive(neighbor) || neighbor == from) continue;
        Weight new_max = (path_max > w ? path_max : w);
        auto res = max_on_path_dfs(g, neighbor, target, current, new_max);
        if (res) return res;
    }
    return std::nullopt;
}
}  // namespace

std::optional<Weight> Graph::max_on_path(Vertex u, Vertex v) const {
    if (u == v) return 0;  // chemin vide
    if (!is_alive(u) || !is_alive(v)) return std::nullopt;
    // -1 n'est pas un sommet valide : pas de "from" au premier appel
    const Vertex no_from = -1;
    return max_on_path_dfs(*this, u, v, no_from, std::numeric_limits<Weight>::lowest());
}

// --- Centre, parent, LCA (arbre) ---
namespace {
// BFS depuis start ; retourne (sommet le plus éloigné, chemin start -> farthest).
std::pair<Vertex, std::vector<Vertex>> farthest_and_path(const Graph& g, Vertex start) {
    const int n = g.num_vertices();
    std::vector<int> dist(static_cast<size_t>(n), -1);
    std::vector<Vertex> parent_bfs(static_cast<size_t>(n), -1);
    std::queue<Vertex> q;
    q.push(start);
    dist[static_cast<size_t>(start)] = 0;
    while (!q.empty()) {
        Vertex u = q.front();
        q.pop();
        for (const auto& [v, w] : g.neighbors(u)) {
            (void)w;
            if (!g.is_alive(v) || dist[static_cast<size_t>(v)] >= 0) continue;
            dist[static_cast<size_t>(v)] = dist[static_cast<size_t>(u)] + 1;
            parent_bfs[static_cast<size_t>(v)] = u;
            q.push(v);
        }
    }
    Vertex farthest = start;
    for (Vertex v = 0; v < n; ++v)
        if (g.is_alive(v) && dist[static_cast<size_t>(v)] >= 0 &&
            dist[static_cast<size_t>(v)] > dist[static_cast<size_t>(farthest)])
            farthest = v;
    std::vector<Vertex> path;
    for (Vertex v = farthest; v != -1; v = parent_bfs[static_cast<size_t>(v)])
        path.push_back(v);
    std::reverse(path.begin(), path.end());
    return {farthest, path};
}

void dfs_fill_parent(const Graph& g, Vertex current, Vertex from,
                     std::vector<Vertex>& parent,
                     std::vector<Weight>& parent_edge_weight) {
    for (const auto& [v, w] : g.neighbors(current)) {
        if (!g.is_alive(v) || v == from) continue;
        parent[static_cast<size_t>(v)] = current;
        parent_edge_weight[static_cast<size_t>(v)] = w;
        dfs_fill_parent(g, v, current, parent, parent_edge_weight);
    }
}
}  // namespace

void Graph::compute_center_and_parent() {
    assert(!directed && "Centre/parent pour graphe non orienté (arbre)");
    const int n = num_vertices();
    Vertex start = -1;
    for (Vertex v = 0; v < n; ++v)
        if (is_alive(v)) { start = v; break; }
    if (start == -1) {
        center_valid_ = false;
        return;
    }
    auto [u, path1] = farthest_and_path(*this, start);
    auto [v, path_diam] = farthest_and_path(*this, u);
    const int L = static_cast<int>(path_diam.size()) - 1;  // nombre d'arêtes
    if (L <= 0) {
        centre_ = path_diam.empty() ? start : path_diam[0];
        diameter_length_ = L;
        parent_.assign(static_cast<size_t>(n), -1);
        parent_[static_cast<size_t>(centre_)] = -1;
        parent_edge_weight_.resize(static_cast<size_t>(n), 0);
        dfs_fill_parent(*this, centre_, -1, parent_, parent_edge_weight_);
        build_binary_lifting(n);
        center_valid_ = true;
        return;
    }
    if (L % 2 == 0) {
        centre_ = path_diam[static_cast<size_t>(L / 2)];
    } else {
        centre_ = path_diam[static_cast<size_t>((L + 1) / 2)];
    }
    diameter_length_ = L;
    parent_.resize(static_cast<size_t>(n), -1);
    parent_[static_cast<size_t>(centre_)] = -1;
    parent_edge_weight_.resize(static_cast<size_t>(n), 0);
    dfs_fill_parent(*this, centre_, -1, parent_, parent_edge_weight_);
    build_binary_lifting(n);
    center_valid_ = true;
}

void Graph::build_binary_lifting(int n) {
    depth_.assign(static_cast<size_t>(n), -1);
    depth_[static_cast<size_t>(centre_)] = 0;
    std::queue<Vertex> q;
    q.push(centre_);
    while (!q.empty()) {
        Vertex u = q.front();
        q.pop();
        for (const auto& [v, w] : neighbors(u)) {
            (void)w;
            if (!is_alive(v) || parent_[static_cast<size_t>(v)] != u) continue;
            depth_[static_cast<size_t>(v)] = depth_[static_cast<size_t>(u)] + 1;
            q.push(v);
        }
    }
    int max_k = 0;
    while ((1 << max_k) < n) ++max_k;
    up_.assign(static_cast<size_t>(n), std::vector<Vertex>(static_cast<size_t>(max_k + 1), -1));
    max_up_.assign(static_cast<size_t>(n), std::vector<Weight>(static_cast<size_t>(max_k + 1), std::numeric_limits<Weight>::lowest()));
    for (Vertex v = 0; v < n; ++v) {
        if (!is_alive(v) || depth_[static_cast<size_t>(v)] < 0) continue;
        up_[static_cast<size_t>(v)][0] = parent_[static_cast<size_t>(v)];
        if (parent_[static_cast<size_t>(v)] >= 0)
            max_up_[static_cast<size_t>(v)][0] = parent_edge_weight_[static_cast<size_t>(v)];
    }
    for (int k = 1; k <= max_k; ++k) {
        for (Vertex v = 0; v < n; ++v) {
            if (!is_alive(v) || depth_[static_cast<size_t>(v)] < 0) continue;
            Vertex mid = up_[static_cast<size_t>(v)][static_cast<size_t>(k - 1)];
            if (mid < 0) {
                up_[static_cast<size_t>(v)][static_cast<size_t>(k)] = -1;
                continue;
            }
            up_[static_cast<size_t>(v)][static_cast<size_t>(k)] = up_[static_cast<size_t>(mid)][static_cast<size_t>(k - 1)];
            Weight w1 = max_up_[static_cast<size_t>(v)][static_cast<size_t>(k - 1)];
            Weight w2 = up_[static_cast<size_t>(mid)][static_cast<size_t>(k - 1)] >= 0
                ? max_up_[static_cast<size_t>(mid)][static_cast<size_t>(k - 1)]
                : std::numeric_limits<Weight>::lowest();
            max_up_[static_cast<size_t>(v)][static_cast<size_t>(k)] = (w1 > w2 ? w1 : w2);
        }
    }
}

bool Graph::has_center() const { return center_valid_; }

Vertex Graph::get_center() const {
    assert(center_valid_ && "Appeler compute_center_and_parent() d'abord");
    return centre_;
}

int Graph::get_diameter_length() const {
    assert(center_valid_ && "Appeler compute_center_and_parent() d'abord");
    return diameter_length_;
}

Vertex Graph::get_parent(Vertex v) const {
    assert(center_valid_ && "Appeler compute_center_and_parent() d'abord");
    assert(0 <= v && v < num_vertices());
    return parent_[static_cast<size_t>(v)];
}

std::optional<Vertex> Graph::lca(Vertex u, Vertex v) const {
    if (!center_valid_) return std::nullopt;
    if (!is_alive(u) || !is_alive(v)) return std::nullopt;
    std::set<Vertex> anc_u;
    for (Vertex w = u; w != -1; w = get_parent(w))
        anc_u.insert(w);
    for (Vertex w = v; w != -1; w = get_parent(w))
        if (anc_u.count(w)) return w;
    return std::nullopt;
}

std::optional<Weight> Graph::max_on_path_to_ancestor(Vertex u, Vertex a) const {
    if (!center_valid_ || !is_alive(u) || !is_alive(a)) return std::nullopt;
    if (u == a) return 0;
    const int du = depth_[static_cast<size_t>(u)];
    const int da = depth_[static_cast<size_t>(a)];
    int d = du - da;
    if (d <= 0) return std::nullopt;  // a n'est pas un ancêtre (ou u == a déjà traité)
    const int max_k = static_cast<int>(up_[static_cast<size_t>(u)].size()) - 1;
    Weight result = std::numeric_limits<Weight>::lowest();
    Vertex current = u;
    for (int k = max_k; k >= 0 && d > 0; --k) {
        if (d >= (1 << k)) {
            Weight w = max_up_[static_cast<size_t>(current)][static_cast<size_t>(k)];
            if (w > result) result = w;
            current = up_[static_cast<size_t>(current)][static_cast<size_t>(k)];
            d -= (1 << k);
        }
    }
    if (current != a) return std::nullopt;  // a n'est pas ancêtre de u
    return result;
}

Vertex Graph::add_vertex() {
    center_valid_ = false;
    if (!free_vertices.empty()) {
        int v = free_vertices.back();
        free_vertices.pop_back();
        alive[v] = 1;
        return v;
    } else {
        cont.emplace_back();
        alive.push_back(1);
        return cont.size() - 1;
    }
}

Graph::Graph() : cont(0), alive(), free_vertices(), directed(false) {}

Graph::Graph(std::vector<std::vector<std::pair<Vertex, Weight>>> adj, bool directed)
    : cont(std::move(adj)), alive(cont.size(), 1), free_vertices(), directed(directed),
      center_valid_(false), centre_(-1), parent_(), diameter_length_(-1) {}

void Graph::delete_edge(Vertex u, Vertex v, std::optional<Weight> w)
{
    assert(0 <= u && u < num_vertices() && 0 <= v && v < num_vertices());
    center_valid_ = false;
    auto match = [&](const std::pair<Vertex, Weight>& e) {
        if (e.first != v) return false;
        if (!w) return true;
        return std::fabs(e.second - *w) <= 1e-12;
    };

    auto& adj_u = cont[u];
    adj_u.erase(std::remove_if(adj_u.begin(), adj_u.end(), match), adj_u.end());

    if (!directed) {
        auto match_rev = [&](const std::pair<Vertex, Weight>& e) {
            if (e.first != u) return false;
            if (!w) return true;
            return std::fabs(e.second - *w) <= 1e-12;
        };

        auto& adj_v = cont[v];
        adj_v.erase(std::remove_if(adj_v.begin(), adj_v.end(), match_rev), adj_v.end());
    }
}

std::ostream& operator<<(std::ostream& out, const Graph& graph) {
    graph.print_graph(out);
    return out;
}
