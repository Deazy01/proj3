# ECS 36C Project 3 — Graphs and Circuit Building Blocks

Student name: Joshua Kofoworola  
Student ID: 924641574


## Project status

Implemented the full mutable graph, mutable grid graph, circuit trace and circuit net
interfaces. Added a Makefile, ten additional GoogleTests. All 155 supplied tests and 10 additional tests passed locally.


## Build and run in VS Code

1. Extract the archive and open its `proj3` folder in VS Code.
2. Install Docker Desktop and VS Code's Dev Containers extension.
3. Start Docker Desktop. In VS Code run **Dev Containers: Reopen in Container**.
   The supplied `.devcontainer` configuration installs the build tools and GoogleTest.
4. In the container terminal run:

```sh
make
make coverage
```

`make` compiles and executes all six test executables. `make coverage` also generates
`tests/htmlcov/index.html`. Open that HTML file in a browser. No separate application
or interactive main program is needed.
For an ordinary Debian/Ubuntu system, the dependencies are `build-essential`,
`libgtest-dev`, and `lcov`.

```sh
make clean       # remove generated directories and coverage data
make -j4         # compile and run all test suites again
```

The Makefile creates both `tests/htmlconv` (the spelling in one requirement) and
`tests/htmlcov` (the coverage-output spelling). The grid executable is called
`testgridgraph`, because reusing `testgraph` for two suites would overwrite it.
All executables are in `tests/bin`. `bin` is also created as requested.
Coverage supports lcov 1.x and 2.x, filtering the requested inconsistent/source
warnings where the installed version supports them. Errors still fail the build.

## Implementation choices

- `CSimpleGraph`: an ordered map of stable vertex IDs. Each vertex has its tag
  and another ordered map from destination ID to edge weight. Deleted IDs are
  not reused. A bidirectional edge inserts two independent directed edges;
  removing or changing an edge affects only the requested direction. Adding an
  existing directed edge updates its weight.
- BFS uses a queue; DFS uses a stack. Both return NaN for distance, as required by
  the supplied tests. The weighted search uses Dijkstra's algorithm, validating
  that all graph weights are finite and nonnegative before callbacks begin.
  It returns false for an invalid start or Stop; true on completion.
  ForEach reports invalid parent, depth zero and NaN distance for each vertex.
- `CSimpleGridGraph`: row-major IDs (`y * width + x`). It reuses graph traversal
  and tag storage, computing neighbors from its direction masks. Cardinal edges
  weigh 1 and diagonal edges weigh sqrt(2). Edges can be one-way. Boundary bits
  are filtered out. Per-vertex overrides survive pattern changes; clearing an
  override restores the pattern. Vertex/edge mutation inherited from the reuse
  base is disabled so the grid retains its fixed dimensions and mask topology.
- `CCircuitTrace`: vectors preserve insertion order for edges and unique
  locations. Reverse edges count as duplicates. IsBroken checks consecutive
  edge endpoints. Fix inserts a diagonal-first connection and then a cardinal
  segment where necessary. Simplify merges consecutive segments that have the
  same direction. Elaborate divides edges into unit grid steps.
- `CCircuitNet`: stores pins and traces. Connectivity builds an undirected graph
  of unit segments and searches from the first pin. Every pin must be reachable;
  unrelated extra components do not by themselves break the net. Broken
  individual traces do break it. Fix repairs each trace, but never routes between
  separate traces. Trace names must match the net name. Identical undirected
  edge sets are rejected as duplicate traces.
- Net simplification first expands and unions unit edges, removing overlaps.
  It walks chains between pins, branches and endpoints, simplifies their straight
  sections, and separately handles closed loops. Pins inside existing segments
  are preserved as trace boundaries. Returned traces use the net's name.
- Both PImpl classes implement deep copying and ownership-transferring moves.
  A moved-from object may be destroyed or assigned a new value; its getters are
  not intended for use until reassigned.

## Test interpretation and limits

`SimplifyMergesTracesAtNonPinJunction` uses A=(0,0), B=(10,10), X=(15,15).
B is a pin inside segment A–X, and X–B overlaps part of A–X. The expected result
is two segments A–B and B–X. This is consistent with unioning overlaps and keeping
pins, despite that test's title. The supplied test assertions were preserved;
only the concrete types and factory includes were substituted.

Trace segments are horizontal, vertical or 45-degree diagonals on the integer
circuit grid. Zero-length and other slopes are rejected by AddEdge. Intersections
are electrical junctions at shared integer grid locations; half-grid crossings
cannot be represented by SCircuitLocation. Expansion requires time and memory
proportional to segment length (and vector duplicate checks add overhead), so
this intentionally simple version targets modest course examples, not enormous
boards. Empty nets and nets without pins are treated as broken. Empty traces
have no gaps and are not individually broken, but cannot be added to a net.
Methods required to be noexcept use standard containers; allocation failure can
terminate, as permitted by the interface's noexcept contract.

## Generative AI Use

I used OpenAI ChatGPT to help implement and understand Project 3. And asked the following questions:
“When I delete a vertex, some other connections stop working. How can I remove it without changing the remaining vertex IDs?”

“My BFS keeps visiting the same vertices in a cycle. Should I mark a vertex visited when I add it to the queue or when I remove it?”

“The corner vertices in my grid are getting neighbors outside the grid. How should I check boundaries before applying the direction mask?”

“My duplicate-trace test is failing. How can I compare two traces when their edges might be stored in reverse order?”

“My SimplifyTraces() merges straight segments, but it also removes a pin in the middle. How can I preserve that pin as a connection point?”

AI Response:
1. Removing a vertex without changing other IDs
   Avoid erasing a vertex from a vector if its position represents its ID, because later elements shift. Store vertices in a map keyed by a stable ID. Delete the selected vertex, then remove references to it from every other vertex’s adjacency list. Keep a separate counter for assigning new IDs.
2. Preventing repeated visits in BFS
   Mark a vertex as visited when you add it to the queue. Otherwise, several vertices could enqueue the same neighbor before it is processed. Mark the starting vertex before the search begins, too.
   if (visited.insert(neighbor).second) {
       queue.push(neighbor);
   }
   
   Here, .second is true only when the insertion adds a previously unseen vertex.
3. Checking grid boundaries
   For each direction, first check whether its coordinate change would leave the grid. Only calculate the neighbor after that check. Since coordinates use unsigned integers, subtracting 1 from 0 wraps around to a very large value.
   For example, moving northwest requires:
   if (x > 0 && y > 0) {
       auto neighbor = GetVertex(x - 1, y - 1);
   }
   
   Filter invalid directions out of the effective connection mask, including when a vertex has a custom override.
4. Comparing traces with reversed edges
   Normalize each edge by putting its smaller endpoint first. Then store the normalized edges in a set and compare the two sets.
   auto normalized = source < destination
   
   This treats A→B and B→A as the same electrical connection and ignores edge insertion order. It detects identical edge sets; detecting equivalent geometry with different segment lengths requires expansion or further normalization.
5. Preserving a pin during simplification
   Treat every pin as a stopping point when building simplified traces. You can merge through an ordinary point with exactly two neighbors, but stop at a pin even when it has only two neighbors.
   
## Code references

1. Instructor-provided `proj3.tgz`: all supplied headers, tests and development
   container files. The project description and diagrams supplied with the task
   determined the required behavior.


