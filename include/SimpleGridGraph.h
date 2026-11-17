#ifndef SIMPLE_GRID_GRAPH_H
#define SIMPLE_GRID_GRAPH_H
#include "SimpleGraph.h"
#include "GridGraph.h"
#include <optional>
// Reuses graph traversal, but computes grid neighbors from masks on demand.
class CSimpleGridGraph : public CSimpleGraph, public CMutableGridGraph {
    TGridCoordinate DWidth,DHeight;
    SGridConnectionPattern Pattern;
    std::vector<std::optional<TGridVertexConnectionMask>> Overrides;
public:
    CSimpleGridGraph(TGridCoordinate,TGridCoordinate,const SGridConnectionPattern& = {});
    SGridConnectionPattern GetGridConnectionPattern() const noexcept override;
    TVertexID GetVertex(const SGridLocation&) const noexcept override;
    TVertexID GetVertex(TGridCoordinate,TGridCoordinate) const noexcept override;
    TGridVertexConnectionMask GetVertexConnectionMask(TVertexID) const noexcept override;
    bool GetVertexLocation(TVertexID,SGridLocation&) const noexcept override;
    TGridCoordinate Width() const noexcept override;
    TGridCoordinate Height() const noexcept override;
    bool SetGridConnectionPattern(const SGridConnectionPattern&) noexcept override;
    bool SetVertexConnectionMask(TVertexID,TGridVertexConnectionMask) noexcept override;
    bool ClearVertexConnectionMask(TVertexID) noexcept override;
    bool GetVertexNeighbors(TVertexID,std::vector<TVertexID>&) const noexcept override;
    bool EdgeExists(TVertexID,TVertexID) const noexcept override;
    bool GetEdgeWeight(TVertexID,TVertexID,double&) const noexcept override;
    // Grid topology has fixed vertices and mask-defined edges.
    TVertexID AddVertex(TVertexTag) noexcept override { return VertexIDInvalid; }
    bool RemoveVertex(TVertexID) noexcept override { return false; }
    bool AddEdge(TVertexID,TVertexID,double=EdgeWeightInvalid,bool=true) noexcept override { return false; }
    bool RemoveEdge(TVertexID,TVertexID) noexcept override { return false; }
    bool SetEdgeWeight(TVertexID,TVertexID,double) noexcept override { return false; }
};
#endif
