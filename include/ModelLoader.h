#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <fstream>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

using std::string;

/* ---------- Estructura de un objeto .OBJ ------------ */
// #es un comentario, al igual que // en C++
// usemtl y mtllib describen la apariencia del modelo
// v es un vértice
// vt es la coordenada de textura de un vértice
// vn es la normal de un vértice
// f es una cara

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;
};

class ModelLoader {
public:
  bool loadOBJ(const string &filepath,
               std::vector<glm::vec3> &out_vertices,
               std::vector<glm::vec2> &out_uvs,
               std::vector<glm::vec3> &normals) {
    std::ifstream file(filepath);
    if (!file.is_open())
      return false;

    std::vector<glm::vec3> tmp_vertices;
    std::vector<glm::vec3> tmp_uvs;
    std::vector<glm::vec3> tmp_normals;
    string line;
    while (std::getline(file, line)) {
      std::istringstream iss(line);
      string type;
      iss >> type;

      std::cout << type << " daaaaaaa\n";

      if (type == "v") {
        glm::vec3 vertex;
        iss >> vertex.x >> vertex.y >> vertex.z;
        tmp_vertices.push_back(vertex);
      }
    }

      file.close();
      out_vertices = tmp_vertices;

      std::cout << "Modelo cargado: " << tmp_vertices.size() << " vértices"
                << std::endl;
    return true;
  }
};

#endif // MODEL_LOADER_H.is_open()) {
