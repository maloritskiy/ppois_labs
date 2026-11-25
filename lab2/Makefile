CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
RELEASE_FLAGS = -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic

HEADERS = audio_collection.hpp audio_core.hpp audio_metadata.hpp \
          audio_playback.hpp audio_search.hpp audio_storage.hpp \
          audio_statistics.hpp

.PHONY: all run run_release test coverage release clean

all: audio_collection_cli

audio_collection_cli: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) main.cpp -o audio_collection_cli

audio_collection_tests: tests.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) tests.cpp -o audio_collection_tests

run: audio_collection_cli
	./audio_collection_cli

run_release: audio_collection_cli_release
	./audio_collection_cli_release

test: audio_collection_tests
	./audio_collection_tests

coverage: tests.cpp $(HEADERS)
	rm -f *.gcda *.gcno
	$(CXX) $(CXXFLAGS) --coverage -O0 -g tests.cpp -o audio_collection_tests_coverage
	./audio_collection_tests_coverage
	gcovr --root . --filter 'audio_.*\.hpp$$' --fail-under-line 90.01 .

release: main.cpp $(HEADERS)
	$(CXX) $(RELEASE_FLAGS) main.cpp -o audio_collection_cli_release

clean:
	rm -f audio_collection_cli audio_collection_tests \
	      audio_collection_tests_coverage audio_collection_cli_release \
	      *.o *.gcda *.gcno