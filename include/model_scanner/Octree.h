#ifndef MODEL_SCANNER_OCTREE_H
#define MODEL_SCANNER_OCTREE_H

#include <glm/vec4.hpp>
#include <vector>
#include <string>
#include <fstream>

namespace model_scanner::Octree {

struct alignas(16) Header {
  uint32_t depth;
  uint32_t size;
  uint32_t _unused[2];
};

struct alignas(16) Node {
  uint32_t hits;
  uint32_t total;
  uint32_t _unused[2];
  glm::vec4 minPoint;
  glm::vec4 maxPoint;
};

void writeOctree(const std::string& filename, float threshold, Header header,
                 const std::vector<Node>& nodes);

void writeNode(std::ofstream& ofs, float threshold,
               const std::vector<Node>& nodes, size_t index);

}  // namespace model_scanner::Octree

#endif  // MODEL_SCANNER_OCTREE_H
