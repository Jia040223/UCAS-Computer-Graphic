#include "head.h"

MyMesh mesh;
vector<MyMesh::EdgeHandle> Edges;  // 边容器

OpenMesh::VPropHandleT<Matrix4d> Q; 
OpenMesh::EPropHandleT<struct EdgeInfo>Edge_Infos; 

//计算顶点的误差度量矩阵Q
Matrix4d ComputeQ(MyMesh::VertexHandle vh){
    Matrix4d Qt_i;
    Qt_i.fill(0);
    auto vertex = mesh.point(vh);  // 从句柄获得点
    // 遍历和目标顶点相邻的面，计算损失矩阵坐标
    for(auto v_fh = mesh.vf_begin(vh); v_fh!=mesh.vf_end(vh); v_fh++){  // v_fh：vertex相邻的face的handle
      if(!mesh.is_valid_handle(*v_fh))
        continue;
      Matrix4d Q_s;
      auto normal = mesh.normal(*v_fh);

      //求点与法向的点积
      double distance = vertex|normal;

      MatrixXd plane(1,4);
      plane << normal[0], normal[1], normal[2], -distance;

      //计算Q_s
      Q_s = plane.transpose() * plane;
      Qt_i += Q_s; 
    }

    return Qt_i;
}

// 为每个顶点增加损失矩阵属性
void Add_Q(){
    cout<<"Computing Q for each vertex..."<<endl;
    if(!mesh.get_property_handle(Q, "Q Property"))
      mesh.add_property(Q, "Q Property");
    // // 获取法向信息
    if(!mesh.has_face_normals())
      mesh.request_face_normals();
    if(!mesh.has_vertex_normals())
      mesh.request_vertex_normals();
    mesh.update_normals();
    // 遍历所有顶点
    for(auto vh=mesh.vertices_begin(); vh!=mesh.vertices_end(); ++vh){  // vh:vertex handle
      mesh.property(Q, *vh) = ComputeQ(*vh);
    }
    cout<<"Q computing for each vertex finished"<<endl;
}

bool computeQEM(EdgeInfo& edge_info, const MyMesh::EdgeHandle& e){
    // Get Q matrices for the two vertices
     // Assuming edge is valid, get the two vertices of the edge
    MyMesh::HalfedgeHandle half_e = mesh.halfedge_handle(e, 0);
    MyMesh::VertexHandle v1 = mesh.from_vertex_handle(half_e);
    MyMesh::VertexHandle v2 = mesh.to_vertex_handle(half_e);

    // Get Q matrices for the two vertices
    Eigen::Matrix4d Q1 = mesh.property(Q, v1);
    Eigen::Matrix4d Q2 = mesh.property(Q, v2);

    // Calculate cost using QEM formula
    Eigen::Matrix4d Qsolve = Q1 + Q2;
    Eigen::Matrix4d Q_sum = Qsolve;

    Eigen::Vector4d tmp_vec(0.0, 0.0, 0.0, 1.0);
    Qsolve.row(3) = tmp_vec;

    Eigen::Vector4d optimalv4 = Qsolve.inverse() * tmp_vec;

    edge_info.cost = optimalv4.transpose() * Q_sum * optimalv4;
    edge_info.optimalv3 = optimalv4.head<3>();

    return true;
}

// Function to compute cost and optimal point for each edge
void Add_EdgeInfo() {
    cout<<"Adding EdgeInfo for each edge"<<endl;
    if(!mesh.get_property_handle(Edge_Infos, "EdgeInfo Property"))
        mesh.add_property(Edge_Infos, "EdgeInfo Property");

    for (auto edge_it = mesh.edges_begin(); edge_it != mesh.edges_end(); ++edge_it) {
        if (!mesh.is_valid_handle(*edge_it))
            continue;

        EdgeInfo edge_info;
        computeQEM(edge_info, *edge_it);

        // Store the cost as an attribute in the edge
        mesh.property(Edge_Infos, *edge_it)=edge_info;
    }
    cout<<"EdgeInfo adding for each edge finished"<<endl;
}

// 比较函数，用于排序边
bool cost_cmp(const MyMesh::EdgeHandle& e1, const MyMesh::EdgeHandle& e2) {
    if (!mesh.is_valid_handle(e1) || !mesh.is_valid_handle(e2))
        return true; 

    auto cost1 = mesh.property(Edge_Infos, e1).cost;
    auto cost2 = mesh.property(Edge_Infos, e2).cost;

    return cost1 < cost2; 
}

