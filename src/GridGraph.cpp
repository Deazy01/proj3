#include "SimpleGridGraph.h"
#include <cmath>
#include <stdexcept>
namespace {
constexpr int DX[8] = {0, 1, 1, 1, 0, -1, -1, -1};
constexpr int DY[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
bool Step(std::size_t x, std::size_t y, int d, std::size_t w, std::size_t h, std::size_t &nx,
          std::size_t &ny) {
    if ((DX[d] < 0 && x == 0) || (DY[d] < 0 && y == 0))
        return false;
    nx = DX[d] < 0 ? x - 1 : x + DX[d];
    ny = DY[d] < 0 ? y - 1 : y + DY[d];
    return nx < w && ny < h;
}
} // namespace
CSimpleGridGraph::CSimpleGridGraph(TGridCoordinate w, TGridCoordinate h,
                                   const SGridConnectionPattern &p)
    : DWidth(w), DHeight(h), Pattern(p) {
    if (h && w > VertexIDInvalid / h)
        throw std::length_error("Grid dimensions overflow");
    Overrides.resize(w * h);
    for (std::size_t i = 0; i < w * h; ++i)
        CSimpleGraph::AddVertex(0);
}
SGridConnectionPattern CSimpleGridGraph::GetGridConnectionPattern() const noexcept {
    return Pattern;
}
CGraph::TVertexID CSimpleGridGraph::GetVertex(const SGridLocation &l) const noexcept {
    return GetVertex(l.DX, l.DY);
}
CGraph::TVertexID CSimpleGridGraph::GetVertex(TGridCoordinate x, TGridCoordinate y) const noexcept {
    return x < DWidth && y < DHeight ? y * DWidth + x : VertexIDInvalid;
}
bool CSimpleGridGraph::GetVertexLocation(TVertexID id, SGridLocation &l) const noexcept {
    if (id >= Overrides.size())
        return false;
    l = {id % DWidth, id / DWidth};
    return true;
}
TGridCoordinate CSimpleGridGraph::Width() const noexcept {
    return DWidth;
}
TGridCoordinate CSimpleGridGraph::Height() const noexcept {
    return DHeight;
}
TGridVertexConnectionMask CSimpleGridGraph::GetVertexConnectionMask(TVertexID id) const noexcept {
    SGridLocation l;
    if (!GetVertexLocation(id, l))
        return 0;
    auto mask = Overrides[id].value_or(
        l.DY % 2 ? (l.DX % 2 ? Pattern.DOddRowOddColumn : Pattern.DOddRowEvenColumn)
                 : (l.DX % 2 ? Pattern.DEvenRowOddColumn : Pattern.DEvenRowEvenColumn));
    for (int d = 0; d < 8; ++d) {
        std::size_t x, y;
        if (!Step(l.DX, l.DY, d, DWidth, DHeight, x, y))
            mask &= ~(1u << d);
    }
    return mask;
}
bool CSimpleGridGraph::SetGridConnectionPattern(const SGridConnectionPattern &p) noexcept {
    Pattern = p;
    return true;
}
bool CSimpleGridGraph::SetVertexConnectionMask(TVertexID id, TGridVertexConnectionMask m) noexcept {
    if (id >= Overrides.size())
        return false;
    Overrides[id] = m;
    return true;
}
bool CSimpleGridGraph::ClearVertexConnectionMask(TVertexID id) noexcept {
    if (id >= Overrides.size())
        return false;
    Overrides[id].reset();
    return true;
}
bool CSimpleGridGraph::GetVertexNeighbors(TVertexID id,
                                          std::vector<TVertexID> &out) const noexcept {
    SGridLocation l;
    if (!GetVertexLocation(id, l))
        return false;
    out.clear();
    auto mask = GetVertexConnectionMask(id);
    for (int d = 0; d < 8; ++d)
        if (mask & (1u << d)) {
            std::size_t x, y;
            Step(l.DX, l.DY, d, DWidth, DHeight, x, y);
            out.push_back(GetVertex(x, y));
        }
    return true;
}
bool CSimpleGridGraph::EdgeExists(TVertexID s, TVertexID t) const noexcept {
    SGridLocation a, b;
    if (!GetVertexLocation(s, a) || !GetVertexLocation(t, b))
        return false;
    auto mask = GetVertexConnectionMask(s);
    for (int d = 0; d < 8; ++d)
        if (mask & (1u << d)) {
            std::size_t x, y;
            Step(a.DX, a.DY, d, DWidth, DHeight, x, y);
            if (x == b.DX && y == b.DY)
                return true;
        }
    return false;
}
bool CSimpleGridGraph::GetEdgeWeight(TVertexID s, TVertexID t, double &w) const noexcept {
    if (!EdgeExists(s, t))
        return false;
    w = (s % DWidth != t % DWidth && s / DWidth != t / DWidth) ? std::sqrt(2.0) : 1.0;
    return true;
}
