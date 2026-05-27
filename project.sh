#!/bin/sh

mode="$1";

if [[ $mode = "cl" ]]; then
    rm -rf ./build/ || printf "Cleaned build files";
elif [[ $mode = "bd" ]]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE:STRING=Debug && cmake --build build;
    mv ./build/compile_commands.json .;
elif [[ $mode = "br" ]]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE:STRING=Release && cmake --build build;
    mv ./build/compile_commands.json .;
    strip ./build/tbasic -o ./tbasic;
elif [[ $mode = "bp" ]]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo && cmake --build build;
    mv ./build/compile_commands.json .;
elif [[ $mode = "dd" ]]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE:STRING=Debug && cmake --build build;
    mv ./build/compile_commands.json .;
elif [[ $mode = "dr" ]]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE:STRING=Release -DCREATE_TBASIC_SHARED && cmake --build build;
    mv ./build/compile_commands.json .;
else
    printf "Usage:\n\tbd: build debug\n\tbr: build release\n\tbp: build profile\n\tdd: build shared lib & demo (debug)\n\tdr: build shared lib & demo (release)\n";
fi
