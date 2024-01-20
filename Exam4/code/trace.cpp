#include "head.h"
#include "trace.h"

Vec_3f img [WIDTH*HEIGHT];
unsigned char img_buf[WIDTH*HEIGHT*3];
extern float fov;

extern float x_rotate;
extern float y_rotate;

// ------------------------------------------------------sphere------------------------------------------------------
#ifndef GEOMETRIC
// 使用代数方式，计算光线与球的交点，结果保存至t0,t1
bool Sphere::intersect(const Vec_3f &rayorigin, const Vec_3f &raydirection, float &t0, float &t1) const{
  // 将光线的表达式p=e+t·d带入球的方程(p-c)^2-R^2=0中进行求解
  Vec_3f dist = rayorigin - center;
  float a = raydirection | raydirection;
  float b = 2.0f * (raydirection | dist);
  float c = (dist | dist) - radius * radius;
  
  float Delta = b * b - 4.0f * a * c;
  // 若判别式小于0，则光线与球未相交，返回-1
  if (Delta <= 0){
    return false;
  }

  Delta = sqrt(Delta);
  t1 = (-b + Delta) / (2.0f * a);
  t0 = (-b - Delta) / (2.0f * a);
  // 获得方程的2解，较小的t0为光线进入球的位置，较大的t1为光线离开球的位置
  // 若2解均小于0，即球位于光线后面，则返回-1
  if (t1 <= 0){
    return false;
  }
  
  return true;
}
#else
// 使用几何方式，计算光线与球的交点，结果保存至t0,t1
bool Sphere::intersect(const Vec_3f &rayorigin, const Vec_3f &raydirection, float &t0, float &t1) const{
  // 光线起点到球中心的向量
  Vec_3f  l = center - rayorigin;

  // 判断光线方向是否与球位置相同
  float tca = l | raydirection;
  if(tca < 0){
    return false;
  }

  // 计算光线与球心的最短距离
  float dist = l.Squared_len() - tca*tca;
  // 距离大于半径，不相交
  if(dist > radius * radius){
    return false;
  }

  float thc = sqrt(radius*radius - dist);

  // t0：第一个相交长度，t1：第二个相交长度
  t0 = tca - thc;
  t1 = tca + thc;

  return true;
}
#endif

Vec_3f Sphere::getSurfaceColor(const Vec_3f &rayorigin, const Vec_3f &raydirection, float tnear) const{
  if(tex == NULL){
    return surfaceColor;
  }

  // 圆心和交点的连线
  Vec_3f r_vec = (rayorigin - center) + raydirection * tnear;
  r_vec.normal();
  // 获取纹理坐标
  // double phi = atan2(r_vec.z, r_vec.x);
  // double theta = asin(r_vec.y);
  double phi = atan2(r_vec.z, r_vec.x) + FLOAT_MOD(y_rotate, M_PI);
  double theta = asin(r_vec.y) + FLOAT_MOD(x_rotate, M_PI);
  double u = 1-(phi + M_PI) / (2*M_PI);
  double v = (theta + M_PI/2) / M_PI;

  // 返回rgb值
  return tex->value(u, v);
}

//----------------------------------------------------------Cube----------------------------------------------------------------

bool Cube::intersect(const Vec_3f &rayorigin, const Vec_3f &raydirection, float &t_near, float &t_far) const {
    if(raydirection.x == 0.0 | raydirection.y == 0.0 | raydirection.z == 0.0){
      return false;
    }
    
    Vec_3f invDir = Vec_3f(1.0f) / raydirection;

    // 计算立方体相对于光线起点的距离
    Vec_3f t_min = (min_point - rayorigin) * invDir;
    Vec_3f t_max = (max_point - rayorigin) * invDir;

    // 找到 t_near 和 t_far
    Vec_3f t1 = min(t_min, t_max);
    Vec_3f t2 = max(t_min, t_max);

    t_near = max(max(t1.x, t1.y), t1.z);
    t_far = min(min(t2.x, t2.y), t2.z);

    // 如果 t_near 大于 t_far，则没有相交
    if (t_near > t_far || t_far < 0) {
        return false;
    }

    return true;
}

