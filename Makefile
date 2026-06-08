make test:
	g++ \
    -std=c++20 \
    ./testing/swiss_table_tests.cpp \
    -lgtest \
    -lgtest_main \
    -pthread \
    -o tests
	./tests
	rm tests