//salvador lopez
//move_fruit.h
typedef double Vec[3];
typedef struct t_bigfoot {
  Vec pos;
  Vec vel;
} Bigfoot;
Bigfoot bigfoot;
Bigfoot bigfoot2;
Bigfoot bigfoot3;
Bigfoot bigfoot4;
Bigfoot gameoversign;
Bigfoot bomb1;
Bigfoot nextlevelsign;

int xres, yres;
extern int myScore;
extern int missedApples;
extern int nextLevel;
extern double physicsRate;
extern int totalScore;

void move_fruit();
