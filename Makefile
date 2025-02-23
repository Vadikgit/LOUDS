OBJECTS := objects
ALL_LDFLAGS := $(LDFLAGS) $(EXTRA_LDFLAGS)

clean:
	@rm -rf $(OBJECTS)/* main test benchmark
	@rm -rf $(OBJECTS)

# Directories
$(OBJECTS):
	$(Q)mkdir -p $@

$(OBJECTS)/rank_counter.o: rank_counter.cpp rank_counter.h | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -g -c rank_counter.cpp -o $@

$(OBJECTS)/select_counter.o: select_counter.cpp select_counter.h | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -g -c select_counter.cpp -o $@

$(OBJECTS)/louds.o: louds.cpp louds.h | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -g -c louds.cpp -o $@

$(OBJECTS)/main.o: main.cpp louds.h | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -g -c main.cpp -o $@

$(OBJECTS)/test.o: test.cpp louds.h | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -g -c test.cpp -o $@
	
$(OBJECTS)/benchmark.o: benchmark.cpp louds.h | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -g -c benchmark.cpp -o $@

main: $(OBJECTS)/rank_counter.o $(OBJECTS)/select_counter.o $(OBJECTS)/louds.o $(OBJECTS)/main.o | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) $^ $(ALL_LDFLAGS) -g -lelf -lz -o $@

test: $(OBJECTS)/rank_counter.o $(OBJECTS)/select_counter.o $(OBJECTS)/louds.o $(OBJECTS)/test.o | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) $^ $(ALL_LDFLAGS) -g  -lgtest -lgmock -pthread -lelf -lz -o $@

benchmark: $(OBJECTS)/rank_counter.o $(OBJECTS)/select_counter.o $(OBJECTS)/louds.o $(OBJECTS)/benchmark.o | $(OBJECTS)
	@$(CXX) $(CXXFLAGS) $^ $(ALL_LDFLAGS) -g -std=c++14 -lbenchmark -lpthread -lelf -lz -o $@

.DELETE_ON_ERROR:

.SECONDARY:
