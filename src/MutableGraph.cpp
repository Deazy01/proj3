#include "SimpleGraph.h"
CGraph::TVertexID CSimpleGraph::AddVertex(TVertexTag tag) noexcept {
    if (NextID == VertexIDInvalid)
        return VertexIDInvalid;
    auto id = NextID++;
    Vertices.emplace(id, SVertex{tag, {}});
    return id;
}
bool CSimpleGraph::RemoveVertex(TVertexID id) noexcept {
    if (!Vertices.erase(id))
        return false;
    for (auto &[other, v] : Vertices)
        v.Edges.erase(id);
    return true;
}
bool CSimpleGraph::SetVertexTag(TVertexID id, TVertexTag tag) noexcept {
    if (!Vertices.contains(id))
        return false;
    Vertices.at(id).Tag = tag;
    return true;
}
bool CSimpleGraph::AddEdge(TVertexID s, TVertexID d, double w, bool bidir) noexcept {
    if (!Vertices.contains(s) || !Vertices.contains(d))
        return false;
    Vertices.at(s).Edges[d] = w;
    if (bidir)
        Vertices.at(d).Edges[s] = w;
    return true;
}
bool CSimpleGraph::RemoveEdge(TVertexID s, TVertexID d) noexcept {
    return Vertices.contains(s) && Vertices.at(s).Edges.erase(d);
}
bool CSimpleGraph::SetEdgeWeight(TVertexID s, TVertexID d, double w) noexcept {
    if (!EdgeExists(s, d))
        return false;
    Vertices.at(s).Edges[d] = w;
    return true;
}
