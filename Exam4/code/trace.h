#ifndef _TRACE_
#define _TRACE_

#include <iostream>
#include <math.h>
#include <vector>

#define MAX_DEPTH 10
#define FLOAT_MOD(a, b) (a)-((int)((a)/(b))*(b))
using namespace std;

template <typename T>
T mix(const T &a, const T &b, float ratio) {
    return b * ratio + a * (1 - ratio);
}


template<typename T>
class Vec_3
{
public:
    // xyz三个分量
    T x, y, z;


    // 三种构造函数
    Vec_3(){
        x = y = z = 0;
    }

    Vec_3(T value){
        x = y = z = value;
    }

    Vec_3(T value1, T value2, T value3){
        x = value1;
        y = value2;
        z = value3;
    }


    // 操作符重载
    //*
    Vec_3<T> operator * (T f) {
        return Vec_3<T>(x*f, y*f, z*f);
    }
    Vec_3<T> operator * (const Vec_3<T> &v) const{
        return Vec_3<T>(x*v.x, y*v.y, z*v.z);
    }

    // /
    Vec_3<T> operator / (T f) {
        return Vec_3<T>(x/f, y/f, z/f);
    }
    Vec_3<T> operator / (const Vec_3<T> &v) const{
        return Vec_3<T>(x/v.x, y/v.y, z/v.z);
    }

    // - 
    Vec_3<T> operator - (const Vec_3<T> &v) const{
        return Vec_3<T>(x-v.x, y-v.y, z-v.z);
    }
    Vec_3<T> operator - () {
        return Vec_3<T>(-x, -y, -z);
    }

    // +
    Vec_3<T> operator + (const Vec_3<T> &v) const{
        return Vec_3<T>(x+v.x, y+v.y, z+v.z);
    }

    // +=
    void operator += (const Vec_3<T> &v){
        x += v.x;
        y += v.y;
        z += v.z;
    }
    void operator += (float f){
        x += f;
        y += f;
        z += f;
    }

    // -=
    void operator -= (const Vec_3<T> &v){
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
    void operator -= (float f){
        x -= f;
        y -= f;
        z -= f;
    }

    // *=
    void operator *= (const Vec_3<T> &v){
        x *= v.x;
        y *= v.y;
        z *= v.z;
    }
    void operator *= (float f){
        x *= f;
        y *= f;
        z *= f;
    }

    // /=
    void operator /= (const Vec_3<T> &v){
        x /= v.x;
        y /= v.y;
        z /= v.z;
    }
    void operator /= (float f){
        x /= f;
        y /= f;
        z /= f;
    }


    // 向量点乘： |
    T operator | (const Vec_3<T> &v) const{
        return x*v.x + y*v.y + z*v.z;
    }

    // 向量长度
    T Squared_len() const{
        return x*x+y*y+z*z;
    }
    T len() const{
        return sqrt(x*x+y*y+z*z);
    }

    // 向量归一化
    void normal(){
        T length = len();
        if(length>0){
            x /= length;
            y /= length;
            z /= length;
        }
    }

    // 输出向量信息
    friend std::ostream & operator << (std::ostream & os, const Vec_3<T> &v){
        os << "[" << v.x << " "<< v.y << " " << v.z << "]";
        return os;
    }
};
typedef Vec_3<float> Vec_3f;

template <typename T>
Vec_3<T> min(const Vec_3<T> &a, const Vec_3<T> &b) {
    return Vec_3<T>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

template <typename T>
Vec_3<T> max(const Vec_3<T> &a, const Vec_3<T> &b) {
    return Vec_3<T>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}



class ImageTexture
{
    public:
    unsigned char* data;
    int nx, ny;

    ImageTexture(){}
    ImageTexture(unsigned char* pixels, int A, int B) : data(pixels), nx(A), ny(B) {}

    //输入u和v，输出对应图片像素的rgb值
    virtual Vec_3f value(float u, float v) const
    {
        int i = int((u)* nx);//求出像素索引
        int j = int((1 - v)*ny - 0.001f);
        if (i < 0) i = 0;
        if (j < 0) j = 0;
        if (i > nx - 1) i = nx - 1;
        if (j > ny - 1) j = ny - 1;
        float r = int(data[3 * i + 3 * nx*j]) / 255.0f;
        float g = int(data[3 * i + 3 * nx*j + 1]) / 255.0f;
        float b = int(data[3 * i + 3 * nx*j + 2]) / 255.0f;
        return Vec_3f(r, g, b);
    }
};



class Sphere{
public:
    Vec_3f center;  // 球心
    float radius;  // 球半径
    Vec_3f surfaceColor, emissionColor; // 球表面颜色、球自发光颜色（光源）
    float transparency, reflection;   // 球的投射度、反射度
    ImageTexture* tex;      // 指向纹理信息的指针

    // 球的构造函数
    Sphere(const Vec_3f &c, float r, const Vec_3f &s_color, float refl = 0, float transp = 0, const Vec_3f &e_color = 0, ImageTexture* t = NULL){
        center = c;
        radius = r;
        surfaceColor = s_color;
        emissionColor = e_color;
        transparency = transp;
        reflection = refl;
        tex = t;
    }

    //计算光线与球的交点，结果保存至t0,t1
    bool intersect(const Vec_3f &rayorigin, const Vec_3f &raydirection, float &t1, float &t2) const;
    Vec_3f getSurfaceColor(const Vec_3f &rayorigin, const Vec_3f &raydirection, float tnear) const;
};

class Cube {
public:
    Vec_3f min_point;  // 立方体的最小顶点
    Vec_3f max_point;  // 立方体的最大顶点
    Vec_3f surfaceColor;  // 立方体表面颜色
    Vec_3f emissionColor;  // 立方体自发光颜色（光源）
    float transparency;  // 立方体的投射度
    float reflection;  // 立方体的反射度
    ImageTexture* tex;  // 指向纹理信息的指针

    // 立方体的构造函数
    Cube(const Vec_3f &min_pt, const Vec_3f &max_pt, const Vec_3f &s_color,
         float refl = 0, float transp = 0, const Vec_3f &e_color = 0, ImageTexture* t = NULL)
        : min_point(min_pt), max_point(max_pt), surfaceColor(s_color),
          emissionColor(e_color), transparency(transp), reflection(refl), tex(t) {}

    // 计算光线与立方体的交点，结果保存至t_near, t_far
    bool intersect(const Vec_3f &rayorigin, const Vec_3f &raydirection, float &t_near, float &t_far) const;

    // 获取立方体表面颜色
    Vec_3f getSurfaceColor(const Vec_3f &rayorigin, const Vec_3f &raydirection, float tnear) const;
    Vec_3f calculateNormal(const Vec_3f &hitPos) const;
};



// 光线追踪函数，输入光线起点、方向、场景中的球体、光线追踪的最大深度
Vec_3f trace(const Vec_3f &rayorigin, const Vec_3f &raydirection, std::vector<Sphere> & spheres, int depth);
Vec_3f cube_trace(const Vec_3f &rayorigin, const Vec_3f &raydirection, const std::vector<Cube> & cubes, int depth);

void render(const std::vector<Sphere> & spheres, float fov);
void cube_render(const std::vector<Cube> & cubes, float fov);

#endif