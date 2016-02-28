

LINUX:
	mkdir -p build && \
	cd build &&	\
	cmake -G "Unix Makefiles" ../

clean:
	rm -rf build && \
	mkdir build