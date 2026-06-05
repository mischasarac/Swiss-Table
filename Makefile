make test:
	g++ \
    ./testing/swiss_table_tests.cpp \
    ./src/swiss_map.cpp \
    -lgtest \
    -lgtest_main \
    -pthread \
    -o tests
	./tests
	rm tests