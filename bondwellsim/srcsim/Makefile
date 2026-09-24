# Compatible with z80pack's top-level machines/clean targets.
.PHONY: all clean distclean test
all:
	cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
	cmake --build ../build
test: all
	ctest --test-dir ../build --output-on-failure
clean:
	cmake --build ../build --target clean
distclean: clean
