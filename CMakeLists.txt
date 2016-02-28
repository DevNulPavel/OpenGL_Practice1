set(PROJECT_NAME "OpenGL_Practice1")
# Конфиги флагов компиляции
set(PROJECT_C_FLAGS -O0 -wall -g)			# Проектные флаги компиляции для C
set(PROJECT_CXX_FLAGS -O0  -wall -g)			# Проектные флаги компиляции для C++
# Пути к исходникам
set(SRC_FOLDERS 
	src/)
# Конфиги используемых библиотек
set(LIBS_FOLDER libs/)
set(LIBS_FOLDERS 
	assimp/
	glm/
	freeImage/
	glfw/)
set(LIBS_PARAMETERS 
	)
set(LIBS_LINK_NAMES
	)
set(LIBS_INCLUDE_FOLDERS
	assimp/include/
	glm/glm/ 			# TODO: может можно просто корень
	freeImage/Source/
	glfw/include/)


####################################################
# Настройка CMake сборки
####################################################
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if (NOT CMAKE_VERSION VERSION_LESS "3.1")
	cmake_policy(SET CMP0054 NEW)
endif()

# Инклюдим файлик с разными хелперами для сборки
include("CMake/Helpers.cmake")

# Если у нас есть компилятор clang на юникс - используем его
if (UNIX)
	execute_process(COMMAND "which clang" OUTPUT_VARIABLE CLANG_PATH)
	execute_process(COMMAND "which clang++" OUTPUT_VARIABLE CLANGXX_PATH)
	if(NOT (${CLANG_PATH} STREQUAL ""))
		set(CMAKE_C_COMPILER ${CLANG_PATH})
		set(CMAKE_CXX_COMPILER ${CLANGXX_PATH})
	endif()
endif()

# Инициализация проекта
project(${PROJECT_NAME})

# Сразу отключаем ненужные предупреждения на Windows
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Не кастомные флаги компиляции
if (UNIX)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
  	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else (MSYS)
	# микрософту не нужен флаг -std=c++11
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif ()

# Флаги компиляции С и C++ из настроек проекта
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${PROJECT_C_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${PROJECT_CXX_FLAGS})

# Вывод информации о настройке копиляции (Функция в CMake/Helpers.cmake)
print_current_cmake_config()

####################################################
# Библиотеки
####################################################
# Прописываем инклюды в которых будем искать хедеры библиотек проекта
foreach(INCLUDE_DIR ${INCLUDE_DIRECTORIES})
	include_directories(${LIBS_FOLDER}${INCLUDE_DIR})
endforeach(INCLUDE_DIR)

# Запускаем конфигурацию подпроектов
foreach(LIBRARY_DIR ${LIBS_FOLDERS})
	add_subdirectory(${LIBS_FOLDER}${LIBRARY_DIR})
endforeach()


####################################################
# Исходники
####################################################
# Запускаем поиск иходников
foreach(DIR ${SRC_FOLDERS})
	# собираем список файлов в папке (не рекурсивно, не GLOB_RECURSE !)
	# можно еще обобщить, задавая список нужных расширений
	file(GLOB HEADERS "${DIR}/*.h")
	file(GLOB SOURCES "${DIR}/*.cpp" "${DIR}/*.c" "${DIR}/*.cc")
	# Формирование групп исходников для красивого отображения в проекте
	# для имен групп в xcode нужен обратный слэш
	string(REGEX REPLACE "/" "\\\\" GROUP_NAME ${DIR})
	source_group("Sources\\${GROUP_NAME}" FILES ${HEADERS} ${SOURCES})
	# сцепляем пути
	list(APPEND ALL_HEADERS ${HEADERS})
	list(APPEND ALL_SOURCES ${SOURCES})
endforeach(DIR)

# Из найденных исходников выставляем генерацию бинарника
add_executable(${PROJECT_NAME} ${ALL_HEADERS} ${ALL_SOURCES})