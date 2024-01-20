#include "head.h"
#include "trace.h"
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float fov = 50;

std::vector<Sphere> spheres;
std::vector<Cube> cubes;
char texture_dir[256];
char dest_dir[256];
ImageTexture* tex[5];

int main(int argc, char* argv[]){
    if(argc==3){
      strcpy(texture_dir, argv[1]);
      strcpy(dest_dir, argv[2]);
    }
    else if(argc == 1){
      strcpy(texture_dir, "../input/");
      strcpy(dest_dir, "../output/");
    }
    else{
        cout<<"the argv is invalid"<<endl;
        return 0;
    }


#ifdef ADD_TEXTURE
    bool est = 1;
    // 获取纹理贴图源数据并创建纹理
    for(int i=0; i<5; i++){
        int nx, ny, nn;
        string img_name = texture_dir + std::to_string(i) + ".jpg";
        unsigned char *img = stbi_load(img_name.data(), &nx, &ny, &nn, 0);
        if(img == NULL){
            cout << "loading img failed" << endl;
            est = 0;
            break;
        }
        tex[i] = new ImageTexture(img, nx, ny);
    }
#else
    bool est = 0;
#endif
    if(est){
#ifndef CUBE
        // 位置，半径，表面颜色， 反射率，透射率，自发光，
        spheres.push_back(Sphere(Vec_3f(0.0, -10004, -20), 10000, Vec_3f(0.79,0.85,0.89), 0.05, 0.0));
        spheres.push_back(Sphere(Vec_3f(0.0, 0, -20), 4, Vec_3f(1.00,0.32,0.36), 1, 0.9, 0.1, tex[1]));
        spheres.push_back(Sphere(Vec_3f(5.0, -1,-15), 2, Vec_3f(0.90,0.76,0.46), 1, 0.9, 0.1, tex[2]));
        spheres.push_back(Sphere(Vec_3f(5.0, 0,-25), 3, Vec_3f(0.65,0.77,0.97), 1, 0.9, 0.1, tex[3]));
        spheres.push_back(Sphere(Vec_3f(-5.5, 0, -15), 3, Vec_3f(0.90,0.90,0.90), 1, 0.9, 0.1, tex[4]));
        // 背景球体
        spheres.push_back(Sphere(Vec_3f(0.0, 0, -20),     1000, Vec_3f(1.00,0.32,0.36), 1, 1.0, 0.1, tex[0]));

        // 光源
        spheres.push_back(Sphere(Vec_3f(0.0, 20, -30), 3, Vec_3f(0.0,0.0,0.0), 0, 0.0, Vec_3f(1)));
#else
        cubes.push_back(Cube(Vec_3f(-10000.0, -20004, -10020), Vec_3f(10000.0, -4, 9980), Vec_3f(0.79,0.85,0.89), 0.05, 0.0));
        cubes.push_back(Cube(Vec_3f(-2.5, -2.5, -22.5), Vec_3f(2.5, 2.5, -17.5), Vec_3f(1.00,0.32,0.36), 1, 0.9, 0.1, tex[1]));
        cubes.push_back(Cube(Vec_3f(4.0, -2,-16), Vec_3f(6.0, 0.0, -14), Vec_3f(0.90,0.76,0.46), 1, 0.9, 0.1, tex[2]));
        cubes.push_back(Cube(Vec_3f(3.0, -2,-27), Vec_3f(7.0, 2.0, -23), Vec_3f(0.65,0.77,0.97), 1, 0.9, 0.1, tex[3]));
        cubes.push_back(Cube(Vec_3f(-7.5, -2.0, -17), Vec_3f(-3.5, 2.0, -13), Vec_3f(0.90,0.90,0.90), 1, 0.9, 0.1, tex[4]));
        // 背景球体
        cubes.push_back(Cube(Vec_3f(-1000.0, -1000, -1020), Vec_3f(1000.0, 1000, 980), Vec_3f(1.00,0.32,0.36), 1, 1.0, 0.1, tex[0]));

        // 光源
        spheres.push_back(Sphere(Vec_3f(0.0, 20, -30), 3, Vec_3f(0.0,0.0,0.0), 0, 0.0, Vec_3f(1)));
#endif
    }
    else{
#ifndef CUBE
        spheres.push_back(Sphere(Vec_3f(0.0, -10004, -20), 10000, Vec_3f(0.20,0.20,0.20), 0, 0.0));
        spheres.push_back(Sphere(Vec_3f(0.0, 0, -20),     4, Vec_3f(1.00,0.32,0.36), 1, 0.5));
        spheres.push_back(Sphere(Vec_3f(5.0, -1,-15),     2, Vec_3f(0.90,0.76,0.46), 1, 0.0));
        spheres.push_back(Sphere(Vec_3f(5.0, 0,-25),     3, Vec_3f(0.65,0.77,0.97), 1, 0.0));
        spheres.push_back(Sphere(Vec_3f(-5.5, 0, -15),     3, Vec_3f(0.90,0.90,0.90), 1, 0.0));

        // 光源
        spheres.push_back(Sphere(Vec_3f(0.0, 20, -30), 3, Vec_3f(0.0,0.0,0.0), 0, 0.0, Vec_3f(1)));
#else
        cubes.push_back(Cube(Vec_3f(-10000.0, -20004, -10020), Vec_3f(10000.0, -4, 9980), Vec_3f(0.20,0.20,0.20), 0, 0.0));
        cubes.push_back(Cube(Vec_3f(-2.5, -2.5, -22.5), Vec_3f(2.5, 2.5, -17.5), Vec_3f(1.00,0.32,0.36), 1, 0.5));
        cubes.push_back(Cube(Vec_3f(4.0, -2,-16), Vec_3f(6.0, 0.0, -14), Vec_3f(0.90,0.76,0.46), 1, 0.0));
        cubes.push_back(Cube(Vec_3f(3.0, -2,-27), Vec_3f(7.0, 2.0, -23), Vec_3f(0.65,0.77,0.97), 1, 0.0));
        cubes.push_back(Cube(Vec_3f(-7.5, -2.0, -17), Vec_3f(-3.5, 2.0, -13), Vec_3f(0.90,0.90,0.90), 1, 0.0));

        // 光源
        cubes.push_back(Cube(Vec_3f(-3.0, 17, -33), Vec_3f(3.0, 23, -27), Vec_3f(0.0,0.0,0.0), 0, 0.0, Vec_3f(1)));
#endif
    }

#if MODE==SAVE_PIC
#ifndef CUBE
    render(spheres, fov); 
#else
    cube_render(cubes, fov);
#endif
    // 保存最终渲染结果
    std::ofstream ofs("../output/1.ppm",std::ios::out | std::ios::binary);
    if(ofs.is_open()){
        ofs << "P6\n" << WIDTH << " "<<HEIGHT<<"\n255\n";
        for(unsigned i = 0;i < WIDTH * HEIGHT * 3; i++){
            // 0, 255
            ofs <<img_buf[i];
        } 

        ofs.close();
    }
    else{
        cout << "Open save file" << "../output/1.ppm " << "failed" << endl;
        cout << "Live pic mode instead" << endl;

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

        glutInitWindowSize(WIDTH, HEIGHT);
        glutInitWindowPosition(100, 100);
        glutCreateWindow("render");

        glutSpecialFunc(SpecialKeys);
        glutMouseFunc(mouseClick);
        glutDisplayFunc(DrawScene);

        glutMainLoop();
    }

#elif MODE==SAVE_VIDEO
    for(int i=0; i<60; i++){
#ifndef CUBE
        render(spheres, fov); 
        Rotate_Sphere();
#else
        cube_render(cubes, fov);
        Rotate_Cube();
#endif
        // 保存最终渲染结果
        std::stringstream sm;
        sm << dest_dir << "frame" << i << ".ppm";
        string file_path;
        sm >> file_path;
        cout << "Saving img at "<< file_path <<endl;
        std::ofstream ofs(file_path, std::ios::out | std::ios::binary);
        ofs << "P6\n" << WIDTH << " "<<HEIGHT<<"\n255\n";
        for(unsigned i = 0;i < WIDTH * HEIGHT * 3; i++){
            // 0, 255
            ofs <<img_buf[i];
        } 
        ofs.close();
    }

    // 图片路径为 "../output/frame1.ppm"，保存路径为 "../output-video/output_video.mp4"
    const char* inputPattern = "../output/frame%d.ppm";
    const char* outputVideoPath = "../output-video/output_video.mp4";

    // 构建 FFmpeg 命令字符串
    std::string ffmpegCommand = "ffmpeg -y -framerate 30 -i " + std::string(inputPattern) + " -c:v libx264 -pix_fmt yuv420p " + std::string(outputVideoPath);

    // 使用 system 函数运行命令行
    int result = system(ffmpegCommand.c_str());

    // 检查运行结果
    if (result == 0) {
        std::cout << "Video creation successful." << std::endl;
    } else {
        std::cerr << "Error: Failed to create video." << std::endl;
    }

#else
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("render");

    glutSpecialFunc(SpecialKeys);
    glutMouseFunc(mouseClick);
    glutDisplayFunc(DrawScene);

    glutMainLoop();

#endif
    return 0;
}