Vec_3f Cube::getSurfaceColor(const Vec_3f &rayorigin, const Vec_3f &raydirection, float tnear) const {
    if (tex == NULL) {
        return surfaceColor;
    }

    // 计算相交点的坐标
    Vec_3f intersection_point = rayorigin + raydirection * tnear;

    // 计算立方体表面的纹理坐标
    float u = (intersection_point.x - min_point.x) / (max_point.x - min_point.x);
    float v = (intersection_point.y - min_point.y) / (max_point.y - min_point.y);

    // 返回rgb值
    return tex->value(u, v);
}

Vec_3f Cube::calculateNormal(const Vec_3f &hitPos) const {
    Vec_3f normal(0, 0, 0);

    // 判断与立方体各个面的距离，找到最近的面
    float minDist = INFINITY;
    float dist;

    // 与 x 轴平行的两个面
    dist = std::abs(hitPos.x - min_point.x);
    if (dist < minDist) {
        minDist = dist;
        normal = Vec_3f(-1, 0, 0);
    }

    dist = std::abs(hitPos.x - max_point.x);
    if (dist < minDist) {
        minDist = dist;
        normal = Vec_3f(1, 0, 0);
    }

    // 与 y 轴平行的两个面
    dist = std::abs(hitPos.y - min_point.y);
    if (dist < minDist) {
        minDist = dist;
        normal = Vec_3f(0, -1, 0);
    }

    dist = std::abs(hitPos.y - max_point.y);
    if (dist < minDist) {
        minDist = dist;
        normal = Vec_3f(0, 1, 0);
    }

    // 与 z 轴平行的两个面
    dist = std::abs(hitPos.z - min_point.z);
    if (dist < minDist) {
        minDist = dist;
        normal = Vec_3f(0, 0, -1);
    }

    dist = std::abs(hitPos.z - max_point.z);
    if (dist < minDist) {
        normal = Vec_3f(0, 0, 1);
    }

    return normal;
}


// ---------------------------------------------------------trace--------------------------------------------------------------- //

