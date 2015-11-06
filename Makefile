all: hello-cpp-world hello-c-world gettweets1

%: %.cc
	g++ -std=c++11 $< -o $@

%: ./src/%.c
	gcc $< -o $@

