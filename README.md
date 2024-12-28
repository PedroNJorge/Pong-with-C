# Pong-with-C
Recreation of the classic Pong from 1972 using C. The main libraries used were SDL3 and SDL3_ttf.
--> The SDL3 library is used to handle the rendering of the graphics.
--> SDL3_ttf complements it with the ability to write text onto the screen with your preferred font (i used joystix).

# Compiling
To compile i used gcc:
  gcc -o pong pong.c -lm -lSDL3_ttf `pkg-config sld3 --cflags --libs`
