#include "CircuitNet.h"
#include "TraceGeometry.h"
#include <algorithm>
#include <map>
#include <set>
struct CCircuitNet::SImplementation {
    TCircuitName Name;
    std::vector<SCircuitLocation> Pins;
    std::vector<CCircuitTrace> Traces;
};
namespace {
using TLocation = SCircuitLocation;
using TAdjacency = std::map<TLocation, std::set<TLocation>>;
using TLink = std::pair<TLocation, TLocation>;
TLink Link(TLocation a, TLocation b) {
    return a < b ? TLink{a, b} : TLink{b, a};
}
TAdjacency BuildAdjacency(const std::vector<CCircuitTrace> &traces) {
    TAdjacency graph;
    for (const auto &trace : traces) {
        auto expanded = trace.Elaborate();
        for (std::size_t i = 0; i < expanded.EdgeCount(); ++i) {
            SCircuitTraceEdge e;
            expanded.Edge(i, e);
            graph[e.DSource].insert(e.DDestination);
            graph[e.DDestination].insert(e.DSource);
        }
    }
    return graph;
}
} // namespace
CCircuitNet::CCircuitNet() : DImplementation(std::make_unique<SImplementation>()) {}
CCircuitNet::~CCircuitNet() = default;
CCircuitNet::CCircuitNet(const CCircuitNet &n) : CCircuitNet() {
    *this = n;
}
CCircuitNet::CCircuitNet(CCircuitNet &&n) noexcept = default;
CCircuitNet &CCircuitNet::operator=(const CCircuitNet &n) {
    if (this != &n)
        DImplementation = std::make_unique<SImplementation>(*n.DImplementation);
    return *this;
}
CCircuitNet &CCircuitNet::operator=(CCircuitNet &&n) noexcept = default;
TCircuitName CCircuitNet::Name() const {
    return DImplementation->Name;
}
TCircuitName CCircuitNet::Name(const TCircuitName &n) {
    if (!n.empty())
        DImplementation->Name = n;
    return Name();
}
std::size_t CCircuitNet::PinCount() const {
    return DImplementation->Pins.size();
}
std::size_t CCircuitNet::TraceCount() const {
    return DImplementation->Traces.size();
}
bool CCircuitNet::PinLocation(std::size_t i, SCircuitLocation &l) const {
    if (i >= PinCount())
        return false;
    l = DImplementation->Pins[i];
    return true;
}
bool CCircuitNet::AddPinLocation(const SCircuitLocation &l) {
    auto &pins = DImplementation->Pins;
    if (std::find(pins.begin(), pins.end(), l) != pins.end())
        return false;
    pins.push_back(l);
    return true;
}
bool CCircuitNet::Trace(std::size_t i, CCircuitTrace &t) const {
    if (i >= TraceCount())
        return false;
    t = DImplementation->Traces[i];
    return true;
}
bool CCircuitNet::AddTrace(const CCircuitTrace &t) {
    if (!t.EdgeCount() || t.Name() != Name())
        return false;
    // Reject the same undirected edge set, even if supplied in reverse order.
    auto signature = [](const CCircuitTrace &trace) {
        std::set<TLink> edges;
        for (std::size_t i = 0; i < trace.EdgeCount(); ++i) {
            SCircuitTraceEdge edge;
            trace.Edge(i, edge);
            edges.insert(Link(edge.DSource, edge.DDestination));
        }
        return edges;
    };
    auto incoming = signature(t);
    for (const auto &old : DImplementation->Traces)
        if (signature(old) == incoming)
            return false;
    DImplementation->Traces.push_back(t);
    return true;
}
bool CCircuitNet::IsBroken() const {
    if (!TraceCount() || !PinCount())
        return true;
    for (const auto &t : DImplementation->Traces)
        if (t.IsBroken())
            return true;
    auto graph = BuildAdjacency(DImplementation->Traces);
    auto start = DImplementation->Pins.front();
    if (!graph.contains(start))
        return true;
    std::set<TLocation> seen{start};
    std::vector<TLocation> stack{start};
    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();
        for (auto next : graph[current])
            if (seen.insert(next).second)
                stack.push_back(next);
    }
    for (auto pin : DImplementation->Pins)
        if (!seen.contains(pin))
            return true;
    return false;
}
bool CCircuitNet::Fix() {
    for (auto &t : DImplementation->Traces)
        t.Fix();
    return !IsBroken();
}
bool CCircuitNet::ElaborateTraces() {
    if (!TraceCount())
        return false;
    for (auto &t : DImplementation->Traces)
        t = t.Elaborate();
    return true;
}
// Unit edges form a union: overlaps disappear automatically. Walk each chain
// until a pin, endpoint or branch, then compress straight runs within it.
bool CCircuitNet::SimplifyTraces() {
    if (!TraceCount())
        return false;
    auto graph = BuildAdjacency(DImplementation->Traces);
    std::set<TLocation> pins(DImplementation->Pins.begin(), DImplementation->Pins.end());
    std::set<TLink> used;
    std::vector<CCircuitTrace> result;
    auto boundary = [&](TLocation p) { return pins.contains(p) || graph.at(p).size() != 2; };
    auto walk = [&](TLocation start, TLocation next) {
        CCircuitTrace trace;
        trace.Name(Name());
        auto current = start;
        while (!used.contains(Link(current, next))) {
            used.insert(Link(current, next));
            trace.AddEdge({current, next});
            auto previous = current;
            current = next;
            if (boundary(current))
                break;
            const auto &neighbors = graph.at(current);
            next = *neighbors.begin();
            if (next == previous)
                next = *neighbors.rbegin();
        }
        result.push_back(trace.Simplify());
    };
    for (const auto &[point, neighbors] : graph)
        if (boundary(point))
            for (auto next : neighbors)
                if (!used.contains(Link(point, next)))
                    walk(point, next);
    // A closed loop has no endpoint or branch; start at its smallest location.
    for (const auto &[point, neighbors] : graph)
        for (auto next : neighbors)
            if (!used.contains(Link(point, next)))
                walk(point, next);
    DImplementation->Traces = std::move(result);
    return true;
}
