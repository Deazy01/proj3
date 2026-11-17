#include "GridGraph.h"
#include "SimpleGridGraph.h"
#include <memory>


template <typename T>
struct SGridGraphFactory;

template<>
struct SGridGraphFactory<CSimpleGridGraph > {
    static std::unique_ptr<CGraph> Create(
        TGridCoordinate width,
        TGridCoordinate height,
        const SGridConnectionPattern& pattern = {}){
        return std::make_unique<CSimpleGridGraph>(width,height,pattern);
    }
};
