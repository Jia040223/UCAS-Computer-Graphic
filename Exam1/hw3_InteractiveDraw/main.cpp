#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <stdio.h>
#include <cmath>

#define N 100
#define PI M_PI

#define BLACK   0.0, 0.0, 0.0
#define WHITE   1.0, 1.0, 1.0
#define GREEN   0.0, 1.0, 0.0
#define RED     1.0, 0.0, 0.0
#define BLUE    0.0, 0.0, 1.0
#define YELLOW  1.0, 1.0, 0.0
#define PURPLE  0.502, 0.0, 0.502

#define GREY    190.0/255, 190.0/255, 190.0/255
#define DIMGREY 105.0/255, 105.0/255, 105.0/255


using namespace std;

void glArc(double x, double y, double start_angle, double end_angle, double radius, int mode);
void glArcPoint(double x, double y, double start_angle, double end_angle, double radius);    // 只绘制散点

int  r0, r1, r2, r3,value, color = 1, shape = 1, mod = 1, num = 0;
int a[N*6];

//for free draw
int b[N*100];
int my_index = 0;
bool isDrawing = false;

void bg_color_menu(int value){
    switch(value)
    {
    case 1 :
        glClearColor(BLACK, 1.0);
        break;
    case 2 :
        glClearColor(GREY, 1.0);
        break;
    case 3 :
        glClearColor(DIMGREY, 1.0);
        break;
    default:
        glClearColor(BLACK, 1.0);
        break;
    }

    glutPostRedisplay();
}

void color_menu(int value){
    color = value;  // 存储颜色序号
}

void shape_menu(int value){
    shape = value;  // 存储形状序号
}

void mode_menu(int value){
    mod = value;  // 存储形状序号
}

void mymenu(int value){
    if(value==4){
        num=0;  // 把原有图形清空
        my_index=0;
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_STENCIL_BUFFER_BIT);
        glFlush();
        printf("清屏\n");
    }
    if(value==5)
        exit(0);
}