// 构建边列表并排序
void SortEdges() {
    Edges.clear();
    for (auto eh : mesh.edges()) {
        Edges.push_back(eh);
    }
    std::sort(Edges.begin(), Edges.end(), cost_cmp);
    std::cout << "Sort " << Edges.size() << " edges in total" << std::endl;
}
 

bool deleteAdjacentEdges(const MyMesh::VertexHandle v) {
    if (!mesh.is_valid_handle(v))
        return false;

    // 遍历与指定VERTEX相邻的边
    for (auto edge : mesh.ve_range(v)) {
        auto it = std::find(Edges.begin(), Edges.end(), edge);
        if (it != Edges.end())
            Edges.erase(it);
    }
    return true;
}

void UpdateEdgeInfo(const MyMesh::VertexHandle v) {
    for (auto edge : mesh.ve_range(v)) {
        if (!mesh.is_valid_handle(edge))
            continue;

        auto v1 = mesh.to_vertex_handle(mesh.halfedge_handle(edge, 0));
        auto v2 = mesh.to_vertex_handle(mesh.halfedge_handle(edge, 1));

        if (!mesh.is_valid_handle(v1) || !mesh.is_valid_handle(v2))
            continue;

        // 重新计算 Q property
        // Get Q matrices for the two vertices
        Eigen::Matrix4d Q1 = ComputeQ(v1);
        Eigen::Matrix4d Q2 = ComputeQ(v2);

        mesh.property(Q, v1) = Q1;
        mesh.property(Q, v2) = Q2;

        // 重新计算 item property
        EdgeInfo edge_info;
        computeQEM(edge_info, edge);
        // Store the cost as an attribute in the edge
        mesh.property(Edge_Infos, edge)=edge_info;

        // 重新插入排序
        auto pos = std::upper_bound(Edges.begin(), Edges.end(), edge, cost_cmp);
        Edges.insert(pos, edge);
    }

}

// 更新边列表
void Delete_Vertex(const MyMesh::EdgeHandle& e, int& total) {
    if (!mesh.is_valid_handle(e))
        return;

    auto half_e = mesh.halfedge_handle(e, 0);
    auto from = mesh.from_vertex_handle(half_e);
    auto to = mesh.to_vertex_handle(half_e);

    if (mesh.is_boundary(e) || mesh.is_boundary(to))
        return;

    if (!mesh.is_collapse_ok(half_e))
        return;

    if(!deleteAdjacentEdges(to) || !deleteAdjacentEdges(from)){
        return;
    }

    if (!mesh.is_valid_handle(to)){
        return;
    }

    EdgeInfo edge_info = mesh.property(Edge_Infos, e);
    OpenMesh::Vec3f newPoint(edge_info.optimalv3[0], edge_info.optimalv3[1], edge_info.optimalv3[2]);
    mesh.set_point(to, newPoint);
    
    mesh.collapse(half_e);
    UpdateEdgeInfo(to);

    mesh.garbage_collection();
    total -= 1;

#ifdef CONSIDER_NO_EDGE
    while (Compare_Vertex_pair_cost(to)){
        total--;
    }
#endif
}

bool No_edge_cost(EdgeInfo& edge_info, const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2){
    // Get Q matrices for the two vertices
    Eigen::Matrix4d Q1 = mesh.property(Q, v1);
    Eigen::Matrix4d Q2 = mesh.property(Q, v2);

    // Calculate cost using QEM formula
    Eigen::Matrix4d Qsolve = Q1 + Q2;
    Eigen::Matrix4d Q_sum = Qsolve;

    Eigen::Vector4d tmp_vec(0.0, 0.0, 0.0, 1.0);
    Qsolve.row(3) = tmp_vec;

    Eigen::Vector4d optimalv4 = Qsolve.inverse() * tmp_vec;

    edge_info.cost = optimalv4.transpose() * Q_sum * optimalv4;
    edge_info.optimalv3 = optimalv4.head<3>();

    return true;  
}

bool Delete_Vertex_no_edge(EdgeInfo edge_info, const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2){
    if (mesh.is_boundary(v1) || mesh.is_boundary(v2))
        return false;

    if(!deleteAdjacentEdges(v1) || !deleteAdjacentEdges(v2)){
        return false;
    }

    OpenMesh::Vec3f newPoint(edge_info.optimalv3[0], edge_info.optimalv3[1], edge_info.optimalv3[2]);

    mesh.set_point(v1, newPoint);
    mesh.set_point(v1, newPoint);
    
    mesh.garbage_collection();
    RemoveDuplicatedVertex(v1, v2);

    if (!mesh.is_valid_handle(v1))
        UpdateEdgeInfo(v1);
    if (!mesh.is_valid_handle(v2))
        UpdateEdgeInfo(v2);

    return true;
}

