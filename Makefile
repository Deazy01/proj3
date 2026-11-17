CXX ?= g++
CPPFLAGS += -Iinclude -Itests/src
CXXFLAGS += -O0 --std=c++20 -g -Wall -Wextra -Wpedantic --coverage -MMD -MP
LDLIBS += -lgtest_main -lgtest -pthread
LDFLAGS += --coverage
TESTS_HTMLCOV_DIR := tests/htmlcov
COVERAGE_INFO_FILE := tests/coverage.info
OBJECTS := obj/Graph.o obj/MutableGraph.o obj/GridGraph.o obj/CircuitTrace.o obj/CircuitNet.o
PROGRAMS := tests/bin/testgraph tests/bin/testmutablegraph tests/bin/testgridgraph tests/bin/testtrace tests/bin/testnet tests/bin/testadditional
.PHONY: all test coverage clean directories
all: test

directories:
	mkdir -p bin obj tests/htmlconv tests/htmlcov tests/bin tests/obj

obj/%.o: src/%.cpp | directories
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

tests/obj/%.o: tests/src/%.cpp | directories
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Each executable has its own GoogleTest main, so suites run independently.
define TEST_RULE
 tests/bin/$(1): $(OBJECTS) tests/obj/$(2).o | directories
	$$(CXX) $$(LDFLAGS) $$^ $$(LDLIBS) -o $$@
endef
$(eval $(call TEST_RULE,testgraph,GraphTest))
$(eval $(call TEST_RULE,testmutablegraph,MutableGraphTest))
$(eval $(call TEST_RULE,testgridgraph,GridGraphTest))
$(eval $(call TEST_RULE,testtrace,CircuitTraceTest))
$(eval $(call TEST_RULE,testnet,CircuitNetTest))
$(eval $(call TEST_RULE,testadditional,AdditionalTest))

test: $(PROGRAMS)
	@set -e; for executable in $(PROGRAMS); do ./$$executable; done

coverage: test
	@set -e; trap 'rm -f lcov.err' EXIT; \
	if lcov --help 2>&1 | grep -q ignore-errors; then \
	  lcov --capture --directory "$(CURDIR)" --output-file $(COVERAGE_INFO_FILE) --ignore-errors inconsistent,source 2>lcov.err; \
	else lcov --capture --directory "$(CURDIR)" --output-file $(COVERAGE_INFO_FILE) 2>lcov.err; fi; \
	grep -v 'mismatched end line' lcov.err >&2 || true
	lcov --remove $(COVERAGE_INFO_FILE) '/usr/*' '*/tests/src/*' '*/include/*' --output-file $(COVERAGE_INFO_FILE)
	genhtml $(COVERAGE_INFO_FILE) --output-directory $(TESTS_HTMLCOV_DIR)

clean:
	rm -rf bin obj tests/htmlconv tests/htmlcov tests/bin tests/obj
	rm -f tests/coverage.info lcov.err

-include $(wildcard obj/*.d tests/obj/*.d)
