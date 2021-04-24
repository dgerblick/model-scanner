#include <model_scanner/Octree.h>

namespace model_scanner::Octree {

void writeOctree(const std::string& filename, float threshold, Header header,
                 const std::vector<Node>& nodes) {
  // TODO
}

void writeNode(std::ofstream& ofs, float threshold,
               const std::vector<Node>& nodes, size_t index) {
  // TODO
}

}  // namespace model_scanner::Octree