// 光线追踪函数，输入光线起点、方向、场景中的球体、光线追踪的最大深度
Vec_3f trace(const Vec_3f &rayorigin, const Vec_3f &raydirection, const std::vector<Sphere> & spheres, int depth)
{
    float tnear = INFINITY; // 最近相交参数
    const Sphere * sphere = NULL; // 最近相交的球体
  
    // 与场景中的球体求交，找到最近的交点
    for(int i = 0; i < spheres.size(); i++){
      float t0 = INFINITY;
      float t1 = INFINITY;

      if(spheres[i].intersect(rayorigin, raydirection, t0, t1)){
        if(t0 < 0){
          t0=t1;
        }

        if(t0<tnear){
          tnear = t0;
          sphere = &spheres[i];
        }
      }
    }
    
    // 光线不与场景中物体相交，返回背景色
    if(sphere == NULL){
      return Vec_3f(BG_COLOR);
    }

    Vec_3f surfaceColor = 0;  // 定义表面颜色
    Vec_3f hitPos = rayorigin + raydirection * tnear; // 计算交点
    Vec_3f hitNorm  = hitPos - sphere->center; // 计算交点背离球心的向量hitNorm ，球的表面法向
    hitNorm /= sphere->radius;  // 正则化
    float bias = 1e-4;
    bool inside = false;

    // hitNorm 与光线点积为正，说明光线起点位于球里面
    if((raydirection | hitNorm ) > 0){
      hitNorm  = -hitNorm ;
      inside = true;
    }

    if((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_DEPTH){
      float facingratio = - (raydirection | hitNorm );
      float fresneleffect = mix(pow(1-facingratio, 3), 1.0, 0.1); //菲涅尔反射公式

      // 计算发射方向
      Vec_3f reflect_direction = raydirection + hitNorm  * 2 * facingratio;
      reflect_direction.normal();
      // 递归调用光线追踪函数
      Vec_3f next_reflection = trace(hitPos + hitNorm  * bias, reflect_direction, spheres, depth+1);
      
      Vec_3f next_refraction = 0;
      // 球体透明，才计算透射
      if(sphere->transparency){
        // 判断是内侧投射or外侧投射
        float ior = 1.1, eta = inside? ior : 1/ior;
        // 计算投射角度
        float cosi = -(hitNorm  | raydirection); //均已经归一化
        float k = 1- eta * eta * (1 - cosi * cosi);
        Vec_3f refraction_direction = raydirection * eta + hitNorm  * (eta * cosi - sqrt(k));
        refraction_direction.normal();
        // 递归调用光线追踪
        next_refraction = trace(hitPos - hitNorm  * bias, refraction_direction, spheres, depth+1);
      }
      // 表面颜色是反射&投射的叠加
      Vec_3f ori_SColor = sphere->getSurfaceColor(rayorigin, raydirection, tnear);

      surfaceColor = (next_reflection * fresneleffect + next_refraction*(1- fresneleffect)* sphere->transparency) * ori_SColor;
    }
    else{
      for(unsigned i=0; i<spheres.size(); i++){
        if(sphere[i].emissionColor.x >0){
          Vec_3f transmission = 1;
          // 计算光线方向
          Vec_3f lightDirection = sphere[i].center - hitPos;
          lightDirection.normal();
          // 计算光源核其他交点间是否有其他物体，添加阴影效果
          for(unsigned j=0; j<spheres.size();++j){
            if(i!=j){
              float t0, t1;
              if(sphere[j].intersect(hitPos + hitNorm  * bias, lightDirection, t0, t1)){
                transmission = 0;
                break;
              }
            }
          }
          // 叠加物体颜色
          surfaceColor += sphere->getSurfaceColor(rayorigin, raydirection, tnear) * transmission * std::max(float(0), (hitNorm | lightDirection))*spheres[i].emissionColor;
        }
      }
    }
    // 返回最终表面颜色
    return surfaceColor + sphere->emissionColor;
}


// 光线追踪函数，输入光线起点、方向、场景中的球体、光线追踪的最大深度
Vec_3f cube_trace(const Vec_3f &rayorigin, const Vec_3f &raydirection, const std::vector<Cube> & cubes, int depth)
{
    float tnear = INFINITY; // 最近相交参数
    const Cube * cube = NULL; // 最近相交的球体
  
    // 与场景中的球体求交，找到最近的交点
    for(int i = 0; i < cubes.size(); i++){
      float t0 = INFINITY;
      float t1 = INFINITY;

      if(cubes[i].intersect(rayorigin, raydirection, t0, t1)){
        if(t0 < 0){
          t0=t1;
        }

        if(t0<tnear){
          tnear = t0;
          cube = &cubes[i];
        }
      }
    }
    
    // 光线不与场景中物体相交，返回背景色
    if(cube == NULL){
      return Vec_3f(BG_COLOR);
    }

    Vec_3f surfaceColor = 0;  // 定义表面颜色
    Vec_3f hitPos = rayorigin + raydirection * tnear; // 计算交点
    Vec_3f hitNorm  = cube->calculateNormal(hitPos); // 计算交点背离球心的向量hitNorm ，球的表面法向
    hitNorm.normal();  // 正则化
    float bias = 1e-4;
    bool inside = false;

    // hitNorm 与光线点积为正，说明光线起点位于球里面
    if((raydirection | hitNorm ) > 0){
      hitNorm  = -hitNorm ;
      inside = true;
    }

    if((cube->transparency > 0 || cube->reflection > 0) && depth < MAX_DEPTH){
      float facingratio = - (raydirection | hitNorm );
      float fresneleffect = mix(pow(1-facingratio, 3), 1.0, 0.1); //菲涅尔反射公式

      // 计算发射方向
      Vec_3f reflect_direction = raydirection + hitNorm  * 2 * facingratio;
      reflect_direction.normal();
      // 递归调用光线追踪函数
      Vec_3f next_reflection = cube_trace(hitPos + hitNorm  * bias, reflect_direction, cubes, depth+1);
      
      Vec_3f next_refraction = 0;
      // 球体透明，才计算透射
      if(cube->transparency){
        // 判断是内侧投射or外侧投射
        float ior = 1.1, eta = inside? ior : 1/ior;
        // 计算投射角度
        float cosi = -(hitNorm  | raydirection); //均已经归一化
        float k = 1- eta * eta * (1 - cosi * cosi);
        Vec_3f refraction_direction = raydirection * eta + hitNorm  * (eta * cosi - sqrt(k));
        refraction_direction.normal();
        // 递归调用光线追踪
        next_refraction = cube_trace(hitPos - hitNorm  * bias, refraction_direction, cubes, depth+1);
      }
      // 表面颜色是反射&投射的叠加
      Vec_3f ori_SColor = cube->getSurfaceColor(rayorigin, raydirection, tnear);

      surfaceColor = (next_reflection * fresneleffect + next_refraction*(1- fresneleffect)* cube->transparency) * ori_SColor;
    }
    else{
      for(unsigned i=0; i<cubes.size(); i++){
        if(cube[i].emissionColor.x >0){
          Vec_3f transmission = 1;
          // 计算光线方向
          Vec_3f lightDirection = (cube[i].min_point + cube[i].max_point) * 0.5f - hitPos;
          lightDirection.normal();
          // 计算光源核其他交点间是否有其他物体，添加阴影效果
          for(unsigned j=0; j<cubes.size();++j){
            if(i!=j){
              float t0, t1;
              if(cube[j].intersect(hitPos + hitNorm  * bias, lightDirection, t0, t1)){
                transmission = 0;
                break;
              }
            }
          }
          // 叠加物体颜色
          surfaceColor += cube->getSurfaceColor(rayorigin, raydirection, tnear) * transmission * std::max(float(0), (hitNorm | lightDirection))*cubes[i].emissionColor;
        }
      }
    }
    // 返回最终表面颜色
    return surfaceColor + cube->emissionColor;
}


// 渲染函数，计算每个像素的颜色
void render(const std::vector<Sphere> &spheres, float fov) {
    Vec_3f *pixel = img;
    float invWidth = 1 / float(WIDTH), invHeight = 1 / float(HEIGHT);
    float aspectratio = WIDTH / float(HEIGHT);
    float angle = tan((M_PI * 0.5 * fov) / 180.);

    // 对所有像素光线追踪
    for (unsigned y = 0; y < HEIGHT; y++) {
      for (unsigned x = 0; x < WIDTH; x++, pixel++) {
        // 计算光线方向
        float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
#if (MODE == LIVE_PIC || MODE == LIVE_VIDEO)
        float yy = (2 * ((y + 0.5) * invHeight) - 1) * angle;
#else
        float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
#endif
        Vec_3f raydir(xx, yy, -1);
        raydir.normal();

        // 使用光线追踪算法计算像素的颜色
        *pixel = trace(Vec_3f(0), raydir, spheres, 0);
      }
    }

    // 将颜色映射到图像缓冲区
    for (unsigned i = 0; i < WIDTH * HEIGHT; i++) {
      img_buf[3 * i] = (unsigned char)(min(float(1), img[i].x) * 255);
      img_buf[3 * i + 1] = (unsigned char)(min(float(1), img[i].y) * 255);
      img_buf[3 * i + 2] = (unsigned char)(min(float(1), img[i].z) * 255);
    }
}


// 渲染函数，计算每个像素的颜色
void cube_render(const std::vector<Cube> &cubes, float fov) {
    Vec_3f *pixel = img;
    float invWidth = 1 / float(WIDTH), invHeight = 1 / float(HEIGHT);
    float aspectratio = WIDTH / float(HEIGHT);
    float angle = tan((M_PI * 0.5 * fov) / 180.);

    // 对所有像素光线追踪
    for (unsigned y = 0; y < HEIGHT; y++) {
      for (unsigned x = 0; x < WIDTH; x++, pixel++) {
        // 计算光线方向
        float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
#if (MODE == LIVE_PIC || MODE == LIVE_VIDEO)
        float yy = (2 * ((y + 0.5) * invHeight) - 1) * angle;
#else
        float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
#endif
        Vec_3f raydir(xx, yy, -1);
        raydir.normal();

        // 使用光线追踪算法计算像素的颜色
        *pixel = cube_trace(Vec_3f(0), raydir, cubes, 0);
      }
    }

    // 将颜色映射到图像缓冲区
    for (unsigned i = 0; i < WIDTH * HEIGHT; i++) {
      img_buf[3 * i] = (unsigned char)(min(float(1), img[i].x) * 255);
      img_buf[3 * i + 1] = (unsigned char)(min(float(1), img[i].y) * 255);
      img_buf[3 * i + 2] = (unsigned char)(min(float(1), img[i].z) * 255);
    }
}

