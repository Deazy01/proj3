#include "Graph.h"
#include "SimpleGraph.h"
#include <memory>

template <typename T>
struct SGraphFactory;

using TGraphEdge = std::tuple<CGraph::TVertexID, CGraph::TVertexID, double, bool>;

template<>
struct SGraphFactory<CSimpleGraph> {
    static std::unique_ptr<CGraph> Create(const std::vector<CGraph::TVertexTag>& tags,
        const std::vector<TGraphEdge>& edges = {}){
        return std::make_unique<CSimpleGraph>(tags,edges);
    }
};
