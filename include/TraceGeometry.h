#ifndef TRACE_GEOMETRY_H
#define TRACE_GEOMETRY_H
#include "CircuitTrace.h"
#include <algorithm>
namespace TraceGeometry {
inline std::size_t Difference(std::size_t a, std::size_t b) {
    return a > b ? a - b : b - a;
}
inline int Direction(std::size_t a, std::size_t b) {
    return (b > a) - (b < a);
}
inline bool Valid(const SCircuitTraceEdge &e) {
    auto x = Difference(e.DSource.DX, e.DDestination.DX),
         y = Difference(e.DSource.DY, e.DDestination.DY);
    return (x || y) && (!x || !y || x == y);
}
inline auto Direction(const SCircuitTraceEdge &e) {
    return std::pair{Direction(e.DSource.DX, e.DDestination.DX),
                     Direction(e.DSource.DY, e.DDestination.DY)};
}
// Move diagonally first, then horizontally or vertically to reach the target.
inline std::vector<SCircuitTraceEdge> Bridge(SCircuitLocation a, SCircuitLocation b) {
    std::vector<SCircuitTraceEdge> result;
    auto steps = std::min(Difference(a.DX, b.DX), Difference(a.DY, b.DY));
    auto middle = a;
    if (steps) {
        middle.DX = b.DX > a.DX ? a.DX + steps : a.DX - steps;
        middle.DY = b.DY > a.DY ? a.DY + steps : a.DY - steps;
        result.push_back({a, middle});
    }
    if (middle != b)
        result.push_back({middle, b});
    return result;
}
} // namespace TraceGeometry
#endif
