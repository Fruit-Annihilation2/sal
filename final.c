//salvador lopez
//final.c
//cs335 Spring 2014
//
//program: lab2_rainforest.c
//author:  Gordon Griesel
//date:    2014
//
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "ppm.h"
#include "fonts.h"
#include "move_fruit.h"
#include "http_get.h"
/*#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>*/
#define USE_SOUND
/*#define USERAGENT "HTMLGET 1.0"
#define PAGE "/"
#define PORT 80
#define HOST "coding.debuntu.org"
 */
#ifdef USE_SOUND
#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"
#endif //USE_SOUND

//defined types
typedef double Flt;
//typedef double Vec[3];
typedef Flt Matrix[4][4];

//macros
#define rnd() (((double)rand())/(double)RAND_MAX)
#define random(a) (rand()%a)

#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
                             (c)[1]=(a)[1]-(b)[1]; \
(c)[2]=(a)[2]-(b)[2]
//constants
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define ALPHA 1
float offsetBackground;
//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
void check_keys(XEvent *e);
void init();
void init_sounds(void);
void physics(void);
void render(void);
void mouse_click(int, int, int, int);

/*char *get_ip(char *);
  int create_tcp_socket();
  void usage();
  void sendData();
  char *build_get_query(char *, char *);
 */
//-----------------------------------------------------------------------------
//Setup timers
double physicsRate = 1.0 / 30.0;
const double oobillion = 1.0 / 1e9;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double timeSpan=0.0;
double timeDiff(struct timespec *start, struct timespec *end) {
  return (double)(end->tv_sec - start->tv_sec ) +
    (double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source) {
  memcpy(dest, source, sizeof(struct timespec));
}

//-----------------------------------------------------------------------------
int lbutton=0;
int rbutton=0;
#define MAXBUTTONS 8
typedef struct t_button {
  Rect r;
  char text[32];
  int over;
  int down;
  int click;
  float color[3];
  float dcolor[3];
  unsigned int text_color;
} Button;
Button button[MAXBUTTONS];
int nbuttons=0;



int done=0;
//int xres=800, yres=600;
int myScore=0;
int missedApples=0;
int activeFruit=0;
int nextLevel=0;
int showNextLevel=0;
int totalScore=0;
int levelnumber=1;

Ppmimage *apple=NULL;
Ppmimage *backgroundImage=NULL;
Ppmimage *forestTransImage=NULL;
Ppmimage *umbrellaImage=NULL;
Ppmimage *basket=NULL;
Ppmimage *menu=NULL;
Ppmimage *gameover=NULL;
Ppmimage *bomb=NULL;
Ppmimage *level2=NULL;

GLuint bigfootTexture;
GLuint silhouetteTexture;
GLuint forestTexture;
GLuint forestTransTexture;
GLuint umbrellaTexture;
GLuint baskettexture;
GLuint silhouetteTextureFruit;
GLuint silhouetteTextureMenu;
GLuint menuTexture;
GLuint gameOverTexture;
GLuint silhouetteTextureGameOver;
GLuint bombTexture;
GLuint silhouetteTextureBomb;
GLuint level2Texture;
GLuint silhouetteTextureLevel2;

int show_gameover=0;
int show_bigfoot=0;
int show_menu=1;
int forest=1;
int silhouette=1;
int trees=1;
int show_rain=0;
#ifdef USE_SOUND
int play_sounds = 0;
#endif //USE_SOUND
//
typedef struct t_raindrop {
  int type;
  int linewidth;
  int sound;
  Vec pos;
  Vec lastpos;
  Vec vel;
  Vec maxvel;
  Vec force;
  Flt lower_boundry;
  float length;
  float color[4];
  struct t_raindrop *prev;
  struct t_raindrop *next;
} Raindrop;
Raindrop *ihead=NULL;
int ndrops=1;
int totrain=0;
int maxrain=0;
void delete_rain(Raindrop *node);
void cleanup_raindrops(void);
//
#define USE_UMBRELLA
#ifdef USE_UMBRELLA
#define UMBRELLA_FLAT  0
#define UMBRELLA_ROUND 1
typedef struct t_umbrella {
  int shape;
  Vec pos;
  Vec lastpos;
  float width;
  float width2;
  float radius;
} Umbrella;
Umbrella umbrella;
int show_umbrella=0;
int deflection=0;
#endif //USE_UMBRELLA


int main(void)
{
  logOpen();
  initXWindows();
  init_opengl();
  init();
  init_sounds();
  clock_gettime(CLOCK_REALTIME, &timePause);
  clock_gettime(CLOCK_REALTIME, &timeStart);
  while(!done) {
    while(XPending(dpy)) {
      XEvent e;
      XNextEvent(dpy, &e);
      check_resize(&e);
      check_mouse(&e);
      check_keys(&e);
    }
    clock_gettime(CLOCK_REALTIME, &timeCurrent);
    timeSpan = timeDiff(&timeStart, &timeCurrent);
    timeCopy(&timeStart, &timeCurrent);
    physicsCountdown += timeSpan;
    while(physicsCountdown >= physicsRate) {
      physics();
      physicsCountdown -= physicsRate;
    }
    render();
    glXSwapBuffers(dpy, win);
  }
  cleanupXWindows();
  //cleanup_fonts();
#ifdef USE_SOUND
  //fmod_cleanup();
#endif //USE_SOUND
  logClose();
  return 0;
}
void cleanupXWindows(void)
{
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}

void set_title(void)
{
  //Set the window title bar.
  XMapWindow(dpy, win);
  XStoreName(dpy, win, "CS335 - apple catcher");
}

void setup_screen_res(const int w, const int h)
{
  xres = w;
  yres = h;
}

void initXWindows(void)
{
  Window root;
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;

  setup_screen_res(640, 480);
  dpy = XOpenDisplay(NULL);
  if(dpy == NULL) {
    printf("\n\tcannot connect to X server\n\n");
    exit(EXIT_FAILURE);
  }
  root = DefaultRootWindow(dpy);
  vi = glXChooseVisual(dpy, 0, att);
  if(vi == NULL) {
    printf("\n\tno appropriate visual found\n\n");
    exit(EXIT_FAILURE);
  } 
  //else {
  //  // %p creates hexadecimal output like in glxinfo
  //  printf("\n\tvisual %p selected\n", (void *)vi->visualid);
  //}
  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    StructureNotifyMask | SubstructureNotifyMask;
  win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0,
      vi->depth, InputOutput, vi->visual,
      CWColormap | CWEventMask, &swa);
  set_title();
  glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);
}

void reshape_window(int width, int height)
{
  //window has been resized.
  setup_screen_res(width, height);
  //
  glViewport(0, 0, (GLint)width, (GLint)height);
  glMatrixMode(GL_PROJECTION); glLoadIdentity();
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glOrtho(0, xres, 0, yres, -1, 1);
  set_title();
}

unsigned char *buildAlphaData(Ppmimage *img)
{
  //add 4th component to RGB stream...
  int i;
  int a,b,c;
  unsigned char *newdata, *ptr;
  unsigned char *data = (unsigned char *)img->data;
  newdata = (unsigned char *)malloc(img->width * img->height * 4);
  ptr = newdata;
  for (i=0; i<img->width * img->height * 3; i+=3) {
    a = *(data+0);
    b = *(data+1);
    c = *(data+2);
    *(ptr+0) = a;
    *(ptr+1) = b;
    *(ptr+2) = c;
    //
    //get the alpha value
    //
    //original code
    //get largest color component...
    //*(ptr+3) = (unsigned char)((
    //    (int)*(ptr+0) +
    //    (int)*(ptr+1) +
    //    (int)*(ptr+2)) / 3);
    //d = a;
    //if (b >= a && b >= c) d = b;
    //if (c >= a && c >= b) d = c;
    //*(ptr+3) = d;
    //
    //new code, suggested by Chris Smith, Fall 2013
    *(ptr+3) = (a|b|c);
    //
    ptr += 4;
    data += 3;
  }
  return newdata;
}

