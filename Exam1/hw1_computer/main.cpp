#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#define  _USE_MATH_DEFINES
#include <math.h>

#define PI M_PI

void glLine(double x1, double y1, double x2, double y2);
void glTri(double x1, double y1, double x2, double y2, double x3, double y3, int mode);
void glRect(double leftX, double leftY, double rightX, double rightY, int mode);
void glArc(double x, double y, double start_angle, double end_angle, double radius, int mode);
void glArcPoint(double x, double y, double start_angle, double end_angle, double radius);    // 只绘制散点
void glRectSmooth(double leftX, double leftY, double rightX, double rightY, double radius, int mode); // 

void init(void)
{
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  return;
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.0f, 0.0f, 0.0f);
  glLineWidth(2.0f);
  // 屏幕外框
  glRect(-0.6, -0.4, 0.6, 0.4, GL_LINE_LOOP); 
  // 屏幕内框
  glRectSmooth(-0.55, -0.35, 0.55, 0.35, 0.05, GL_LINE_LOOP);
  // 基座
  glRectSmooth(-0.25, -0.62, 0.25, -0.47, 0.03, GL_LINE_LOOP);
  glColor3f(1.0f, 1.0f, 1.0f);  // “擦掉”多余边
  glLine(-0.16, -0.47, 0.16, -0.47);
  glColor3f(0.0f, 0.0f, 0.0f);
  glRect(-0.16, -0.55, 0.16, -0.40, GL_LINE_LOOP);
  // 屏幕内图像
  glTri(-0.15, -0.1, 0.15, -0.1, 0, 0.15, GL_LINE_LOOP);
  glArc(0, 0, 0, 2*PI, 0.05, GL_LINE_STRIP);
  glFlush();
  return;
}

int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(600, 600);
  glutCreateWindow("my computer");
  glutDisplayFunc(display);
  init();
  glutMainLoop();
  return 0;
}

//画线,传入两点坐标
void glLine(double x1, double y1, double x2, double y2) {
  //画封闭线
  glBegin(GL_LINE_STRIP);
  //一点
  glVertex2f(x1, y1);
  //二点
  glVertex2f(x2, y2);
  //结束画线
  glEnd();
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