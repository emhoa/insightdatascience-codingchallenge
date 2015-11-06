all: gettweets1

%: ./src/%.c
	gcc $< -o $@

