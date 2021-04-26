#ifndef MODEL_SCANNER_OCTREE_H
#define MODEL_SCANNER_OCTREE_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <array>
#include <string>
#include <fstream>

namespace model_scanner {

class Octree {
public:
  Octree();
  Octree(glm::vec4 minPoint, glm::vec4 maxPoint, int depth);
  void clear();
  void update();
  void bindData();
  void bindSubData();
  void write(const std::string& filename, float threshold);

private:
  struct alignas(16) Header {
    uint32_t depth;
    uint32_t size;
    uint32_t _unused[2];
  };

  struct alignas(16) Node {
    uint32_t hits;
    uint32_t total;
    uint32_t depth;
    uint32_t _unused;
    glm::vec4 minPoint;
    glm::vec4 maxPoint;
  };

  Header _header;
  std::vector<Node> _nodeList;

  std::vector<std::array<glm::vec3, 3>> writeNode(size_t idx, float threshold);
  size_t search(glm::vec3 point, size_t current = 0);
  bool isPartOf(size_t idx, float threshold);
  static size_t depthToSize(int depth);
};

}  // namespace model_scanner

#endif  // MODEL_SCANNER_OCTREE_H
