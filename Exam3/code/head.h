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
#include <Eigen/Dense>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

#define MAX 1000 //用于对坐标的归一化
#define SCALE_GAP 0.05
#define LIGHT_GAP 0.1
#define LOWER_BOUND 0.05  //放大下界
#define UPPER_BOUND 3 //放大上界
#define MAX_LIGHT 3.0   //光强上界
#define MIN_LIGHT 0.2   //光强下界
#define ROTATE_GAP 5.0f

#define MAX_DEST 1.0
#define CONSIDER_NO_EDGE
#define SAME_NORMAL
//#define FACES_TARGET
//#define SHOW_EDGE

using namespace Eigen;
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

    Eigen::Matrix4d Q;

    bool operator==(const VERTEX& other) const {
        return X == other.X && Y == other.Y && Z == other.Z;
    }
};

struct NORMAL { //法向量的数据结构
    double NX;
    double NY;
    double NZ;
};

struct EdgeInfo {
    char type;
    double cost;            // Cost associated with the edge
    Eigen::Vector3d optimalv3; // Information about the optimal point
};

struct EDGE { //法向量的数据结构
    VERTEX V[2];
    EdgeInfo edge_info;

    bool operator==(const EDGE& other) const {
        return ((V[0]) == (other.V[0]) && (V[1]) == (other.V[1])) ||
                ((V[0]) == (other.V[1]) && (V[1]) == (other.V[0]));
    }
};

struct FACE { //平面数据结构
    int V[3];
    int VN[3];

     // Define the equality operator
    bool operator==(const FACE& other) const {
        return equal(begin(V), end(V), begin(other.V)) &&
               equal(begin(VN), end(VN), begin(other.VN));
    }
};//由三个v/vn的索引形式组成，一个面含三个顶点


class OBJ{ //OBJ文件定义的物体
    public:
        vector<VERTEX> V;
        vector<NORMAL> VN;
        vector<FACE> F;
        vector<EDGE> Edges;

        bool Read_Obj(const string& filename); //读取OBJ文件
        bool Write_Obj(const string& filename);//写OBJ文件
};
//---------Above data structures may not be used in this lab---------

typedef OpenMesh::TriMesh_ArrayKernelT <> MyMesh;
struct item{
  double cost;
  MyMesh::Point optimalv3;
};

// show_model.cpp
void Calc_NORMAL();
void Set_material();
void Init_scene();
void GLCube();
void DrawScene();
void SpecialKeys(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
void mouseClick(int button, int state, int x, int y);
void show_model(int argc, char* argv[]);
// simplify.cpp
Matrix4d ComputeQ(MyMesh::VertexHandle vh);
void Add_Q();
bool computeQEM(EdgeInfo& edge_info, const MyMesh::EdgeHandle& e);
void Add_EdgeInfo();
bool cost_cmp(const MyMesh::EdgeHandle& e1, const MyMesh::EdgeHandle& e2);
void SortEdges();
bool deleteAdjacentEdges(const MyMesh::VertexHandle v);
void UpdateEdgeInfo(const MyMesh::VertexHandle v);
void Delete_Vertex(const MyMesh::EdgeHandle& e, int& total);
void simplify(int total);
void MeshPropInit();
void MeshPropDel();

bool No_edge_cost(EdgeInfo& edge_info, const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2);
bool Delete_Vertex_no_edge(EdgeInfo edge_info, const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2);
bool Compare_Vertex_pair_cost(const MyMesh::VertexHandle v);
bool HasDuplicatedVertex();
int RemoveDuplicatedVertex(const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2);

void showProgressBar(int progress, int total);

#endif