void init_opengl(void)
{
  //OpenGL initialization
  glViewport(0, 0, xres, yres);
  //Initialize matrices
  glMatrixMode(GL_PROJECTION); glLoadIdentity();
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  //This sets 2D mode (no perspective)
  glOrtho(0, xres, 0, yres, -1, 1);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  glDisable(GL_CULL_FACE);

  //Clear the screen
  glClearColor(1.0, 1.0, 1.0, 1.0);
  //glClear(GL_COLOR_BUFFER_BIT);
  //Do this to allow fonts
  glEnable(GL_TEXTURE_2D);
  //initialize_fonts();
  //
  //load the images file into a ppm structure.
  //
  apple     = ppm6GetImage("./images/bigfoot.ppm");
  backgroundImage      = ppm6GetImage("./images/duckhunt.ppm");
  forestTransImage = ppm6GetImage("./images/forestTrans.ppm");
  umbrellaImage    = ppm6GetImage("./images/apple.ppm");
  basket    = ppm6GetImage("./images/apple.ppm");
  menu      = ppm6GetImage("./images/menu.ppm");
  gameover= ppm6GetImage("./images/gameover.ppm");
  bomb= ppm6GetImage("./images/bomb.ppm");
  level2=ppm6GetImage("./images/nextlevel.ppm");
  //
  //create opengl texture elements

  glGenTextures(1, &bigfootTexture);
  glGenTextures(1, &silhouetteTexture);
  glGenTextures(1, &forestTexture);
  glGenTextures(1, &umbrellaTexture);
  glGenTextures(1,&baskettexture);
  glGenTextures(1,&silhouetteTextureFruit);
  glGenTextures(1,&silhouetteTextureMenu);
  glGenTextures(1, &menuTexture);
  glGenTextures(1,&gameOverTexture);
  glGenTextures(1,&silhouetteTextureGameOver);
  glGenTextures(1,&bombTexture);
  glGenTextures(1,&silhouetteTextureBomb);
  glGenTextures(1,&level2Texture);
  glGenTextures(1,&silhouetteTextureLevel2);

int nextw = level2->width;
  int nexth = level2->height;
  //
  glBindTexture(GL_TEXTURE_2D, level2Texture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, nextw, nexth, 0,
      GL_RGB, GL_UNSIGNED_BYTE, level2->data);
  //-------------------------------------------------------------------------
  //
  //silhouette
  //this is similar to a sprite graphic
  ///
  glBindTexture(GL_TEXTURE_2D, silhouetteTextureLevel2);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  unsigned char *silhouetteDataNextLevel = buildAlphaData(level2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nextw, nexth, 0,GL_RGBA, GL_UNSIGNED_BYTE, silhouetteDataNextLevel);
  free(silhouetteDataNextLevel);





//bomb image
 int bombw = bomb->width;
  int bombh = bomb->height;
  //
  glBindTexture(GL_TEXTURE_2D, bombTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, bombw, bombh, 0,
      GL_RGB, GL_UNSIGNED_BYTE, bomb->data);
  //-------------------------------------------------------------------------
  //
  //silhouette
  //this is similar to a sprite graphic
  ///
  glBindTexture(GL_TEXTURE_2D, silhouetteTextureBomb);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  unsigned char *silhouetteDatabomb = buildAlphaData(bomb);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bombw, bombh, 0,GL_RGBA, GL_UNSIGNED_BYTE, silhouetteDatabomb);
  free(silhouetteDatabomb);




//game over sign
 int gow = gameover->width;
  int goh = gameover->height;
  //
  glBindTexture(GL_TEXTURE_2D, gameOverTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, gow, goh, 0,
      GL_RGB, GL_UNSIGNED_BYTE, gameover->data);
  //-------------------------------------------------------------------------
  //
  //silhouette
  //this is similar to a sprite graphic
  ///
  glBindTexture(GL_TEXTURE_2D, silhouetteTextureGameOver);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  unsigned char *silhouetteDatago = buildAlphaData(gameover);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gow, goh, 0,GL_RGBA, GL_UNSIGNED_BYTE, silhouetteDatago);
  free(silhouetteDatago);





  //glGenTextures(1,&silhouetteTextureMenu);
  //-------------------------------------------------------------------------
  //menue option
  int mw = menu->width;
  int mh = menu->height;
  //
  glBindTexture(GL_TEXTURE_2D, menuTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, mw, mh, 0,
      GL_RGB, GL_UNSIGNED_BYTE, menu->data);
  //-------------------------------------------------------------------------
  //
  //silhouette
  //this is similar to a sprite graphic
  ///
  glBindTexture(GL_TEXTURE_2D, silhouetteTextureMenu);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  unsigned char *silhouetteDataM = buildAlphaData(menu);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mw, mh, 0,GL_RGBA, GL_UNSIGNED_BYTE, silhouetteDataM);
  free(silhouetteDataM);
  //new basket photo
  //
  int fw = basket->width;
  int fh = basket->height;
  //
  glBindTexture(GL_TEXTURE_2D, baskettexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, fw, fh, 0,
      GL_RGB, GL_UNSIGNED_BYTE, basket->data);
  //-------------------------------------------------------------------------
  //
  //silhouette
  //this is similar to a sprite graphic
  //
  glBindTexture(GL_TEXTURE_2D, silhouetteTextureFruit);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  unsigned char *silhouetteDataF = buildAlphaData(basket);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fw, fh, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, silhouetteDataF);
  free(silhouetteDataF);

  /*------------------------------------------------------*/


  //-------------------------------------------------------------------------
  //bigfoot
  //
  int w = apple->width;
  int h = apple->height;
  //
  glBindTexture(GL_TEXTURE_2D, bigfootTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
      GL_RGB, GL_UNSIGNED_BYTE, apple->data);
  //-------------------------------------------------------------------------
  //
  //silhouette
  //this is similar to a sprite graphic
  //
  glBindTexture(GL_TEXTURE_2D, silhouetteTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  unsigned char *silhouetteData = buildAlphaData(apple); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
  free(silhouetteData);
  //-------------------------------------------------------------------------
  //
  //umbrella
  //
  glBindTexture(GL_TEXTURE_2D, umbrellaTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  silhouetteData = buildAlphaData(umbrellaImage); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
  free(silhouetteData);
  //-------------------------------------------------------------------------
  //
  //forest
  glBindTexture(GL_TEXTURE_2D, forestTexture);
  glTranslatef(offsetBackground,0.0f,0.0f);//work on background movement 
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
      backgroundImage->width, backgroundImage->height,
      0, GL_RGB, GL_UNSIGNED_BYTE, backgroundImage->data);
  //-------------------------------------------------------------------------
  //
  //forest transparent part
  //
  glBindTexture(GL_TEXTURE_2D, forestTransTexture);
  //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  //
  //must build a new set of data...
  w = forestTransImage->width;
  h = forestTransImage->height;
  unsigned char *ftData = buildAlphaData(forestTransImage); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, ftData);
  free(ftData);
  //-------------------------------------------------------------------------
}

