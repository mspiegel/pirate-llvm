#ifndef GAPS_GR_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

template<typename N>
class Gr {

public:
    typedef N Node;

    std::unordered_map<Node, std::vector<Node>> edges;
    std::vector<Node> dfs_enumeration_util(std::unordered_set<Node> *seen, Node start) const;
    std::vector<std::vector<Node>> dfs_enumeration() const;
    std::vector<std::vector<Node>> scc_enumeration() const;
    Gr transpose() const;

    void set_node(Node, std::vector<Node> connections);
};

template<typename N>
void Gr<N>::set_node(Node node, std::vector<Node> targets)
{
    edges.emplace(node, targets);
}

template<typename N>
Gr<N> Gr<N>::transpose() const
{
    Gr<N> output;

    for (auto const& entry : edges) {
        output.edges[entry.first] = {};
    }

    for (auto const& entry : edges) {
        for (auto target : entry.second) {
            output.edges[target].push_back(entry.first);
        }
    }

    return output;
}


template <typename N>
std::vector<typename Gr<N>::Node>
Gr<N>::dfs_enumeration_util(std::unordered_set<Node> *seen, Node start) const {
    std::vector<Node> local;
    std::vector<Node> work { start };

    while (!work.empty()) {
        auto current = work.back();
        work.pop_back();
            
        if (!seen->insert(current).second) {
                continue;
        }

        local.push_back(current);

        auto const& next = edges.find(current)->second;
        work.insert(work.end(), next.begin(), next.end());
    }

    std::reverse(local.begin(), local.end());

    return local;
}

template<typename N>
std::vector<std::vector<typename Gr<N>::Node>>
Gr<N>::dfs_enumeration() const {

    std::unordered_set<Node> seen;
    std::vector<std::vector<Node>> output;

    for (auto const& entry : edges) {
        output.emplace_back(dfs_enumeration_util(&seen, entry.first));
    }

    return output;
}

template <typename N>
std::vector<std::vector<typename Gr<N>::Node>>
Gr<N>::scc_enumeration() const
{
    std::vector<Node> dfs_flat;
    for (auto const& tree : dfs_enumeration()) {
        dfs_flat.insert(dfs_flat.end(), tree.begin(), tree.end());
    }

    auto g_ = transpose();
    std::vector<std::vector<Node>> output;
    std::unordered_set<Node> seen;

    while (!dfs_flat.empty()) {
        auto current = dfs_flat.back();
        dfs_flat.pop_back();

        output.push_back(g_.dfs_enumeration_util(&seen, current));
    }

    return output;
}

template<typename N>
Gr<unsigned long>
scc_graph(
    Gr<N> const& g,
    std::vector<std::vector<typename Gr<N>::Node>> components)
{
    Gr<unsigned long> output;
    auto n = components.size();

    std::unordered_map<typename Gr<N>::Node, unsigned long> component_map;

    for (unsigned long i = 0; i < n; i++) {
        for (auto x : components[i]) {
            component_map[x] = i;
        }
    }

    for (decltype(n) i = 0; i < n; i++) {
        auto& edges = output.edges[i];
        for (auto member : components[i]) {
            for (auto outgoing : g.edges.find(member)->second) {
                auto target = component_map[outgoing];
                if (target != i) edges.push_back(target);                
            }
        }
        std::sort(edges.begin(), edges.end());
        auto newend = std::unique(edges.begin(), edges.end());
        edges.erase(newend, edges.end());
    }

    return output;
}

#endif
