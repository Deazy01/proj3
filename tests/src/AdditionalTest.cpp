#include "CircuitNet.h"
#include "SimpleGraph.h"
#include "SimpleGridGraph.h"
#include <cmath>
#include <gtest/gtest.h>

TEST(AdditionalGraph, InvalidQueriesAndStableIDs) {
    CSimpleGraph g({1, 2, 3});
    std::size_t tag = 99;
    std::vector<std::size_t> neighbors{42};
    double weight = 9;
    EXPECT_FALSE(g.GetVertexTag(99, tag));
    EXPECT_EQ(tag, 99U);
    EXPECT_FALSE(g.GetVertexNeighbors(99, neighbors));
    EXPECT_EQ(neighbors[0], 42U);
    EXPECT_FALSE(g.GetEdgeWeight(0, 1, weight));
    EXPECT_EQ(weight, 9);
    EXPECT_TRUE(g.RemoveVertex(1));
    EXPECT_FALSE(g.RemoveVertex(1));
    EXPECT_EQ(g.AddVertex(4), 3U);
    EXPECT_TRUE(g.GetVertexTag(2, tag));
    EXPECT_TRUE(g.AddEdge(0, 0, 1));
    EXPECT_TRUE(g.RemoveEdge(0, 0));
    EXPECT_TRUE(g.AddEdge(0, 2));
    EXPECT_TRUE(g.GetEdgeWeight(0, 2, weight));
    EXPECT_TRUE(std::isnan(weight));
}
TEST(AdditionalGraph, TraversalControlsAndInvalidWeights) {
    CSimpleGraph g({0, 1, 2}, {{0, 1, 1, false}, {1, 2, 1, false}, {0, 2, 8, false}});
    int count = 0;
    auto visit = [&](auto, auto, auto, auto) {
        ++count;
        return CGraph::EGraphTraversal::SkipNeighbors;
    };
    g.DepthFirstSearch(0, visit);
    EXPECT_EQ(count, 1);
    EXPECT_TRUE(g.BreadthFirstSearchWeighted(0, visit));
    EXPECT_EQ(count, 2);
    auto stop = [](auto, auto, auto, auto) { return CGraph::EGraphTraversal::Stop; };
    EXPECT_FALSE(g.BreadthFirstSearchWeighted(0, stop));
    g.BreadthFirstSearchUnweighted(99, visit);
    g.DepthFirstSearch(99, visit);
    EXPECT_FALSE(g.BreadthFirstSearchWeighted(99, visit));
    EXPECT_FALSE(g.ForEach({}));
    g.DepthFirstSearch(0, {});
    g.BreadthFirstSearchUnweighted(0, {});
    EXPECT_FALSE(g.BreadthFirstSearchWeighted(0, {}));
    g.SetEdgeWeight(0, 1, CGraph::EdgeWeightInvalid);
    EXPECT_FALSE(g.BreadthFirstSearchWeighted(0, visit));
}
TEST(AdditionalGrid, TagsEmptyDimensionsAndFixedTopology) {
    CSimpleGridGraph g(2, 2);
    EXPECT_TRUE(g.SetVertexTag(0, 42));
    std::size_t tag = 0;
    EXPECT_TRUE(g.GetVertexTag(0, tag));
    EXPECT_EQ(tag, 42U);
    EXPECT_EQ(g.AddVertex(5), CGraph::VertexIDInvalid);
    EXPECT_FALSE(g.RemoveVertex(0));
    EXPECT_FALSE(g.AddEdge(0, 1));
    EXPECT_FALSE(g.RemoveEdge(0, 1));
    EXPECT_FALSE(g.SetEdgeWeight(0, 1, 9));
    std::vector<std::size_t> out;
    double w;
    SGridLocation l{9, 9};
    EXPECT_FALSE(g.GetVertexNeighbors(99, out));
    EXPECT_FALSE(g.GetVertexLocation(99, l));
    EXPECT_FALSE(g.GetEdgeWeight(0, 0, w));
    EXPECT_FALSE(g.EdgeExists(99, 0));
    EXPECT_EQ(g.GetVertexConnectionMask(99), 0);
    CSimpleGridGraph empty(0, 3);
    EXPECT_EQ(empty.VertexCount(), 0U);
    EXPECT_EQ(empty.GetVertex(0, 0), CGraph::VertexIDInvalid);
    EXPECT_THROW(CSimpleGridGraph(CGraph::VertexIDInvalid, 2), std::length_error);
}
TEST(AdditionalTrace, CopiesMovesNamesAndInvalidEdges) {
    CCircuitTrace t;
    SCircuitTraceEdge e;
    EXPECT_FALSE(t.SourceDestinationAsEdge(e));
    EXPECT_TRUE(t.Fix());
    EXPECT_FALSE(t.AddEdge({{0, 0}, {0, 0}}));
    EXPECT_FALSE(t.AddEdge({{0, 0}, {2, 1}}));
    t.Name("N");
    t.AddEdge({{0, 0}, {2, 0}});
    CCircuitTrace copy(t);
    copy.Name("Other");
    EXPECT_EQ(t.Name(), "N");
    copy = t;
    copy = copy;
    EXPECT_EQ(copy.EdgeCount(), 1U);
    CCircuitTrace moved(std::move(copy));
    copy = std::move(moved);
    EXPECT_EQ(copy.Name(), "N");
    CCircuitTrace empty;
    EXPECT_TRUE(empty.ConcatenateTrace(t));
    EXPECT_FALSE(t.ConcatenateTrace(t));
    EXPECT_EQ(t.Elaborate().Name(), "N");
    EXPECT_EQ(t.Simplify().Name(), "N");
}
TEST(AdditionalTrace, ReverseStepsAndRepairTurns) {
    CCircuitTrace t;
    t.AddEdge({{3, 2}, {0, 2}});
    EXPECT_EQ(t.Elaborate().EdgeCount(), 3U);
    CCircuitTrace vertical;
    vertical.AddEdge({{1, 3}, {1, 0}});
    EXPECT_EQ(vertical.Elaborate().EdgeCount(), 3U);
    t.AddEdge({{0, 0}, {0, 1}});
    EXPECT_TRUE(t.IsBroken());
    EXPECT_TRUE(t.Fix());
    auto simple = t.Simplify();
    EXPECT_FALSE(simple.IsBroken());
    CCircuitTrace disconnected;
    disconnected.AddEdge({{0, 0}, {1, 0}});
    disconnected.AddEdge({{5, 5}, {6, 5}});
    EXPECT_FALSE(t.ConcatenateTrace(disconnected));
}
TEST(AdditionalNet, NamesEmptyTraceAndOutputIndependence) {
    CCircuitNet n;
    n.Name("N");
    CCircuitTrace t;
    EXPECT_FALSE(n.AddTrace(t));
    t.AddEdge({{0, 0}, {2, 0}});
    EXPECT_FALSE(n.AddTrace(t));
    t.Name("N");
    EXPECT_TRUE(n.AddTrace(t));
    EXPECT_TRUE(n.IsBroken());
    CCircuitTrace out;
    EXPECT_FALSE(n.Trace(99, out));
    EXPECT_TRUE(n.Trace(0, out));
    out.Name("Changed");
    EXPECT_TRUE(n.Trace(0, out));
    EXPECT_EQ(out.Name(), "N");
    n.AddPinLocation({1, 0});
    EXPECT_FALSE(n.IsBroken());
    CCircuitNet copy(n);
    copy = copy;
    EXPECT_FALSE(copy.IsBroken());
}
TEST(AdditionalNet, NonPinChainAndLoop) {
    CCircuitNet n;
    CCircuitTrace a, b;
    a.AddEdge({{0, 0}, {2, 0}});
    b.AddEdge({{2, 0}, {4, 0}});
    n.AddTrace(a);
    n.AddTrace(b);
    EXPECT_TRUE(n.SimplifyTraces());
    EXPECT_EQ(n.TraceCount(), 1U);
    CCircuitNet loop;
    CCircuitTrace square;
    square.AddEdge({{0, 0}, {2, 0}});
    square.AddEdge({{2, 0}, {2, 2}});
    square.AddEdge({{2, 2}, {0, 2}});
    square.AddEdge({{0, 2}, {0, 0}});
    loop.AddTrace(square);
    EXPECT_TRUE(loop.SimplifyTraces());
    EXPECT_EQ(loop.TraceCount(), 1U);
    loop.AddPinLocation({0, 0});
    EXPECT_FALSE(loop.IsBroken());
}