void check_resize(XEvent *e)
{
  //The ConfigureNotify is sent by the
  //server if the window is resized.
  if (e->type != ConfigureNotify)
    return;
  XConfigureEvent xce = e->xconfigure;
  if (xce.width != xres || xce.height != yres) {
    //Window size did change.
    reshape_window(xce.width, xce.height);
  }
}

void init_sounds(void)
{
#ifdef USE_SOUND
  //FMOD_RESULT result;
  /*
     if (fmod_init()) {
     printf("ERROR - fmod_init()\n\n");
     return;
     }
     if (fmod_createsound("./sounds/tick.wav", 0)) {
     printf("ERROR - fmod_createsound()\n\n");
     return;
     }
     if (fmod_createsound("./sounds/drip.wav", 1)) {
     printf("ERROR - fmod_createsound()\n\n");
     return;
     }
   */
  //fmod_setmode(0,FMOD_LOOP_OFF);
  //fmod_playsound(0);
  //fmod_systemupdate();
#endif //USE_SOUND
}

void init() {
#ifdef USE_UMBRELLA
  umbrella.pos[0] = 220.0;
  umbrella.pos[1] = (double)(yres-200);
  VecCopy(umbrella.pos, umbrella.lastpos);
  umbrella.width = 200.0;
  umbrella.width2 = umbrella.width * 0.5;
  umbrella.radius = (float)umbrella.width2;
  umbrella.shape = UMBRELLA_FLAT;
#endif //USE_UMBRELLA
  //MakeVector(-150.0,180.0,0.0, bigfoot.pos);  
  MakeVector(-2.0,60.0,0.0,bigfoot.pos);
  MakeVector(6.0,0.0,0.0, bigfoot.vel);
  MakeVector(200.0,500.0,0.0,bigfoot2.pos);
  MakeVector(0.0,0.0,0.0,bigfoot2.vel);
  MakeVector(400.0,800.0,0.0,bigfoot3.pos);
  MakeVector(0.0,0.0,0.0,bigfoot3.vel);
  MakeVector(500.0,700.0,0.0,bomb1.pos);
  MakeVector(0.0,0.0,0.0,bomb1.vel);
  MakeVector(300.0,300.0,0.0,bigfoot4.pos);
  MakeVector(6.0,0.0,0.0,bigfoot4.vel);
  MakeVector(300.0,300.0,0.0,gameoversign.pos);
  MakeVector(300.0,300.0,0.0,nextlevelsign.pos);
  MakeVector(6.0,0.0,0.0,nextlevelsign.vel);
}
void check_mouse(XEvent *e)
{
  //Did the mouse move?
  //Was a mouse button clicked?
  static int savex = 0;
  static int savey = 0;
  //
  if (e->type == ButtonRelease) {
    return;
  }
  if (e->type == ButtonPress) {
    if (e->xbutton.button==1) {
      //Left button is down
      printf("hello world");
    }
    if (e->xbutton.button==3) {
      //Right button is down
      printf("hello world");
    }
  }
  if (savex != e->xbutton.x || savey != e->xbutton.y) {
    //Mouse moved
    savex = e->xbutton.x;
    savey = e->xbutton.y;
  }


}

