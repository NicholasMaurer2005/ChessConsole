cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(ChessConsole CXX)

add_executable(ChessConsole
        ChessConsole/BitBoard.cpp  ChessConsole/Move.cpp
        ChessConsole/PreGen.cpp    ChessConsole/Engine.cpp
        ChessConsole/MoveGen.cpp   ChessConsole/Random.cpp
        ChessConsole/Main.cpp      ChessConsole/MoveList.cpp
        ChessConsole/State.cpp
)
target_compile_features(ChessConsole PUBLIC cxx_std_20)
set_property(TARGET ChessConsole PROPERTY CXX_EXTENSIONS OFF)
