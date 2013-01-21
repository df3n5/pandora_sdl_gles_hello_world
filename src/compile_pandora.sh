$CC -std=c99 eglport.c main.c -o egl_hello -lSDL -lGLES_CM -lEGL -lX11 -lSDL -lSDL_image -DHAVE_GLES
