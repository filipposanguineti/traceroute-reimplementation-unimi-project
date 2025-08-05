# Makefile wrapper per semplificare l'uso di CMake
.PHONY: all run clean

all:
	@cmake -S . -B build
	@cmake --build build

run: all
	@./bin/traceroute

clean:
	@rm -rf build
