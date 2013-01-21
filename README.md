pandora_sdl_gles_hello_world
============================

Hello World for SDL and GLES with pandora. 
Shows a textured quad on the screen.

Compiling
=========
Firstly you will need to setup a cross compiler to compile for pandora: http://www.gp32x.com/board/index.php?/topic/58490-yactfeau/page__view__findpost__p__940518
Then you should be able to use the ``./compile_pandora.sh`` bash script to compile in linux.

The ``./compile_linux_host.sh`` script is also provided which lets you also compile on linux with SDL-1.2, SDL_Image and GLEW installed.

Running
=======
You can copy the cross compiled executable over to the pandora's SD card along with the ``test.png`` provided. 

Notes / Caveats
===============
Texture image should be a power of two, this confused / enraged me for a long time.
