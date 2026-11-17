#include "CircuitTrace.h"
#include "TraceGeometry.h"
#include <algorithm>
struct CCircuitTrace::SImplementation {
    TCircuitName Name;
    std::vector<SCircuitTraceEdge> Edges;
    std::vector<SCircuitLocation> Locations;
};
CCircuitTrace::CCircuitTrace() : DImplementation(std::make_unique<SImplementation>()) {}
CCircuitTrace::~CCircuitTrace() = default;
CCircuitTrace::CCircuitTrace(const CCircuitTrace &t) : CCircuitTrace() {
    *this = t;
}
CCircuitTrace::CCircuitTrace(CCircuitTrace &&t) noexcept = default;
CCircuitTrace &CCircuitTrace::operator=(const CCircuitTrace &t) {
    if (this != &t)
        DImplementation = std::make_unique<SImplementation>(*t.DImplementation);
    return *this;
}
CCircuitTrace &CCircuitTrace::operator=(CCircuitTrace &&t) noexcept = default;
TCircuitName CCircuitTrace::Name() const {
    return DImplementation->Name;
}
TCircuitName CCircuitTrace::Name(const TCircuitName &n) {
    if (!n.empty())
        DImplementation->Name = n;
    return Name();
}
std::size_t CCircuitTrace::LocationCount() const {
    return DImplementation->Locations.size();
}
std::size_t CCircuitTrace::EdgeCount() const {
    return DImplementation->Edges.size();
}
bool CCircuitTrace::Location(std::size_t i, SCircuitLocation &l) const {
    if (i >= LocationCount())
        return false;
    l = DImplementation->Locations[i];
    return true;
}
bool CCircuitTrace::Edge(std::size_t i, SCircuitTraceEdge &e) const {
    if (i >= EdgeCount())
        return false;
    e = DImplementation->Edges[i];
    return true;
}
bool CCircuitTrace::SourceDestinationAsEdge(SCircuitTraceEdge &e) const {
    if (!EdgeCount())
        return false;
    e = {DImplementation->Edges.front().DSource, DImplementation->Edges.back().DDestination};
    return true;
}
bool CCircuitTrace::AddEdge(const SCircuitTraceEdge &e) {
    if (!TraceGeometry::Valid(e))
        return false;
    for (auto old : DImplementation->Edges)
        if (old == e || old == SCircuitTraceEdge{e.DDestination, e.DSource})
            return false;
    DImplementation->Edges.push_back(e);
    for (auto l : {e.DSource, e.DDestination}) {
        auto &locations = DImplementation->Locations;
        if (std::find(locations.begin(), locations.end(), l) == locations.end())
            locations.push_back(l);
    }
    return true;
}
bool CCircuitTrace::IsBroken() const {
    for (std::size_t i = 1; i < EdgeCount(); ++i)
        if (DImplementation->Edges[i - 1].DDestination != DImplementation->Edges[i].DSource)
            return true;
    return false;
}
bool CCircuitTrace::Fix() {
    CCircuitTrace result;
    result.Name(Name());
    for (auto e : DImplementation->Edges) {
        SCircuitTraceEdge ends;
        if (result.SourceDestinationAsEdge(ends))
            for (auto bridge : TraceGeometry::Bridge(ends.DDestination, e.DSource))
                result.AddEdge(bridge);
        result.AddEdge(e);
    }
    *this = std::move(result);
    return !IsBroken();
}
bool CCircuitTrace::ConcatenateTrace(const CCircuitTrace &t) {
    if (!t.EdgeCount() || IsBroken() || t.IsBroken())
        return false;
    if (!EdgeCount()) {
        auto name = Name();
        *this = t;
        if (!name.empty())
            Name(name);
        return true;
    }
    auto a = DImplementation->Edges, b = t.DImplementation->Edges;
    auto reverse = [](auto &edges) {
        std::reverse(edges.begin(), edges.end());
        for (auto &e : edges)
            std::swap(e.DSource, e.DDestination);
    };
    if (a.back().DDestination == b.front().DSource) {
    } else if (a.back().DDestination == b.back().DDestination)
        reverse(b);
    else if (a.front().DSource == b.back().DDestination)
        a.swap(b);
    else if (a.front().DSource == b.front().DSource) {
        reverse(b);
        a.swap(b);
    } else
        return false;
    CCircuitTrace result;
    result.Name(Name());
    for (auto e : a)
        if (!result.AddEdge(e))
            return false;
    for (auto e : b)
        if (!result.AddEdge(e))
            return false;
    *this = std::move(result);
    return true;
}
CCircuitTrace CCircuitTrace::Simplify() const {
    CCircuitTrace result;
    result.Name(Name());
    std::vector<SCircuitTraceEdge> edges;
    for (auto e : DImplementation->Edges) {
        if (!edges.empty() && edges.back().DDestination == e.DSource &&
            TraceGeometry::Direction(edges.back()) == TraceGeometry::Direction(e))
            edges.back().DDestination = e.DDestination;
        else
            edges.push_back(e);
    }
    for (auto e : edges)
        result.AddEdge(e);
    return result;
}
CCircuitTrace CCircuitTrace::Elaborate() const {
    CCircuitTrace result;
    result.Name(Name());
    for (auto e : DImplementation->Edges) {
        auto a = e.DSource;
        auto [dx, dy] = TraceGeometry::Direction(e);
        while (a != e.DDestination) {
            auto b = a;
            if (dx)
                b.DX = dx > 0 ? a.DX + 1 : a.DX - 1;
            if (dy)
                b.DY = dy > 0 ? a.DY + 1 : a.DY - 1;
            result.AddEdge({a, b});
            a = b;
        }
    }
    return result;
}
