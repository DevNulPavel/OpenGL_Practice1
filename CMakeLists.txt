set(PROJECT_NAME "OpenGL_Practice1")
set(EXECUTABLE_NAME "OpenGLBin")
# Конфиги флагов компиляции
set(PROJECT_C_FLAGS "-O0 -Wall -g")			# Проектные флаги компиляции для C
set(PROJECT_CXX_FLAGS "-O0 -Wall -g")			# Проектные флаги компиляции для C++
set(PROJECT_LINK_FLAGS )					# Флаги линковки
# Пути к исходникам
set(SRC_FOLDERS
	src
)
# Конфиги используемых библиотек
set(LIBS_FOLDER libs/)
set(LIBS_FOLDERS
	glm
	# freeImage
	glfw
)
set(LIBS_PARAMETERS
)
set(LIBS_LINK_NAMES
	glfw
)
set(LIBS_DEPENCY_NAMES
	# TODO: проверить
	glfw
	glm
)
set(LIBS_INCLUDE_FOLDERS
	glm/glm 			# TODO: может можно просто корень?
	# freeImage/Source
	glfw/include
    glfw/deps
)


####################################################
# Настройка CMake сборки
####################################################
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
if (NOT CMAKE_VERSION VERSION_LESS "3.1")
	cmake_policy(SET CMP0054 NEW)
endif()

# Инклюдим файлик с разными хелперами для сборки
include("CMake/Helpers.cmake")

# Если у нас есть компилятор clang на юникс - используем его
if (UNIX)
	message("********* Unix compilation check *********")
	 # Используем ccache (если есть) для ускорения сборки
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        message("-> ССache will used for compilation")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)

    # Вариант с путями
	# execute_process(COMMAND which clang OUTPUT_VARIABLE CLANG_PATH)
	# execute_process(COMMAND which clang++ OUTPUT_VARIABLE CLANGXX_PATH)
	# if(NOT ("${CLANG_PATH}" STREQUAL ""))
	find_program(CLANG_FOUND clang)
	if(CLANG_FOUND)
		message("-> Clang will used for compilation")
		set(CMAKE_C_COMPILER "clang")
		set(CMAKE_CXX_COMPILER "clang++")
	endif(CLANG_FOUND)
	message("**********************************")
endif()

# Инициализация проекта
project(${PROJECT_NAME} C CXX)

enable_testing()

####################################################
# Библиотеки
####################################################
# Прописываем инклюды в которых будем искать хедеры библиотек проекта
message("******** Lib include dirs ***********")
foreach(INCLUDE_DIR ${LIBS_INCLUDE_FOLDERS})
	include_directories(${LIBS_FOLDER}${INCLUDE_DIR})
	message(${LIBS_FOLDER}${INCLUDE_DIR})
endforeach(INCLUDE_DIR)

# Запускаем конфигурацию подпроектов
foreach(LIBRARY_DIR ${LIBS_FOLDERS})
	message("******** Subproject start ***********")
	message("Project with path = ${LIBS_FOLDER}${LIBRARY_DIR}")
	add_subdirectory(${LIBS_FOLDER}${LIBRARY_DIR} ${CMAKE_BINARY_DIR}/libs/${LIBRARY_DIR})
	message("LIBS = ${${LIBRARY_DIR}_LIBRARIES}")
	message("includes = ${GLFW_INCLUDE_DIRS}")
	message("libs = ${GLFW_PKG_DEPS}")
	message("pkg = ${GLFW_PKG_LIBS}")
endforeach()

####################################################
# Системные библиотеки
####################################################
if(APPLE)
	set (SYSTEM_LIBS
		"-framework OpenGL"
		"-framework Cocoa"
		"-framework IOKit"
		"-framework CoreVideo"
		"-ObjC")
else (UNIX)
	set (SYSTEM_LIBS
		rt Xrandr Xinerama Xi Xxf86vm Xcursor
        m c dl z GL GLEW pthread X11)
else (MSYS)
	set (SYSTEM_LIBS )
	# TODO:
endif ()
message("******** System libs list ***********")
message("${SYSTEM_LIBS}")


####################################################
# Настройка сборки текущего проекта
####################################################
# Сразу отключаем ненужные предупреждения на Windows
if (MSYS)
	add_definitions ( -D_CRT_SECURE_NO_WARNINGS )
	add_definitions ( -D_SCL_SECURE_NO_WARNINGS )
endif ()

# Не кастомные флаги компиляции
if(APPLE)
	# TODO: пробросить линковку
	set (PROJECT_LINK_FLAGS "${PROJECT_LINK_FLAGS} -stdlib=libc++")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else (UNIX)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
  	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else (MSYS)
	# микрософту не нужен флаг -std=c++11
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif ()

# Флаги компиляции С и C++ из настроек проекта
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PROJECT_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PROJECT_CXX_FLAGS}")

# Вывод информации о настройке копиляции (Функция в CMake/Helpers.cmake)
print_current_cmake_config()


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
    # source_group("Sources\\${GROUP_NAME}" FILES ${HEADERS} ${SOURCES})
	source_group("${GROUP_NAME}" FILES ${HEADERS} ${SOURCES})
	# сцепляем пути
	list(APPEND ALL_HEADERS ${HEADERS})
	list(APPEND ALL_SOURCES ${SOURCES})
endforeach(DIR)
message("******** Sources list ***********")
message("${ALL_HEADERS}")
message("${ALL_SOURCES}")

####################################################
# Генерация
####################################################
message("******** Generating started ***********")
# Из найденных исходников выставляем генерацию бинарника
add_executable(${EXECUTABLE_NAME} ${ALL_HEADERS} ${ALL_SOURCES})

# Зависимости сборки
add_dependencies (${EXECUTABLE_NAME} ${LIBS_DEPENCY_NAMES})

# Библиотеки, с которыми будем ликовать
target_link_libraries (${EXECUTABLE_NAME} ${LIBS_LINK_NAMES} ${SYSTEM_LIBS})