void check_keys(XEvent *e)
{
  //keyboard input?
  static int shift=0;
  int key = XLookupKeysym(&e->xkey, 0);
  if (e->type == KeyRelease) {
    if (key == XK_Shift_L || key == XK_Shift_R)
      shift=0;
    return;
  }
  if (e->type == KeyPress) {
    if (key == XK_Shift_L || key == XK_Shift_R) {
      shift=1;
      return;
    }
  } else {
    return;
  }
  //
  switch(key) {
    case XK_b:
      levelnumber=1;
      totalScore=0;
      nextLevel=0;
      show_bigfoot ^= 1;
      missedApples=0;
      show_gameover=0;
      bigfoot2.pos[1]=500;
      bigfoot3.pos[1]=700;
      bomb1.pos[1]=800;
      myScore=0;
      show_menu=0;
      if (show_bigfoot) {
        bigfoot.pos[0] = 200; //-250.0;
      }
      break;
    case XK_d:
      deflection ^= 1;
      break;
    case XK_f:
      //forest ^= 1;
      activeFruit=1;
      //forest=1;
      break;
    case XK_s:
      silhouette ^= 1;
      printf("silhouette: %i\n",silhouette);
      break;
    case XK_t:
      //trees ^= 1;
      sendData();
      break;
    case XK_u:
     // show_umbrella ^= 1;
      myScore=0;
      show_bigfoot=1;
      showNextLevel=0;
      break;
    case XK_p:
      umbrella.shape ^= 1;
      break;
    case XK_r:
      //show_rain ^= 1;
      break;
    case XK_Left:
      bigfoot.pos[0] -= 40.0;
      if(bigfoot.vel[0]>0){
        bigfoot.vel[0] = -bigfoot.vel[0];
      }
      break;
    case XK_Right:
      bigfoot.pos[0] += 40.0;
      if(bigfoot.vel[0]<0){
        bigfoot.vel[0] = -bigfoot.vel[0];
      }
      offsetBackground+=.75;
      break;
    case XK_Up:
      //bigfoot.pos[1] += 10.0;
      break;
    case XK_Down:
      break;
    case XK_equal:
      if (++ndrops > 60)
        ndrops=60;
      break;
    case XK_minus:
      if (--ndrops < 0)
        ndrops = 0;
      break;
    case XK_n:
      play_sounds ^= 1;
      break;
    case XK_w:
      if (shift) {
        //shrink the umbrella
        umbrella.width *= (1.0 / 1.05);
      } else {
        //enlarge the umbrella
        umbrella.width *= 1.05;
      }
      //half the width
      umbrella.width2 = umbrella.width * 0.5;
      umbrella.radius = (float)umbrella.width2;
      break;
    case XK_Escape:
      printf("%d\n",myScore);
      done=1;
      break;
  }
}

void normalize(Vec vec)
{
  Flt xlen = vec[0];
  Flt ylen = vec[1];
  Flt zlen = vec[2];
  Flt len = xlen*xlen + ylen*ylen + zlen*zlen;
  if (len == 0.0) {
    MakeVector(0.0,0.0,1.0,vec);
    return;
  }
  len = 1.0 / sqrt(len);
  vec[0] = xlen * len;
  vec[1] = ylen * len;
  vec[2] = zlen * len;
}

void cleanup_raindrops(void)
{
  Raindrop *s;
  while(ihead) {
    s = ihead->next;
    free(ihead);
    ihead = s;
  }
  ihead=NULL;
}

