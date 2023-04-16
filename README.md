# SquareRacing-WIP-
Simple racing game using SDL

This program is a simple racing game written in C, using the SDL2 and SDL2_image libraries. The program loads a racing track texture from the "track.png" file and allows the player to move around the track using the W, A, S, D keys.


__________________________
## SDL2 installation

sudo apt-get install libsdl2-dev

sudo apt-get install libsdl2-image-dev


## Run


gcc -o racing_game main.c \`sdl2-config --cflags --libs\` -lSDL2_image

./racing_game
