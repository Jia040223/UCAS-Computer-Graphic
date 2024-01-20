#ifndef _HEAD_
#define _HEAD_

#include <GL/glut.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>        
#include <string>


//------------------------------- for show_pic.cpp ------------------------------
#define MAX 1000
#define BG_COLOR 1
// #define BG_COLOR 1,0.96,0.93
using namespace std;
// #define WIDTH 960
// #define HEIGHT 720
#define WIDTH 640
#define HEIGHT 480
#define FOV_GAP 5

#define ROTATE_GAP (M_PI/60)
#define LOWER_BOUND 0
#define UPPER_BOUND 100
//extern Vec_3f img [WIDTH*HEIGHT];
extern unsigned char img_buf[WIDTH*HEIGHT*3];
void DrawScene();
void mouseClick(int button, int state, int x, int y);
void SpecialKeys(int key, int x, int y);
void Rotate_Sphere();
void Rotate_Cube();

//------------------------------- for main.cpp ------------------------------
#define SAVE_PIC 0   // 生成单张图片
#define SAVE_VIDEO 1   // 保存动画
#define LIVE_PIC 2   // 实时渲染单张图片（可交互）
#define LIVE_VIDEO 3  // 实时渲染动画

// #define MODE LIVE_PIC
#define GEOMETRIC
#define MODE SAVE_VIDEO
#define ADD_TEXTURE

//#define CUBE

#endif