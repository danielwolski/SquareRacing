# SquareRacing-WIP-
Simple racing game using SDL


//install sdl

sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev

//run

gcc -o racing_game main.c `sdl2-config --cflags --libs` -lSDL2_image
