#ifndef SIMPLE_GRAPH_H
#define SIMPLE_GRAPH_H
#include "MutableGraph.h"
#include <map>
#include <tuple>
// Ordered maps keep IDs stable after deletion and visits deterministic.
class CSimpleGraph : public CFullMutableGraph {
protected:
    struct SVertex { TVertexTag Tag; std::map<TVertexID,double> Edges; };
    std::map<TVertexID,SVertex> Vertices;
    TVertexID NextID = 0;
public:
    using TEdge = std::tuple<TVertexID,TVertexID,double,bool>;
    CSimpleGraph(const std::vector<TVertexTag>& tags = {}, const std::vector<TEdge>& edges = {});
    std::size_t VertexCount() const noexcept override;
    bool GetVertexTag(TVertexID,TVertexTag&) const noexcept override;
    bool GetVertexNeighbors(TVertexID,std::vector<TVertexID>&) const noexcept override;
    bool EdgeExists(TVertexID,TVertexID) const noexcept override;
    bool GetEdgeWeight(TVertexID,TVertexID,double&) const noexcept override;
    TVertexID AddVertex(TVertexTag) noexcept override;
    bool RemoveVertex(TVertexID) noexcept override;
    bool SetVertexTag(TVertexID,TVertexTag) noexcept override;
    bool AddEdge(TVertexID,TVertexID,double = EdgeWeightInvalid,bool = true) noexcept override;
    bool RemoveEdge(TVertexID,TVertexID) noexcept override;
    bool SetEdgeWeight(TVertexID,TVertexID,double) noexcept override;
    void BreadthFirstSearchUnweighted(TVertexID,const TTraversalFunction&) const noexcept override;
    void DepthFirstSearch(TVertexID,const TTraversalFunction&) const noexcept override;
    bool BreadthFirstSearchWeighted(TVertexID,const TTraversalFunction&) const noexcept override;
    bool ForEach(const TTraversalFunction&) const noexcept override;
};
#endif
