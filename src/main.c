#if !defined(HAVE_GLES)
#include <GL/glew.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#include "eglport.h"
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define COG_MAX_FILE_BUF 2500

SDL_Surface* load_image(const char* filename)
{
    //Loads an image and returns an SDL_Surface.
    SDL_Surface* tempsurface;
    SDL_Surface* result;

    tempsurface = IMG_Load(filename);
    if(!tempsurface)
    {
        fprintf(stderr, "Cannot load image file <%s> : <%s>", filename, SDL_GetError());
        return 0;
    }

    if((result = SDL_DisplayFormatAlpha(tempsurface))==NULL)
    {
        perror(SDL_GetError());
    }
    SDL_FreeSurface(tempsurface);

    return result;
}

GLuint upload_surface(SDL_Surface* image)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);

    int w = image->w;
    int h = image->h;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif
    /* Create the target alpha surface with correct color component ordering */
    SDL_Surface* alphaimage = SDL_CreateRGBSurface(SDL_SWSURFACE,
            image->w,image->h,32,
            rmask,gmask,bmask,amask);
    if(!alphaimage)
    {
        fprintf(stderr, "upload_surface : RGB surface creation failed.");
    }
    // Set up so that colorkey pixels become transparent :
    Uint32 colorkey = SDL_MapRGBA( alphaimage->format, rmask, gmask, bmask, amask );
    SDL_FillRect( alphaimage, 0, colorkey );
    colorkey = SDL_MapRGBA( image->format, rmask, gmask, bmask, amask);
    SDL_SetColorKey( image, SDL_SRCCOLORKEY, colorkey );
    SDL_Rect area;
    SDL_SetAlpha(image, 0, amask); //http://www.gamedev.net/topic/518525-opengl--sdl--transparent-image-make-textures/
    // Copy the surface into the GL texture image :
    area.x = 0;
    area.y = 0;
    area.w = image->w;
    area.h = image->h;
    SDL_BlitSurface( image, &area, alphaimage, &area );
    // Create an OpenGL texture for the image
    GLuint textureID;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    /* Prepare the filtering of the texture image */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    /* Map the alpha surface to the texture */
    glTexImage2D( GL_TEXTURE_2D,
            0,
            GL_RGBA,
            w,
            h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            alphaimage->pixels );
    SDL_FreeSurface(alphaimage);
    return textureID;
}

GLuint load_texture(char* filename)
{
    SDL_Surface* image = load_image(filename);
    GLuint texture = upload_surface(image);
    SDL_FreeSurface(image);
    return texture;
}

void init_draw(void) {
    glClearColor(0.3f,0.3f,0.5f,0.0f);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
}

void draw(void) {
#if defined (HAVE_GLES)
    GLfloat vertices[] = {1,0,0, 0,1,0, -1,0,0};
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
#else
    glClear( GL_COLOR_BUFFER_BIT );
    glBegin(GL_TRIANGLES);

    glVertex3f(1,0,0);
    glVertex3f(0,1,0);
    glVertex3f(-1,0,0);

    glEnd();
#endif
}

static void draw_simple_quad(float X, float Y, float Z, float W, float H)
{
	GLfloat box[] = {X,Y + H,Z,  X + W,Y + H,Z,     X + W, Y, Z,   X,Y,Z};
	glEnableClientState(GL_VERTEX_ARRAY);
     GLfloat vertices[] = {X, Y, 0, //bottom left corner
         X,  Y+H, 0, //top left corner
         X+W,  Y+H, 0, //top right corner
         X+W, Y, 0}; // bottom right rocner
     GLubyte indices[] = {0,1,2, // first triangle (bottom left - top left - top right)
     0,2,3}; // second triangle (bottom left - top right - bottom right)
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
	glDisableClientState(GL_VERTEX_ARRAY);
}

static void DrawSprite(GLuint sprite, float X, float Y, float Z, float W, float H)
{
    glClear( GL_COLOR_BUFFER_BIT );
    glLoadIdentity();

    GLfloat vertices[] = {
        X,  Y+H, 0, //top left corner
        X+W,  Y+H, 0, //top right corner
        X+W, Y, 0, // bottom right rocner
        X, Y, 0}; //bottom left corner
	GLfloat tex[] = {1,0, 0,0, 0,1, 1,1};
    GLubyte indices[] = {3,0,1, // first triangle (bottom left - top left - top right)
        3,1,2}; // second triangle (bottom left - top right - bottom right)
 
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 0, tex);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
 
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

int main(void) {
    // other stuff here

    SDL_Init( SDL_INIT_VIDEO );
#if defined(HAVE_GLES)
    if(EGL_Open()) {
        exit(1);
    }
#endif


    SDL_Surface* screen;
#if !defined(HAVE_GLES)
    screen = SDL_SetVideoMode( 800, 600, 0, SDL_HWSURFACE | SDL_OPENGL | SDL_FULLSCREEN);
    if(screen==0) {
        printf("%s",SDL_GetError());
    }
#else
    screen = SDL_SetVideoMode( 800, 480, 0, SDL_SWSURFACE | SDL_FULLSCREEN );
    EGL_Init();
#endif

    init_draw();
    // some more initialisations
    GLuint sprite = load_texture("test.png");

	glBindTexture(GL_TEXTURE_2D,sprite);

    int quit = 0;
    while(!quit) {
        SDL_Event event;
        if( SDL_PollEvent( &event ) ) { 
            if( event.type == SDL_KEYDOWN ) {
                switch( event.key.keysym.sym ) { 
                    case SDLK_q: 
                        quit = 1;
                        break; 
                } 
            } else if( event.type == SDL_QUIT ) { 
                quit = 1;
            } 
        } 
        DrawSprite(sprite, -0.5f, -0.5f, 0, 1,1);
//        draw_simple_quad(-0.5f, -0.5f, 0, 1,1);
#if !defined(HAVE_GLES)
        SDL_GL_SwapBuffers();
#else
        EGL_SwapBuffers();
#endif
    }

    // probably some more clean-ups here
#if defined(HAVE_GLES)
    EGL_Close();
#endif
    SDL_Quit();

    return 0;
}
