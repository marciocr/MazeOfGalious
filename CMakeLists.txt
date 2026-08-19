cmake_minimum_required(VERSION 3.21)
project(MazeOfGalious)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(MOG_VERSION "0.63")

set(SHARED_SOURCES
	sources/MOGtypes.h
	sources/bitmaps.cpp
	sources/bitmaps.h
	sources/debug.cpp
	sources/debug.h
	sources/debug-report.cpp
	sources/demons.cpp
	sources/drawing.cpp
	sources/drawing.h
	sources/filehandling.cpp
	sources/filehandling.h
	sources/gameaux.cpp
	sources/gamecycle.cpp
	sources/gameinteractions.cpp
	sources/gameobjects.cpp
	sources/gametiles.cpp
	sources/ingamecycle.cpp
	sources/keyboard.cpp
	sources/line_clip.cpp
	sources/line_clip.h
	sources/loadpcx.cpp
	sources/loadpcx.h
	sources/main.cpp
	sources/mog.h
	sources/object.h
	sources/passwords.cpp
	sources/path.cpp
	sources/render.cpp
	sources/resource.h
	sources/roomchange.cpp
	sources/shrines.cpp
	sources/sound.cpp
	sources/sound.h
	sources/sprites.h
	sources/tiles.h
)

set(SHARED_LIBS
)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(SDL_sound REQUIRED)

include_directories(.)

if (SDL_FOUND)
	include_directories(${SDL_INCLUDE_DIRS} ${SDL_INCLUDE_DIR})
	set(SHARED_SOURCES ${SHARED_SOURCES} ${SDL_SOURCES})
	set(SHARED_LIBS ${SHARED_LIBS} ${SDL_LIBRARIES})
endif()

if (SDL_IMAGE_FOUND)
	include_directories(${SDL_IMAGE_INCLUDE_DIRS})
	set(SHARED_SOURCES ${SHARED_SOURCES} ${SDL_IMAGE_SOURCES})
	set(SHARED_LIBS ${SHARED_LIBS} ${SDL_IMAGE_LIBRARIES})
endif()

if (SDL_MIXER_FOUND)
	include_directories(${SDL_MIXER_INCLUDE_DIRS})
	set(SHARED_SOURCES ${SHARED_SOURCES} ${SDL_MIXER_SOURCES})
	set(SHARED_LIBS ${SHARED_LIBS} ${SDL_MIXER_LIBRARIES})
endif()

if (SDL_SOUND_FOUND)
	include_directories(${SDL_SOUND_INCLUDE_DIR})
	set(SHARED_SOURCES ${SHARED_SOURCES} ${SDL_SOUND_SOURCES})
	set(SHARED_LIBS ${SHARED_LIBS} ${SDL_SOUND_LIBRARIES})
endif()

add_executable(MazeOfGalious ${SHARED_SOURCES})
target_compile_definitions(MazeOfGalious PRIVATE MOG_VERSION_STRING="Maze of Galious v${MOG_VERSION}")

target_link_libraries(MazeOfGalious ${SHARED_LIBS})

source_group("shared" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/sources/.+")
