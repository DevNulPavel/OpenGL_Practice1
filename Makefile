
# кол-во потоков сборки
ifndef JOB_SLOTS_NUMBER
	OS=$(shell uname)
	# узнаем сколько ядер
	ifeq ($(OS),Linux)
		# shell nproc - можно и так ( grep -c ^processor /proc/cpuinfo)
	  	NPROCS=$(shell nproc | sed 's/$$/+1/' | bc)
	else ifeq ($(OS),Darwin)
	  	NPROCS=$(shell sysctl -n hw.ncpu | sed 's/$$/+1/' | bc)
	else
		NPROCS=4
	endif

	JOB_SLOTS_NUMBER=$(NPROCS)
endif

#############################################
# Цели сборки
#############################################
LINUX:
	mkdir -p build/Linux && \
	cd build/Linux &&	\
	cmake -G "Unix Makefiles" ../../ && \
	echo "*********** Make started **********" && \
	make -j $(JOB_SLOTS_NUMBER) && \
	cd ../../

clean:
	rm -rf build && \
	mkdir build && \
	mkdir -p build/Linux && \
	mkdir -p build/LinuxQtCreator && \
	mkdir -p build/LinuxKDevelop

clear: clean