void menu(){
    //创建背景颜色子菜单
    r0 = glutCreateMenu(bg_color_menu);
    glutAddMenuEntry("Black", 1);
    glutAddMenuEntry("Grey", 2);
    glutAddMenuEntry("DimGrey", 3);
    //创建颜色子菜单
    r1 = glutCreateMenu(color_menu);
    glutAddMenuEntry("Green", 1);
    glutAddMenuEntry("Red", 2);
    glutAddMenuEntry("Blue", 3);
    glutAddMenuEntry("White", 4);
    glutAddMenuEntry("Yellow", 5);
    glutAddMenuEntry("Purple", 6);
    //创建图形子菜单
    r2 = glutCreateMenu(shape_menu);
    glutAddMenuEntry("Line segment", 1);
    glutAddMenuEntry("Rect", 2);
    glutAddMenuEntry("Triangle", 3);
    glutAddMenuEntry("Circle", 4);
    //创建绘画模式子菜单
    r3 = glutCreateMenu(mode_menu);
    glutAddMenuEntry("Basic Draw Mode", 0); // 进入普通绘制模式
    glutAddMenuEntry("Free Draw Mode", -1); // 进入自由绘制模式
    glutAddMenuEntry("Close Draw Mode", 1); // 退出绘制模式
    //创建菜单，连接子菜单
    glutCreateMenu(mymenu);
    glutAddSubMenu("background color", r0);
    glutAddSubMenu("Choose color", r1);
    glutAddSubMenu("Choose shape", r2);
    glutAddSubMenu("Draw Mode", r3);
    glutAddMenuEntry("Clear Screen", 4);
    glutAddMenuEntry("Exit", 5);
    //鼠标右键控制并选择菜单按钮
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void drawOBjects(int mode){
    int i;
    double my_circle_x, my_circle_y, my_circle_r;
    int start_idx, end_idx;
    for(i=0; i<num; i++){
        switch (int(a[i*6+4]))  // 存储的颜色
        {
        case 1:
            glColor3f(GREEN);
            break;
        case 2:
            glColor3f(RED);
            break;
        case 3:
            glColor3f(BLUE);
            break;
        case 4:
            glColor3f(WHITE);
            break;
        case 5:
            glColor3f(YELLOW);
            break;
        case 6:
            glColor3f(PURPLE);
            break;
        default:
            glColor3f(BLACK);
            break;
        }
        switch (int(a[i*6+5]))  // 存储的形状
        {
        case 1:
            glBegin(GL_LINES);
            glVertex2f(a[i*6], a[i*6+1]);
            glVertex2f(a[i*6+2], a[i*6+3]);
            glEnd();
            break;
        case 2:
            glRectf(a[i*6], a[i*6+1], a[i*6+2], a[i*6+3]);
            break;
        case 3:
            glBegin(GL_TRIANGLES);
            glVertex2f(a[i*6], a[i*6+1]);
            glVertex2f(a[i*6+2], a[i*6+3]);
            glVertex2f(a[i*6], a[i*6+3]);
            glEnd();
            break;
        case 4:
            my_circle_x = ((double)(a[i*6] + a[i*6+2]))/2;
            my_circle_y = ((double)(a[i*6+1] + a[i*6+3]))/2;
            my_circle_r = sqrt((a[i*6] - a[i*6+2])*(a[i*6] - a[i*6+2]) + (a[i*6+1] - a[i*6+3])*(a[i*6+1] - a[i*6+3]))/2;
            glArc(my_circle_x, my_circle_y, 0, 2*PI, my_circle_r, GL_POLYGON);
            break;
        case 5:
            start_idx = a[i*6];
            end_idx = a[i*6+1] ? a[i*6+2] : my_index;
            glBegin(GL_LINE_STRIP);
            for(int j = start_idx; j < end_idx; j+=2){
                glVertex2f(b[j], b[j+1]);
            }
            glEnd();
            break;
        default:
            break;
        }
    }
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    drawOBjects(GL_RENDER);
    glFlush();
}

void mouseClick(int button, int state, int x, int y){
    // 存储起点坐标
    if(mod == 0){
        if ((num <= N) && (state== GLUT_DOWN) && (button== GLUT_LEFT_BUTTON)){
            a[num * 6] = (x); 
            a[num * 6 + 1] = (400 -y);
        }
        // 存储终点坐标，绘制颜色，图形种类
        if((num<=N) && (state==GLUT_UP) && (button==GLUT_LEFT_BUTTON) && (x!=a[num*6]|| y!=a[num*6+1])){
            a[num*6+2] = x;
            a[num*6+3] = 400-y;
            a[num*6+4] = color;
            a[num*6+5] = shape;
            printf("num:%d\n", num+1);
            num++;
            display();
        }
    }
    else if(mod == -1){
        if ((num <= N) && (state== GLUT_DOWN) && (button== GLUT_LEFT_BUTTON)){
            a[num * 6] = my_index; 
            a[num*6+1] = 0; 
            b[my_index++] = x;
            b[my_index++] = (400 -y);
            isDrawing = true;
        }
        // 存储终点坐标，绘制颜色，图形种类
        if((num<=N) && (state==GLUT_UP) && (button==GLUT_LEFT_BUTTON) && (x!=a[num*6]|| y!=a[num*6+1])){
            a[num*6+1] = 1; 
            a[num*6+2] = my_index;
            a[num*6+4] = color;
            a[num*6+5] = 5;
            printf("num:%d\n", num+1);
            num++;
            isDrawing = false;
            display();
        }
    }
    
}

void motion(int x, int y) {
    if (isDrawing && mod == -1) {
        b[my_index++] = x;
        b[my_index++] = 400 - y;
        glutPostRedisplay();
    }
}

void init(){
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 400);

    glutCreateWindow("Simple");
    menu(); // 建立菜单

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //设置相机位置
    gluLookAt(200, 200, 100, 200, 200, 0, 0, 1, 0);
    //设置为正交投影
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-200, 200, -200, 200, 0, 200);
}

int main(int argc, char *argv[]){
    glutInit(&argc, argv);
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouseClick);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}

//画弧线,相对偏移量XY,开始的弧度,结束的弧度,半径
void glArc(double x, double y, double start_angle, double end_angle, double radius, int mode){
  //开始绘制曲线
  glBegin(mode);
  glArcPoint(x, y, start_angle, end_angle, radius);
  //结束绘画
  glEnd();
}

void glArcPoint(double x, double y, double start_angle, double end_angle, double radius){
  //每次画增加的弧度
  double delta_angle = PI / 180;
  //画圆弧
  for (double i = start_angle; i <= end_angle; i += delta_angle){
    //绝对定位加三角函数值
    double vx = x + radius * cos(i);
    double vy = y + radius * sin(i);
    glVertex2f(vx, vy);
  }
}