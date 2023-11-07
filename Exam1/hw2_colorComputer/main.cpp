#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define  _USE_MATH_DEFINES
#include <math.h>

#define PI M_PI
#define BLACK 0,0,0
#define WHITE 1,1,1
#define GREY    190.0/255, 190.0/255, 190.0/255
#define DIMGREY 105.0/255, 105.0/255, 105.0/255
#define YELLOW  255.0/255, 255.0/255, 0.0/255
#define RED     205.0/255, 79.0/255,  57.0/255
#define BLUE    135.0/255, 206.0/255, 250.0/255

using namespace std;

void glTri(double x1, double y1, double x2, double y2, double x3, double y3, int mode);
void glRect(double leftX, double leftY, double rightX, double rightY, int mode);
void glArc(double x, double y, double start_angle, double end_angle, double radius, int mode);
void glArcPoint(double x, double y, double start_angle, double end_angle, double radius);    // 只绘制散点
void glRectSmooth(double leftX, double leftY, double rightX, double rightY, double radius, int mode); // 画四角为圆弧的长方体
void glStar(double cx, double cy, double radius, int numSides, int mode);//画N角星(N为奇数)
void glString(const char* str, double x, double y);//展示字符串

void init(void)
{
  glClearColor(WHITE, 0.0f);
  return;
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  //显示字符串
  glString("By JCH", 0.6, 0.7);
  glLineWidth(2.0f);
  // 屏幕外框
  glColor3f(GREY);
  glRect(-0.6, -0.4, 0.6, 0.4, GL_POLYGON); 
  // 屏幕内框
  glColor3f(BLUE);
  glRectSmooth(-0.55, -0.35, 0.55, 0.35, 0.05, GL_POLYGON);
  // 基座
  glColor3f(DIMGREY);
  glRectSmooth(-0.25, -0.62, 0.25, -0.47, 0.03, GL_POLYGON);
  glColor3f(BLACK);
  glRectSmooth(-0.25, -0.62, 0.25, -0.47, 0.03, GL_LINE_LOOP);  // 基座的描线需特殊处理
  glColor3f(DIMGREY);
  glRect(-0.16, -0.55, 0.16, -0.40, GL_POLYGON);
  // 屏幕内图像
  glColor3f(YELLOW);
  glArc(0, 0, 0, 2*PI, 0.1, GL_POLYGON);
  //glTri(-0.15, -0.1, 0.15, -0.1, 0, 0.15, GL_POLYGON);
  glColor3f(RED);
  //glArc(0, 0, 0, 2*PI, 0.05, GL_POLYGON);
  glStar(0.0, 0.0, 0.1, 5, GL_TRIANGLE_FAN);
  

  // 统一描线
  glColor3f(BLACK);
  glRect(-0.6, -0.4, 0.6, 0.4, GL_LINE_LOOP); 
  // 屏幕内框
  glRectSmooth(-0.55, -0.35, 0.55, 0.35, 0.05, GL_LINE_LOOP);
  // 基座
  glRect(-0.16, -0.55, 0.16, -0.40, GL_LINE_LOOP);
  // 屏幕内图像
  //glTri(-0.15, -0.1, 0.15, -0.1, 0, 0.15, GL_LINE_LOOP);
  //glArc(0, 0, 0, 2*PI, 0.05, GL_LINE_STRIP);
  glArc(0, 0, 0, 2*PI, 0.1, GL_LINE_STRIP);
  glFlush();
  return;
}

int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(600, 600);
  glutCreateWindow("my colorComputer");
  glutDisplayFunc(display);
  init();
  glutMainLoop();
  return 0;
}

//画三角形,传入三个点的坐标, mode：GL_LINE_LOOP / GL_POLYGON
void glTri(double x1, double y1, double x2, double y2, double x3, double y3, int mode) {
  //画封闭线
  glBegin(mode);
  //一点
  glVertex2f(x1, y1);
  //二点
  glVertex2f(x2, y2);
  //三点
  glVertex2f(x3, y3);
  //结束画线
  glEnd();
}

//画矩形,传入的是左下角XY坐标和右上角XY坐标
void glRect(double leftX, double leftY, double rightX, double rightY, int mode) {
  //画封闭曲线
  glBegin(mode);
  //左下角
  glVertex2f(leftX, leftY);
  //右下角
  glVertex2f(rightX, leftY);
  //右上角
  glVertex2f(rightX, rightY);
  //左上角
  glVertex2f(leftX, rightY);
  //结束画线
  glEnd(); 
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

void glRectSmooth(double leftX, double leftY, double rightX, double rightY, double radius, int mode){
  glBegin(mode);
  // 为了画圆弧，需要缩进(shrink)一部分
  double s_leftX, s_leftY, s_rightX, s_rightY;  
  s_leftX = leftX + radius;
  s_leftY = leftY + radius;
  s_rightX = rightX - radius;
  s_rightY = rightY - radius;

  // Arc画屏幕内框的圆弧的散点
  glArcPoint( s_leftX,  s_leftY,       PI, 1.5 * PI, radius);
  glArcPoint(s_rightX,  s_leftY, 1.5 * PI,   2 * PI, radius);
  glArcPoint(s_rightX, s_rightY,        0, 0.5 * PI, radius);
  glArcPoint( s_leftX, s_rightY, 0.5 * PI,       PI, radius);

  glEnd();
}

//画N角星(N为奇数)
void glStar(double cx, double cy, double radius, int numSides, int mode){
  double angleStep = 2.0 * PI / numSides;
  double offsetAngle = PI / 2.0;

  glBegin(mode);
  glVertex2f(cx, cy);
  for(int i = 0; i <= numSides; i++){
    double angle = offsetAngle + ((i * 2) % numSides) * angleStep;
    double x = cx + radius * cos(angle);
    double y = cy + radius * sin(angle);
    glVertex2f(x, y);
  }

  glEnd();
}

void glString(const char* str, double x, double y){
  glColor3f(0.0, 0.0, 0.0);
  glRasterPos2f(x, y);//字符串位置
  
  for(int i = 0;i < strlen(str); i++){
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24 , str[i]);
  }

  glutSwapBuffers();
}