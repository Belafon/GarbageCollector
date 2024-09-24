default:
	g++ -std=c++20 tests.cpp -o tests
	./tests
	g++ -std=c++20 example.cpp -o example
	./example