bool Compare_Vertex_pair_cost(const MyMesh::VertexHandle v){
    if (mesh.is_valid_handle(v)){
        return false;
    }
    for(auto vh=mesh.vertices_begin(); vh!=mesh.vertices_end(); ++vh){  // vh:vertex handle
        if (mesh.is_valid_handle(*vh)){
            continue;
        }
        MyMesh::Point p1 = mesh.point(v);
        MyMesh::Point p2 = mesh.point(*vh);

        // 计算两点之间的欧几里德距离
        double distance = (p1 - p2).length();
        if(distance > MAX_DEST && distance != 0){
            continue;
        }

        EdgeInfo edge_info;
        No_edge_cost(edge_info, v, *vh);
        double cost = mesh.property(Edge_Infos, *Edges.begin()).cost;
        if(edge_info.cost < cost){
            if(Delete_Vertex_no_edge(edge_info, v, *vh)){
                return true;
            }
        }
    }

    return false;
}



// 删除指定数量的顶点
void simplify(int total) {
    MeshPropInit();
    SortEdges();

#ifdef FACES_TARGET
    int origin = mesh.n_faces();
    int remain = origin;
    while (remain > total && !Edges.empty()) {
        auto e_del = Edges.front();
        //cout << Edges.size() << endl;
        Edges.erase(Edges.begin());
        Delete_Vertex(e_del, remain);
        remain = mesh.n_faces();
        showProgressBar(origin - remain, origin-total);
    }
    if(total > 0){
        cout << "Edege is empty, still have " << total - remain << " faces to delete" << endl;
    }
    else{
        cout << "Delete faces comletely" << endl;
    }
#else
    int origin = total;
    while (total > 0 && !Edges.empty()) {
        auto e_del = Edges.front();
        //cout << Edges.size() << endl;
        Edges.erase(Edges.begin());
        Delete_Vertex(e_del, total);
        showProgressBar(origin-total, origin);
    }
    if(total > 0){
        cout << "Edege is empty, still have " << total << " vertex to delete" << endl;
    }
    else{
        cout << "Delete vertex comletely" << endl;
    }
#endif
    MeshPropDel();
}

// 初始化mesh的性质
void MeshPropInit(){
  // 获取点边面的信息，用于后续顶点删除
  if(!mesh.has_face_status())
    mesh.request_face_status();
  if(!mesh.has_edge_status())
    mesh.request_edge_status();
  if(!mesh.has_vertex_status())
    mesh.request_vertex_status();

  // 加入自定义属性
  Add_Q();
  Add_EdgeInfo();
}

// mesh性质的回收
void MeshPropDel(){
  mesh.release_face_status();
  mesh.release_edge_status();
  mesh.release_vertex_status();
  mesh.remove_property(Q);
  mesh.remove_property(Edge_Infos);
}

bool HasDuplicatedVertex()
{
    std::map<MyMesh::Point, int> mapPoints;
    for (auto iter = mesh.vertices_begin(); iter != mesh.vertices_end(); ++iter)
    {
        auto iterPt = mapPoints.find(mesh.point(*iter));
        if (iterPt == mapPoints.end())
        {
            mapPoints.insert(std::make_pair(mesh.point(*iter), 1));
            mapPoints[mesh.point(*iter)] = 1;
        }
        else
        {
            return true;
        }
    }

    return false;
}

int RemoveDuplicatedVertex(const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2)
{   
    vector<std::array<MyMesh::VertexHandle, 3>> faces;
    for (auto vf_it = mesh.vf_iter(v2); vf_it.is_valid(); ++vf_it)
    {
        std::array<MyMesh::VertexHandle, 3> face;
        int i = 0;
        auto fv_it = mesh.fv_begin(*vf_it);
        while(i < 3){
            if(*fv_it == v2){
                face[i++] = v1;
            }
            else{
                face[i++] = *(fv_it);
            }
            fv_it++;
        }
        faces.push_back(face);
    }

    if (!mesh.has_vertex_status()) mesh.request_vertex_status();
    if (!mesh.has_face_status())   mesh.request_face_status();
    if (!mesh.has_edge_status())   mesh.request_edge_status();

    // remove all duplicated vertices
    mesh.delete_vertex(v2);
    mesh.delete_isolated_vertices();

    // garbage_collection edge and face
    mesh.garbage_collection();

    for(auto face : faces){
        mesh.add_face(face[0], face[1], face[2]);
    }

    // garbage_collection vertex
    mesh.garbage_collection();

    if (!mesh.has_vertex_status()) mesh.release_vertex_status();
    if (!mesh.has_face_status())   mesh.release_vertex_status();
    if (!mesh.has_edge_status())   mesh.release_vertex_status();


    return 1;
}

