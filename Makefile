

LINUX:
	mkdir -p build && \
	cd build &&	\
	cmake -G "Unix Makefiles" ../ && \
	echo "*********** Make started **********" && \
	make && \
	cd ../


clean:
	rm -rf build && \
	mkdir build