void delete_rain(Raindrop *node)
{
  //remove a node from linked list
  //Log("delete_rain()...\n");
  if (node->prev == NULL) {
    if (node->next == NULL) {
      //Log("only 1 item in list.\n");
      ihead = NULL;
    } else {
      //Log("at beginning of list.\n");
      node->next->prev = NULL;
      ihead = node->next;
    }
  } else {
    if (node->next == NULL) {
      //Log("at end of list.\n");
      node->prev->next = NULL;
    } else {
      //Log("in middle of list.\n");
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }
  }
  free(node);
  node = NULL;

}
//use this to move characters
//add keys to move character
/***********************************************************/
void create_raindrop(const int n)
{
  //create new rain drops...
  int i;
  for (i=0; i<n; i++) {
    Raindrop *node = (Raindrop *)malloc(sizeof(Raindrop));
    if (node == NULL) {
      Log("error allocating node.\n");
      exit(EXIT_FAILURE);
    }
    node->prev = NULL;
    node->next = NULL;
    node->sound=0;
    node->pos[0] = rnd() * (float)xres;
    node->pos[1] = rnd() * 100.0f + (float)yres;
    VecCopy(node->pos, node->lastpos);
    node->vel[0] = 
      node->vel[1] = 0.0f;
    node->color[0] = rnd() * 0.05f + 0.95f;
    node->color[1] = rnd() * 0.05f + 0.95f;
    node->color[2] = rnd() * 0.05f + 0.95f;
    node->color[3] = rnd() * 0.2f + 0.3f; //alpha
    node->linewidth = random(8)+1;
    //larger linewidth = faster speed
    node->maxvel[1] = (float)(node->linewidth*16);
    node->length = node->maxvel[1] * 0.2f + rnd();
    //
    node->lower_boundry = rnd() * 100.0;
    //
    //put raindrop into linked list
    node->next = ihead;
    if (ihead != NULL)
      ihead->prev = node;
    ihead = node;
    ++totrain;
  }
}

void check_raindrops()
{
  if (random(100) < 50) {
    create_raindrop(ndrops);
  }
  //
  //move rain droplets
  Raindrop *node = ihead;
  while(node) {
    //force is toward the ground
    node->vel[1] += gravity;
    VecCopy(node->pos, node->lastpos);
    if (node->pos[1] > node->lower_boundry) {
      node->pos[0] += node->vel[0] * timeslice;
      node->pos[1] += node->vel[1] * timeslice;
    }
    if (fabs(node->vel[1]) > node->maxvel[1])
      node->vel[1] *= 0.96;
    node->vel[0] *= 0.999;
    //
    node = node->next;
  }
  //}
  //
  //check rain droplets
int n=0;
node = ihead;
while(node) {
  n++;
#ifdef USE_SOUND
  if (node->pos[1] < 0.0f) {
    //raindrop hit ground
    if (!node->sound && play_sounds) {
      //small chance that a sound will play
      int r = random(50);
      if (r==1)
        //fmod_playsound(0);
        //if (r==2)
        //  fmod_playsound(1);
        //sound plays once per raindrop
        node->sound=1;
    }
  }
#endif //USE_SOUND
#ifdef USE_UMBRELLA
  //collision detection for raindrop on umbrella
  if (show_umbrella) {
    if (umbrella.shape == UMBRELLA_FLAT) {
      if (node->pos[0] >= (umbrella.pos[0] - umbrella.width2) &&
          node->pos[0] <= (umbrella.pos[0] + umbrella.width2)) {
        if (node->lastpos[1] > umbrella.lastpos[1] ||
            node->lastpos[1] > umbrella.pos[1]) {
          if (node->pos[1] <= umbrella.pos[1] ||
              node->pos[1] <= umbrella.lastpos[1]) {
            if (node->linewidth > 1) {
              Raindrop *savenode = node->next;
              delete_rain(node);
              node = savenode;
              continue;
            }
          }
        }
      }
    }
    if (umbrella.shape == UMBRELLA_ROUND) {
      float d0 = node->pos[0] - umbrella.pos[0];
      float d1 = node->pos[1] - umbrella.pos[1];
      float distance = sqrt((d0*d0)+(d1*d1));
      //Log("distance: %f  umbrella.radius: %f\n",
      //              distance,umbrella.radius);
      if (distance <= umbrella.radius &&
          node->pos[1] > umbrella.pos[1]) {
        if (node->linewidth > 1) {
          if (deflection) {
            //deflect raindrop
            double dot;
            Vec v, up = {0,1,0};
            VecSub(node->pos, umbrella.pos, v);
            normalize(v);
            node->pos[0] =
              umbrella.pos[0] + v[0] * umbrella.radius;
            node->pos[1] =
              umbrella.pos[1] + v[1] * umbrella.radius;
            dot = VecDot(v,up);
            dot += 1.0;
            node->vel[0] += v[0] * dot * 1.0;
            node->vel[1] += v[1] * dot * 1.0;
          } else {
            Raindrop *savenode = node->next;
            delete_rain(node);
            node = savenode;
            continue;
          }
        }
      }
    }
    //VecCopy(umbrella.pos, umbrella.lastpos);
  }
#endif //USE_UMBRELLA
  if (node->pos[1] < -20.0f) {
    //rain drop is below the visible area
    Raindrop *savenode = node->next;
    delete_rain(node);
    node = savenode;
    continue;
  }
  //if (node->next == NULL) break;
  node = node->next;
}
if (maxrain < n)
  maxrain = n;
  //}
  }

