.PHONY: build

build:
	mkdir -p build/
	c++ -Wall src/main.cpp -o build/bf

clean:
	rm build/bf
	rm build
