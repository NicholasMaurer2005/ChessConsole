cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(ChessConsole CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ChessConsole
        ChessConsole/BitBoard.cpp  ChessConsole/Move.cpp
        ChessConsole/PreGen.cpp    ChessConsole/Engine.cpp
        ChessConsole/MoveGen.cpp   ChessConsole/Random.cpp
        ChessConsole/Main.cpp      ChessConsole/MoveList.cpp
        ChessConsole/State.cpp
)