void physics(void)
{
  if (show_bigfoot){
    if(activeFruit==1){
      move_fruit();
    }
  }
  if(totalScore>6){
    nextLevel=1;
  }
  if(levelnumber==1){
  if(myScore==8){
    levelnumber=2;
    show_bigfoot=0;
    showNextLevel=1;
  }
  }else{

  }
  if(missedApples>3){
    show_bigfoot=0;
    show_gameover=1;
   
  }
  if (show_rain)
    check_raindrops();
}

#ifdef USE_UMBRELLA
void draw_umbrella(void)
{
  //Log("draw_umbrella()...\n");
  if (umbrella.shape == UMBRELLA_FLAT) {
    glColor4f(1.0f, 0.2f, 0.2f, 0.5f);
    glLineWidth(8);
    glBegin(GL_LINES);
    glVertex2f(umbrella.pos[0]-umbrella.width2, umbrella.pos[1]);
    glVertex2f(umbrella.pos[0]+umbrella.width2, umbrella.pos[1]);
    glEnd();
    glLineWidth(1);
  } else {
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    glPushMatrix();
    glTranslatef(umbrella.pos[0],umbrella.pos[1],umbrella.pos[2]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glBindTexture(GL_TEXTURE_2D, umbrellaTexture);
    glBegin(GL_QUADS);
    float w = umbrella.width2;
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-w,  w);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( w,  w);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( w, -w);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-w, -w);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
    glPopMatrix();
  }
}
#endif //USE_UMBRELLA

void draw_raindrops(void)
{
  //if (ihead) {
  Raindrop *node = ihead;
  while(node) {
    glPushMatrix();
    glTranslated(node->pos[0],node->pos[1],node->pos[2]);
    glColor4fv(node->color);
    glLineWidth(node->linewidth);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, node->length);
    glEnd();
    glPopMatrix();
    node = node->next;
  }
  //}
  glLineWidth(1);
}

