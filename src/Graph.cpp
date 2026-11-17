#include "SimpleGraph.h"
#include <cmath>
#include <deque>
#include <queue>
#include <set>
CSimpleGraph::CSimpleGraph(const std::vector<TVertexTag> &tags, const std::vector<TEdge> &edges) {
    for (auto tag : tags)
        AddVertex(tag);
    for (auto [s, d, w, b] : edges)
        AddEdge(s, d, w, b);
}
std::size_t CSimpleGraph::VertexCount() const noexcept {
    return Vertices.size();
}
bool CSimpleGraph::GetVertexTag(TVertexID id, TVertexTag &tag) const noexcept {
    auto it = Vertices.find(id);
    if (it == Vertices.end())
        return false;
    tag = it->second.Tag;
    return true;
}
bool CSimpleGraph::GetVertexNeighbors(TVertexID id, std::vector<TVertexID> &out) const noexcept {
    auto it = Vertices.find(id);
    if (it == Vertices.end())
        return false;
    out.clear();
    for (auto [dest, w] : it->second.Edges)
        out.push_back(dest);
    return true;
}
bool CSimpleGraph::EdgeExists(TVertexID s, TVertexID d) const noexcept {
    return Vertices.contains(s) && Vertices.at(s).Edges.contains(d);
}
bool CSimpleGraph::GetEdgeWeight(TVertexID s, TVertexID d, double &w) const noexcept {
    if (!EdgeExists(s, d))
        return false;
    w = Vertices.at(s).Edges.at(d);
    return true;
}
// A queue gives BFS; a stack gives DFS. Parent and depth travel with each item.
void CSimpleGraph::BreadthFirstSearchUnweighted(TVertexID start,
                                                const TTraversalFunction &cb) const noexcept {
    if (!Vertices.contains(start) || !cb)
        return;
    std::deque<std::tuple<TVertexID, TVertexID, std::size_t>> queue{{start, VertexIDInvalid, 0}};
    std::set<TVertexID> seen{start};
    while (!queue.empty()) {
        auto [id, parent, depth] = queue.front();
        queue.pop_front();
        auto action = cb(id, parent, depth, EdgeWeightInvalid);
        if (action == EGraphTraversal::Stop)
            return;
        if (action == EGraphTraversal::SkipNeighbors)
            continue;
        std::vector<TVertexID> neighbors;
        GetVertexNeighbors(id, neighbors);
        for (auto n : neighbors)
            if (seen.insert(n).second)
                queue.emplace_back(n, id, depth + 1);
    }
}
void CSimpleGraph::DepthFirstSearch(TVertexID start, const TTraversalFunction &cb) const noexcept {
    if (!Vertices.contains(start) || !cb)
        return;
    std::vector<std::tuple<TVertexID, TVertexID, std::size_t>> stack{{start, VertexIDInvalid, 0}};
    std::set<TVertexID> seen;
    while (!stack.empty()) {
        auto [id, parent, depth] = stack.back();
        stack.pop_back();
        if (!seen.insert(id).second)
            continue;
        auto action = cb(id, parent, depth, EdgeWeightInvalid);
        if (action == EGraphTraversal::Stop)
            return;
        if (action == EGraphTraversal::SkipNeighbors)
            continue;
        std::vector<TVertexID> neighbors;
        GetVertexNeighbors(id, neighbors);
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it)
            stack.emplace_back(*it, id, depth + 1);
    }
}
// Dijkstra: always finalize the vertex with the smallest known distance.
bool CSimpleGraph::BreadthFirstSearchWeighted(TVertexID start,
                                              const TTraversalFunction &cb) const noexcept {
    if (!Vertices.contains(start) || !cb)
        return false;
    for (const auto &[id, v] : Vertices) {
        std::vector<TVertexID> neighbors;
        GetVertexNeighbors(id, neighbors);
        for (auto n : neighbors) {
            double w;
            GetEdgeWeight(id, n, w);
            if (!std::isfinite(w) || w < 0)
                return false;
        }
    }
    using TItem = std::tuple<double, TVertexID, TVertexID, std::size_t>;
    std::priority_queue<TItem, std::vector<TItem>, std::greater<TItem>> queue;
    std::map<TVertexID, double> best{{start, 0}};
    std::set<TVertexID> done;
    queue.emplace(0, start, VertexIDInvalid, 0);
    while (!queue.empty()) {
        auto [distance, id, parent, depth] = queue.top();
        queue.pop();
        if (!done.insert(id).second)
            continue;
        auto action = cb(id, parent, depth, distance);
        if (action == EGraphTraversal::Stop)
            return false;
        if (action == EGraphTraversal::SkipNeighbors)
            continue;
        std::vector<TVertexID> neighbors;
        GetVertexNeighbors(id, neighbors);
        for (auto n : neighbors) {
            double w;
            GetEdgeWeight(id, n, w);
            double next = distance + w;
            if (!best.contains(n) || next < best[n]) {
                best[n] = next;
                queue.emplace(next, n, id, depth + 1);
            }
        }
    }
    return true;
}
bool CSimpleGraph::ForEach(const TTraversalFunction &cb) const noexcept {
    if (!cb)
        return false;
    for (const auto &[id, v] : Vertices)
        if (cb(id, VertexIDInvalid, 0, EdgeWeightInvalid) == EGraphTraversal::Stop)
            return false;
    return true;
}
