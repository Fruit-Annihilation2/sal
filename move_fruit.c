//salvador lopez
//hw2
//move_fruit.c
//function will drop fruit from top of the screen.
//once fruit goes out of bounds or is caught by basket
//it is respawned and re-dropped.
//you can execute this function by pressing the f key
//it also checks if apple caught by basket.
//counts number of missed apples and number of 
//caught apples

#include <stdio.h>
#include "move_fruit.h"
#define random(a) (rand()%a)


void move_fruit()
{
  int addgrav = 1;

  bigfoot2.pos[1] -= 5.0;
  //move bigfoot...
  //Update position
  bigfoot2.pos[0] += bigfoot2.vel[0];
  bigfoot2.pos[1] += bigfoot2.vel[1];
  if ((bigfoot2.pos[1] < 150.0 && bigfoot2.vel[1] < 0.0) ||
      (bigfoot2.pos[1] >= (float)yres && bigfoot2.vel[1] > 0.0)) {
    bigfoot2.vel[1] =- bigfoot2.vel[1];
    addgrav = 0;

  }

  if(bigfoot.pos[0]-50<bigfoot2.pos[0] &&bigfoot.pos[0]+50>bigfoot2.pos[0] && bigfoot2.pos[1]<bigfoot.pos[1]) {
    bigfoot2.pos[1]=500;
    bigfoot2.pos[0]=random(500);
    myScore++;
    totalScore++;
    printf("myscore:%d\n",myScore);
  }

  if(bigfoot2.pos[1]<-100) {
    bigfoot2.pos[1]=500;
    bigfoot2.pos[0]=random(500);
    missedApples++;
    printf("missedApples:%d\n",missedApples);
  }
  bigfoot3.pos[1] -= 5.0;
  //move bigfoot...
  //Update position
  bigfoot3.pos[0] += bigfoot3.vel[0];
  bigfoot3.pos[1] += bigfoot3.vel[1];
  if ((bigfoot3.pos[1] < 150.0 && bigfoot3.vel[1] < 0.0) ||
      (bigfoot3.pos[1] >= (float)yres && bigfoot3.vel[1] > 0.0)) {
    bigfoot3.vel[1] = -bigfoot3.vel[1];
    addgrav = 0;
  }
  if(bigfoot.pos[0]-50<bigfoot3.pos[0] && bigfoot.pos[0]+50>bigfoot3.pos[0] && bigfoot3.pos[1]<bigfoot.pos[1]) {

    bigfoot3.pos[1]=500;
    bigfoot3.pos[0]=random(500);
    myScore++;
    totalScore++;
    printf("myscore:%d\n",myScore);
  }
  if(bigfoot3.pos[1]<-100) {
    bigfoot3.pos[1]=800;
    bigfoot3.pos[0]=random(500);
    missedApples++;
    printf("missedApples:%d\n",missedApples);
  }

//bomb
  if(nextLevel){
    physicsRate=0.8/30.0;
 bomb1.pos[1] -= 5.0;
  //move bigfoot...
  //Update position
  bomb1.pos[0] += bomb1.vel[0];
  bomb1.pos[1] += bomb1.vel[1];
  if ((bomb1.pos[1] < 150.0 && bomb1.vel[1] < 0.0) ||
      (bomb1.pos[1] >= (float)yres && bomb1.vel[1] > 0.0)) {
    bomb1.vel[1] =- bomb1.vel[1];
    addgrav = 0;

  }

  if(bigfoot.pos[0]-50<bomb1.pos[0] &&bigfoot.pos[0]+50>bomb1.pos[0] && bomb1.pos[1]<bigfoot.pos[1]) {
    
    missedApples=5;
  }

  if(bomb1.pos[1]<-1) {
    bomb1.pos[1]=500;
    bomb1.pos[0]=random(500);
  }
  }

//socre vel
  if(myScore>6){
   bigfoot3.vel[1]+=bigfoot.vel[1];
   bigfoot2.vel[1]+=bigfoot.vel[1];
  }
}