void render(void)
{

  Rect r;

  //Clear the screen
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  //
  //
  //draw a quad with texture
  float wid = 20.0f;
  float widB=60.0f;
  glColor3f(1.0, 1.0, 1.0);

  if (forest) {
    glBindTexture(GL_TEXTURE_2D, forestTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
    glEnd();
  }
 if(show_gameover){
float widm=100.0f;
    glPushMatrix();
    glTranslatef(gameoversign.pos[0], gameoversign.pos[1], gameoversign.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D,gameOverTexture);
    } else {
      glBindTexture(GL_TEXTURE_2D,silhouetteTextureGameOver);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    glBegin(GL_QUADS);
    if (bigfoot4.vel[0] > 0.0) {
      glTexCoord2f(0.0f, 1.0f); glVertex2i(-widm,-widm);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-widm, widm);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( widm, widm);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( widm,-widm);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-widm,-widm);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-widm, widm);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( widm, widm);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( widm,-widm);
    }
    glEnd();
    glPopMatrix();
  }


  if(show_menu){
float widm=160.0f;
    glPushMatrix();
    glTranslatef(bigfoot4.pos[0], bigfoot4.pos[1], bigfoot4.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D,menuTexture);
    } else {
      glBindTexture(GL_TEXTURE_2D,silhouetteTextureMenu);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    //glColor3f(0.5f,0.0f,0.0f);
    glBegin(GL_QUADS);
    if (bigfoot4.vel[0] > 0.0) {
      glTexCoord2f(0.0f, 1.0f); glVertex2i(-widm,-widm);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-widm, widm);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( widm, widm);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( widm,-widm);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-widm,-widm);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-widm, widm);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( widm, widm);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( widm,-widm);
    }
    glEnd();
    glPopMatrix();
  }
  if(showNextLevel){
    float widm=160.0f;
    glPushMatrix();
    glTranslatef(nextlevelsign.pos[0], nextlevelsign.pos[1], nextlevelsign.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D,level2Texture);
    } else {
      glBindTexture(GL_TEXTURE_2D,silhouetteTextureLevel2);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    //glColor3f(0.5f,0.0f,0.0f);
    glBegin(GL_QUADS);
    if (nextlevelsign.vel[0] > 0.0) {
      glTexCoord2f(0.0f, 1.0f); glVertex2i(-widm,-widm);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-widm, widm);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( widm, widm);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( widm,-widm);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-widm,-widm);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-widm, widm);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( widm, widm);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( widm,-widm);
    }
    glEnd();
    glPopMatrix();

  }

  if (show_bigfoot) {

    glPushMatrix();
    glTranslatef(bigfoot.pos[0], bigfoot.pos[1], bigfoot.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D,baskettexture);
    } else {
      glBindTexture(GL_TEXTURE_2D, silhouetteTextureFruit );
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    glBegin(GL_QUADS);
    if (bigfoot.vel[0] > 0.0) {
      glTexCoord2f(0.0f, 1.0f); glVertex2i(-widB,-widB);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-widB, widB);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( widB, widB);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( widB,-widB);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-widB,-widB);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-widB, widB);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( widB, widB);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( widB,-widB);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(bigfoot2.pos[0], bigfoot2.pos[1], bigfoot2.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D, bigfootTexture);
    } else {
      glBindTexture(GL_TEXTURE_2D, silhouetteTexture);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    glBegin(GL_QUADS);
    if (bigfoot.vel[0] > 0.0) {
      glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
    }
    glEnd();


    glPopMatrix();

    glPushMatrix();
    glTranslatef(bigfoot3.pos[0], bigfoot3.pos[1], bigfoot3.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D, bigfootTexture);
    } else {
      glBindTexture(GL_TEXTURE_2D, silhouetteTexture);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    glBegin(GL_QUADS);
    if (bigfoot.vel[0] > 0.0) {

      glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
    }
    glEnd();
    glPopMatrix();
    
    if(nextLevel){
    glPushMatrix();
    glTranslatef(bomb1.pos[0], bomb1.pos[1], bomb1.pos[2]);
    if (!silhouette) {
      glBindTexture(GL_TEXTURE_2D, bombTexture);
    } else {
      glBindTexture(GL_TEXTURE_2D, silhouetteTextureBomb);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.0f);
      glColor4ub(239,223,255,255);
    }
    glBegin(GL_QUADS);
    if (bomb1.vel[0] > 0.0) {

      glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
      glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
      glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
      glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    } else {
      glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
      glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
      glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
      glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
    }
    glEnd();
    glPopMatrix();
    }


    //start of menu screen 


    //
    /*
       if (trees && silhouette) {
       glBindTexture(GL_TEXTURE_2D, forestTransTexture);
       glBegin(GL_QUADS);
       glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
       glTexCoord2f(0.0f, 0.0f); glVertex2i(0, yres);
       glTexCoord2f(1.0f, 0.0f); glVertex2i(xres, yres);
       glTexCoord2f(1.0f, 1.0f); glVertex2i(xres, 0);
       glEnd();
       }*/
    glDisable(GL_ALPHA_TEST);
  }


  glDisable(GL_TEXTURE_2D);
  //glColor3f(1.0f, 0.0f, 0.0f);
  //glBegin(GL_QUADS);
  //  glVertex2i(10,10);
  //  glVertex2i(10,60);
  //  glVertex2i(60,60);
  //  glVertex2i(60,10);
  //glEnd();
  //return;
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  if (show_rain)
    draw_raindrops();
  glDisable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  //
#ifdef USE_UMBRELLA
  if (show_umbrella)
    draw_umbrella();
#endif //USE_UMBRELLA
  glBindTexture(GL_TEXTURE_2D, 0);
  //
  //
  r.bot = yres - 20;
  r.left = 10;
  r.center = 0;
  unsigned int cref = 0x00ffffff;
  //ggprint8b(&r, 16, cref, "B - Bigfoot");
  //ggprint8b(&r, 16, cref, "F - Forest");
  //ggprint8b(&r, 16, cref, "S - Silhouette");
  //ggprint8b(&r, 16, cref, "T - Trees");
  //ggprint8b(&r, 16, cref, "U - Umbrella");
  //ggprint8b(&r, 16, cref, "R - Rain");
  //ggprint8b(&r, 16, cref, "D - Deflection");
  //ggprint8b(&r, 16, cref, "N - Sounds");
}

