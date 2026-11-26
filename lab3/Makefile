CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
RELEASE_FLAGS = -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic

HEADERS = restaurant_management.hpp restaurant_exceptions.hpp \
          restaurant_core.hpp restaurant_menu.hpp restaurant_customers.hpp \
          restaurant_orders.hpp restaurant_staff.hpp restaurant_kitchen.hpp \
          restaurant_inventory.hpp restaurant_delivery.hpp restaurant_statistics.hpp

.PHONY: all run test coverage release clean

all: restaurant_cli

restaurant_cli: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) main.cpp -o restaurant_cli

restaurant_tests: tests.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) tests.cpp -o restaurant_tests

run: restaurant_cli
	./restaurant_cli

test: restaurant_tests
	./restaurant_tests

coverage: tests.cpp $(HEADERS)
	rm -f *.gcda *.gcno
	$(CXX) $(CXXFLAGS) --coverage -O0 -g tests.cpp -o restaurant_tests_coverage
	./restaurant_tests_coverage
	gcovr --root . --filter 'restaurant_.*[.]hpp$$' --fail-under-line 90.01 --print-summary .

release: main.cpp $(HEADERS)
	$(CXX) $(RELEASE_FLAGS) main.cpp -o restaurant_cli_release

clean:
	rm -f restaurant_cli restaurant_tests restaurant_tests_coverage \
	      restaurant_cli_release *.o *.gcda *.gcno *.gcov