void showProgressBar(int progress, int total) {
    float percentage = static_cast<float>(progress) / total;
    int barWidth = 50;

    std::cout << "[";
    int pos = static_cast<int>(barWidth * percentage);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(percentage * 100.0);
    if(progress != total)
        cout << " %\r";
    else
        cout << " %\n";
    std::cout.flush();
}

/*
int RemoveDuplicatedVertex(const MyMesh::VertexHandle v1, const MyMesh::VertexHandle v2)
{
    // Find all duplicated vertices
    int iOriginalNumVertices = static_cast<int>(mesh.n_vertices());
    std::map<MyMesh::Point, MyMesh::VertexHandle> mapPoints;
    std::map<MyMesh::VertexHandle, std::set<MyMesh::VertexHandle>> mapDuplicatedVertices;

    
    mapDuplicatedVertices.insert(std::make_pair(v1, std::set<MyMesh::VertexHandle>{ v1, v2 }));

    // Record the duplicated vertices related faces
    std::map<MyMesh::VertexHandle, std::set<std::array<MyMesh::VertexHandle, 3>>> mapVertexFaces;
    for (auto iter = mapDuplicatedVertices.begin(); iter != mapDuplicatedVertices.end(); ++iter)
    {
        auto pair = mapVertexFaces.insert(std::make_pair(iter->first, std::set<std::array<MyMesh::VertexHandle, 3>>{}));
        auto& vecFaces = pair.first->second;
        for (auto v_it = iter->second.begin(); v_it != iter->second.end(); ++v_it)
        {
            for (auto vf_it = mesh.vf_iter(*v_it); vf_it.is_valid(); ++vf_it)
            {
                std::array<MyMesh::VertexHandle, 3> face;
                int i = 0;
                auto fv_it = mesh.fv_begin(*vf_it);
                face[i++] = *(fv_it++);
                face[i++] = *(fv_it++);
                face[i++] = *(fv_it++);
                vecFaces.insert(face);
            }
        }
    }

    if (!mesh.has_vertex_status()) mesh.request_vertex_status();
    if (!mesh.has_face_status())   mesh.request_face_status();
    if (!mesh.has_edge_status())   mesh.request_edge_status();

    // remove all duplicated vertices
    for (auto iter = mapDuplicatedVertices.begin(); iter != mapDuplicatedVertices.end(); ++iter)
    {
        for (auto v_it = iter->second.begin(); v_it != iter->second.end(); ++v_it)
        {
            mesh.delete_vertex(*v_it);
        }
    }

    mesh.delete_isolated_vertices();

    // garbage_collection edge and face
    mesh.garbage_collection(false, true, true);

    // add not degenereated faces
    std::set<MyMesh::VertexHandle> setRemainVertices;
    for (auto iter = mapVertexFaces.begin(); iter != mapVertexFaces.end(); ++iter)
    {
        for (auto f_it = iter->second.begin(); f_it != iter->second.end(); ++f_it)
        {
            std::array<MyMesh::VertexHandle, 3> face = *f_it;
            unsigned short result = 0;
            for (auto iter = mapDuplicatedVertices.begin(); iter != mapDuplicatedVertices.end(); ++iter)
            {
                // bit pos records if the vertex is duplicated vertex or not
                result = 0;
                result |= (iter->second.count(face[0]) == 0 ? 0 : 1);
                result |= (iter->second.count(face[1]) == 0 ? 0 : 2);
                result |= (iter->second.count(face[2]) == 0 ? 0 : 4);
                if (result == 1 || result == 2 || result == 4)
                {
                    // replace the duplicated vertex as remaining vertex
                    face[result / 2] = iter->first;
                    mesh.add_face(face[0], face[1], face[2]);
                    setRemainVertices.insert(face[0]);
                    setRemainVertices.insert(face[1]);
                    setRemainVertices.insert(face[2]);
                    break;
                }
            }
        }
    }

    // recover status of remain vertex
    for (auto v_it = setRemainVertices.begin(); v_it != setRemainVertices.end(); ++v_it)
    {
        mesh.status(*v_it).set_deleted(false);
    }

    // garbage_collection vertex
    mesh.garbage_collection();

    if (!mesh.has_vertex_status()) mesh.release_vertex_status();
    if (!mesh.has_face_status())   mesh.release_vertex_status();
    if (!mesh.has_edge_status())   mesh.release_vertex_status();

    return iOriginalNumVertices - static_cast<int>(mesh.n_vertices());
}
*/