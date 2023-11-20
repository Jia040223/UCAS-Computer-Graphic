

#include <GL/glut.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>        
#include <string>
#include <vector>
#include <stdio.h>

#define MAX 1000 //用于对坐标的归一化
#define SCALE_GAP 0.05
#define LIGHT_GAP 0.1
#define LOWER_BOUND 0.05  //放大下界
#define UPPER_BOUND 3 //放大上界
#define MAX_LIGHT 3.0   //光强上界
#define MIN_LIGHT 0.5   //光强下界
#define ROTATE_GAP 5.0f
#define CALC_NORMAL // 自行计算法向量

using namespace std;

struct VECTOR3{ //存储三维坐标
    double X;
    double Y;
    double Z;
};
struct VERTEX { //点的数据结构
    double X;
    double Y;
    double Z;
};

struct NORMAL { //法向量的数据结构
    double NX;
    double NY;
    double NZ;
};

struct FACE { //平面数据结构
    int V[3];
    int VN[3];
};//由三个v/vn的索引形式组成，一个面含三个顶点


class OBJ{ //OBJ文件定义的物体
    public:
        vector<VERTEX> V;
        vector<NORMAL> VN;
        vector<FACE> F;

        void Read_Obj(); ////读取OBJ文件
};

float x_rotate = 0.0; //旋转角度
float y_rotate = 0.0; //旋转角度
float scale = 0.8;  //放大倍率

int material = 0; //用于定义材质
int mouse_state = GLUT_UP;
int mouse_Button = GLUT_LEFT_BUTTON;

double light = 1.0f;

OBJ my_obj;

void OBJ::Read_Obj() {
    ifstream obj_file("./bunny.obj");
    FACE* f;
    VERTEX* v;
    NORMAL* vn;

    if(obj_file.is_open()){
        string line;
        while (getline(obj_file, line)) {
            if (line.length() < 2) {
                continue;
            }

            if (line[0] == 'v') {
                if (line[1] == 'n') {
                    istringstream in(line);
                    vn = new NORMAL();
                #ifndef CALC_NORMAL
                    string head;
                    in >> head >> vn->NX >> vn->NY >> vn->NZ;
                #else
                    vn->NX=vn->NY=vn->NZ=0;
                #endif
                    my_obj.VN.push_back(*vn);
                }
                else {
                    istringstream in(line);
                    v = new VERTEX();
                    string head;
                    in >> head >> v->X >> v->Y >> v->Z;
                    my_obj.V.push_back(*v);
                }
            }
            else if (line[0] == 'f') {
                for (std::string::iterator it = line.begin();it != line.end();it++) {
                    if (*it == '/'){
                        *it = ' ';
                    }
                }

                istringstream in(line);
                f = new FACE();
                string head;
                in >> head;
                int i = 0;
                while (i < 3) {
                    //确定顶点对应的vertex index（从0开始计数）
                    if (my_obj.V.size() != 0) {
                        in >> f->V[i];
                        f->V[i] -= 1;
                    }
                    
                    //确定顶点对应的法向量 index（从0开始计数）
                    if (my_obj.VN.size() != 0) {
                        in >> f->VN[i];
                        f->VN[i] -= 1;
                    }
                    i++;
                }
                my_obj.F.push_back(*f);
            }
        }
    }
}

//计算叉积(未归一化)
VECTOR3 cross(const VERTEX& v0, const VERTEX& v1, const VERTEX& v2) {
    VECTOR3 v0_v1 = {v1.X - v0.X, v1.Y - v0.Y, v1.Z - v0.Z};
    VECTOR3 v0_v2 = {v2.X - v0.X, v2.Y - v0.Y, v2.Z - v0.Z};

    VECTOR3 fv =   {v0_v1.Y * v0_v2.Z - v0_v1.Z * v0_v2.Y,
                    v0_v1.Z * v0_v2.X - v0_v1.X * v0_v2.Z,
                    v0_v1.X * v0_v2.Y - v0_v1.Y * v0_v2.X}; // 平面法向量（未归一化，含面积信息）
    
    return fv;
}


//自行计算法向量
void Calc_NORMAL(){
    FACE *f;
    VERTEX v0, v1, v2;      // 平面的三个顶点
    NORMAL *vn;
    double length;
    cout<<"Calculating NORMAL..."<<endl;

    for(auto it = my_obj.F.begin();it != my_obj.F.end(); it++){
        //根据三个顶点计算平面法向量
        v0 = my_obj.V[it->V[0]];
        v1 = my_obj.V[it->V[1]];
        v2 = my_obj.V[it->V[2]];
        VECTOR3 fv = cross(v0, v1, v2);// 平面法向量（未归一化，含面积信息）

        // 将带权法向量加到每一个顶点的法向向量上
        for(int i = 0;i < 3;i++){
            int idx = it->VN[i];

            my_obj.VN[idx].NX += fv.X;
            my_obj.VN[idx].NY += fv.Y;
            my_obj.VN[idx].NZ += fv.Z;
        }
    }

    //遍历顶点，归一化顶点方向向量
    for(auto it = my_obj.VN.begin();it != my_obj.VN.end();it++){
        vn = &(*it);
        length = sqrt(vn->NX*vn->NX + vn->NY*vn->NY + vn->NZ*vn->NZ);
        vn->NX /= length;
        vn->NY /= length;
        vn->NZ /= length;
    }
}


