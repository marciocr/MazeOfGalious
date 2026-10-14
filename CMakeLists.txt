cmake_minimum_required(VERSION 3.21)
project(MazeOfGalious)

set(MOG_VERSION "0.7")

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
	sources/keyboardstate.cpp
	sources/keyboardstate.h
	sources/line_clip.cpp
	sources/line_clip.h
	sources/loadpcx.cpp
	sources/loadpcx.h
	sources/main.cpp
	sources/mog.h
	sources/object.h
	sources/options.cpp
	sources/passwords.cpp
	sources/path.cpp
	sources/postfx.cpp
	sources/postfx.h
	sources/render.cpp
	sources/roomchange.cpp
	sources/shrines.cpp
	sources/sound.cpp
	sources/sound.h
	sources/sprites.h
	sources/tiles.h
	sources/video.cpp
	sources/video.h
)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
pkg_check_modules(SDL3_MIXER REQUIRED IMPORTED_TARGET sdl3-mixer)

add_executable(MazeOfGalious ${SHARED_SOURCES})
target_compile_definitions(MazeOfGalious PRIVATE MOG_VERSION_STRING="Maze of Galious v${MOG_VERSION}")

target_link_libraries(MazeOfGalious PkgConfig::SDL3 PkgConfig::SDL3_MIXER OpenGL::GL)

source_group("shared" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/sources/.+")
