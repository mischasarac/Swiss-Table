CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic
GTEST_LIBS := -lgtest -lgtest_main -pthread

test:
	$(CXX) $(CXXFLAGS) \
		./testing/swiss_table_tests.cpp \
		$(GTEST_LIBS) \
		-o tests
	./tests
	rm tests

test_insert:
	$(CXX) $(CXXFLAGS) \
		./testing/insert_tests.cpp \
		$(GTEST_LIBS) \
		-o tests
	./tests
	rm tests

compilation_test:
	$(CXX) $(CXXFLAGS) \
		-fsyntax-only \
		./testing/compilation_tests.cpp