//设置物体材质
void Set_material(){
    // 定义黄铜材质
    static GLfloat brass_ambient[] = { 0.33f, 0.22f, 0.03f, 1.0f };
    static GLfloat brass_diffuse[] = { 0.78f, 0.57f, 0.11f, 1.0f };
    static GLfloat brass_specular[] = { 0.99f, 0.91f, 0.81f,1.0f };
    static GLfloat brass_matemission[] = { 0.000f, 0.000f, 0.000f, 1.0f };
    static GLfloat fShininess = 27.800f;

    //定义蓝宝石材质
    static GLfloat blue_sapphire_ambient[] = {0.0f, 0.0f, 0.2f, 1.0f};  // 深蓝色环境光
    static GLfloat blue_sapphire_diffuse[] = {0.0f, 0.0f, 0.8f, 1.0f};  // 深蓝色漫反射光
    static GLfloat blue_sapphire_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};  // 镜面高光部分
    static GLfloat blue_sapphire_matemission[] = { 0.000f, 0.000f, 0.000f, 1.0f };
    static GLfloat blue_sapphire_shininess = 100.0f;  // 镜面高光指数

    //定义绿宝石材质
    static GLfloat emerald_ambient[] = {0.0215f, 0.1745f, 0.0215f, 1.0f};  // 深绿色环境光
    static GLfloat emerald_diffuse[] = {0.07568f, 0.61424f, 0.07568f, 1.0f};  // 深绿色漫反射光
    static GLfloat emerald_specular[] = {0.633f, 0.727811f, 0.633f, 1.0f};  // 镜面高光部分
    static GLfloat emerald_matemission[] = { 0.000f, 0.000f, 0.000f, 1.0f };
    static GLfloat emerald_shininess = 76.8f;  // 镜面高光指数

    //定义红宝石材质
    static GLfloat ruby_ambient[] = {0.1745f, 0.01175f, 0.01175f, 1.0f};  // 深红色环境光
    static GLfloat ruby_diffuse[] = {0.61424f, 0.04136f, 0.04136f, 1.0f};  // 深红色漫反射光
    static GLfloat ruby_specular[] = {0.727811f, 0.626959f, 0.626959f, 1.0f};  // 镜面高光部分
    static GLfloat ruby_matemission[] = { 0.000f, 0.000f, 0.000f, 1.0f };
    static GLfloat ruby_shininess = 76.8f;  // 镜面高光指数


    //指定 用于光照计算的材质属性，参数FACE的取值GL_FRONT，GL_BACK，GL_FRONT_AND_BACK，决定材质属性用于哪一面
    switch(material){
        case 0:
            glMaterialfv(GL_FRONT, GL_AMBIENT, brass_ambient);      //GL_AMBIENT表示各种光线照射到该材质上，经过很多次反射后最终遗留在环境中的光线强度（颜色）
            glMaterialfv(GL_FRONT, GL_DIFFUSE, brass_diffuse);      //GL_DIFFUSE表示光线照射到该材质上，经过漫反射后形成的光线强度（颜色）
            glMaterialfv(GL_FRONT, GL_SPECULAR, brass_specular);    //GL_SPECULAR表示光线照射到该材质上，经过镜面反射后形成的光线强度（颜色）
            glMaterialfv(GL_FRONT, GL_EMISSION, brass_matemission);    //该材质本身就微微的向外发射光线对应颜色
            glMaterialf(GL_FRONT, GL_SHININESS, fShininess);        //“镜面指数”，取值范围是0到128。该值越小，表示材质越粗糙，点光源发射的光线照射到上面，也可以产生较大的亮点
            break;
        case 1:
            glMaterialfv(GL_FRONT, GL_AMBIENT, blue_sapphire_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, blue_sapphire_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, blue_sapphire_specular);
            glMaterialfv(GL_FRONT, GL_EMISSION, blue_sapphire_matemission);
            glMaterialf(GL_FRONT, GL_SHININESS, blue_sapphire_shininess);
            break;
        case 2:
            glMaterialfv(GL_FRONT, GL_AMBIENT, emerald_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, emerald_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, emerald_specular);
            glMaterialfv(GL_FRONT, GL_EMISSION, emerald_matemission);
            glMaterialf(GL_FRONT, GL_SHININESS, emerald_shininess);
            break;
        case 3:
            glMaterialfv(GL_FRONT, GL_AMBIENT, ruby_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, ruby_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, ruby_specular);
            glMaterialfv(GL_FRONT, GL_EMISSION, ruby_matemission);
            glMaterialf(GL_FRONT, GL_SHININESS, ruby_shininess);
            break;
        default:
            glMaterialfv(GL_FRONT, GL_AMBIENT, brass_ambient);      
            glMaterialfv(GL_FRONT, GL_DIFFUSE, brass_diffuse);  
            glMaterialfv(GL_FRONT, GL_SPECULAR, brass_specular);   
            glMaterialfv(GL_FRONT, GL_EMISSION, brass_matemission);    
            glMaterialf(GL_FRONT, GL_SHININESS, fShininess);        
            break;
    }
}


