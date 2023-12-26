#include "head.h"
#include <sys/time.h>
extern MyMesh mesh;

int main(int argc, char* argv[]){
    
    struct timeval tv_start, tv_end;
    char input_name[256];
    char output_name[256];
    float c;
    int total=0;  // 计算总顶点个数
    if(argc==4){
      strcpy(input_name, argv[1]);
      strcpy(output_name, argv[2]);
      sscanf(argv[3], "%f", &c);
    }
    else if(argc == 1){
      cout << "please input the path of input OBJ file" <<endl;
      std::cin.getline(input_name, 256);

      cout << "please input the path of output OBJ file" <<endl;
      std::cin.getline(output_name, 256);

      cout << "please input the ratio of simplification" << endl;
      cin >> c;
    }
    else{
        cout<<"the argv is invalid"<<endl;
        return 0;
    }

    // 读取文件失败
    if(OpenMesh::IO::read_mesh(mesh, input_name)==false){
        cout<<"Failed to load in the raw model, please check your path"<<endl;
        return 0;
    }
    cout<<"Load model successfully"<<endl;

  #ifdef FACES_TARGET
    total = mesh.n_faces();
    cout<<"the raw Model has "<<mesh.n_vertices()<<" vertex in total"<<endl;
    cout<<"the raw Model has "<<total<<" face in total"<<endl;
    int del_num = total* c; // 简化后的面个数
    cout << "Deleting "<< total - del_num <<" faces in total"<<endl;
  #else
    total = mesh.n_vertices();
    cout<<"the raw Model has "<<total<<" vertex in total"<<endl;
    cout<<"the raw Model has "<<mesh.n_faces()<<" face in total"<<endl;
    int del_num = total*(1-c); // 待删除的顶点个数
    cout << "Deleting "<< del_num <<" vertexs in total"<<endl;
  #endif

    gettimeofday(&tv_start,NULL);
    simplify(del_num);
    gettimeofday(&tv_end,NULL);

    long start_us = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
    long end_us   = tv_end.tv_sec   * 1000000 + tv_end.tv_usec;

    cout<<"CPU time for simplification = "<<static_cast<float>(end_us - start_us) / 1000000 << " s" <<endl;
  
    cout<<"the simplified Model has "<<mesh.n_vertices()<<" vertex in total"<<endl;
    cout<<"the simplified Model has "<<mesh.n_faces()<<" face in total"<<endl;

    if(OpenMesh::IO::write_mesh(mesh, output_name)){
      cout<<"Saving simplified models with simplification ratio "<< c <<" at "<< output_name <<endl;
      // 显示模型
      cout<<"Loading the model"<<endl;
      show_model(argc, argv);
    }
    else{
      cout<<"Fail to save the model"<<endl;
    }
}