void Init_scene() {
#ifdef CALC_NORMAL
    Calc_NORMAL();
#endif
    glClearColor(0.000f, 0.000f, 0.000f, 1.0f);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}

//进行绘画
void GLCube(){
    for (int i = 0; i < my_obj.F.size(); i++){
        // drawing begin
        glBegin(GL_TRIANGLES);
        for(int j=0; j <3; j++){  	
            glNormal3f(my_obj.VN[my_obj.F[i].VN[j]].NX, my_obj.VN[my_obj.F[i].VN[j]].NY, my_obj.VN[my_obj.F[i].VN[j]].NZ);
            glVertex3f(my_obj.V[my_obj.F[i].V[j]].X / MAX, my_obj.V[my_obj.F[i].V[j]].Y / MAX, my_obj.V[my_obj.F[i].V[j]].Z / MAX);
        }
        // drawing end
        glEnd();
    }
}

void DrawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glLoadIdentity(); 

    Set_material();//设置材质

    // Add a light source
    GLfloat glfLight[] = { -4.0f, 4.0f, -4.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, glfLight);

    GLfloat light_intensity[4];//光源强度
    for(int i = 0;i < 3;i++){
        light_intensity[i] = light;
    }
    light_intensity[3] = 1.0f;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // 上下旋转
    glRotatef(x_rotate, 1.0f, 0.0f, 0.0f);
    // 左右旋转
    glRotatef(y_rotate, 0.0f, 1.0f, 0.0f); 
    // 放大缩小
    glScalef(scale, scale, scale);


    GLCube();

    glFlush();
    glutSwapBuffers();
}

// 键盘上下左右键调整视角
void SpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) 
        x_rotate -= ROTATE_GAP;
    else if (key == GLUT_KEY_DOWN) 
        x_rotate += ROTATE_GAP;
    else if (key == GLUT_KEY_LEFT) 
        y_rotate -= ROTATE_GAP;
    else if (key == GLUT_KEY_RIGHT) 
        y_rotate += ROTATE_GAP;
    DrawScene();
}


void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        //键盘1234定义物体材质
        case '1':
            material = 0;
            break;
        case '2':
            material = 1;
            break;
        case '3':
            material = 2;
            break;
        case '4':
            material = 3;
            break;
        //键盘ws修改光照强度
        case 'w':
            if(light >= MAX_LIGHT)
                printf("Reach the light upper bound!\n");
            else
                light += LIGHT_GAP;
            break;
        case 's':
            if(light <= MIN_LIGHT)
                printf("Reach the light lower bound!\n");
            else
                light -= LIGHT_GAP;
            break;
        default:
            return;
    }
    DrawScene();
}

//设置计时器，使得鼠标左击每隔一段时间缩小，右击每隔一段时间放大，每次操作步幅为SCALE_GAP
void timer(int value) { 
    if (mouse_Button == GLUT_RIGHT_BUTTON && mouse_state == GLUT_DOWN) {
        if(scale>=UPPER_BOUND)
            printf("Reach the scale upper bound!\n");
        else
            scale += SCALE_GAP;
    }

    if (mouse_Button == GLUT_LEFT_BUTTON && mouse_state == GLUT_DOWN){
        if(scale<=LOWER_BOUND)    // 避免scale减小为非正值
            printf("Reach the scale lower bound!\n");
        else
            scale -= SCALE_GAP;
    }
    DrawScene();

    // 定时器函数，按下鼠标后每隔200ms调用一次即可实现连续缩放
    glutTimerFunc(200, timer, 0); // 设置下一次调用定时器的时间
}

//设置全局变量，用于缩放
void mouseClick(int button, int state, int x, int y) {
    mouse_state = state;
    if (state == GLUT_DOWN) {
        mouse_Button = button;
    }
}


int main(int argc, char* argv[]) {
    my_obj.Read_Obj();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("bunny");

    Init_scene();    
    glutDisplayFunc(DrawScene);
    glutSpecialFunc(SpecialKeys);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseClick);
    glutTimerFunc(200, timer, 0);// 设置第一次调用定时器的时间

    glutMainLoop();
    return 0;
}
