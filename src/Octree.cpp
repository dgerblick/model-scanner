#include <model_scanner/Octree.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

namespace model_scanner {

Octree::Octree() {
  _header.depth = 0;
  _header.size = 0;
}

Octree::Octree(glm::vec4 minPoint, glm::vec4 maxPoint, int depth) {
  _nodeList.resize(depthToSize(depth));
  _nodeList[0].total = 1;
  _nodeList[0].hits = 0;
  _nodeList[0].depth = 0;
  _nodeList[0].minPoint = minPoint;
  _nodeList[0].maxPoint = maxPoint;
  for (size_t i = 1; i < _nodeList.size(); ++i) {
    Node& node = _nodeList[i];
    Node& parent = _nodeList[(i - 1) / 8];
    node.total = 1;
    node.hits = 0;
    node.depth = parent.depth + 1;
    node.minPoint.w = 1.0;
    node.maxPoint.w = 1.0;
    for (size_t j = 0; j < 3; ++j) {
      if ((((i - 1) % 8) & (1 << j)) == 0) {
        node.minPoint[j] = parent.minPoint[j];
        node.maxPoint[j] = (parent.minPoint[j] + parent.maxPoint[j]) / 2;
      } else {
        node.minPoint[j] = (parent.minPoint[j] + parent.maxPoint[j]) / 2;
        node.maxPoint[j] = parent.maxPoint[j];
      }
    }
  }

  _header.depth = depth;
  _header.size = _nodeList.size();
}

void Octree::clear() {
  for (size_t i = 0; i < _nodeList.size(); ++i) {
    Node& node = _nodeList[i];
    node.total = 1;
    node.hits = 1;
  }
}

void Octree::update() {
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(Header),
                     sizeof(Node) * _nodeList.size(), _nodeList.data());
}

void Octree::bindData() {
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               sizeof(Header) + sizeof(Node) * _nodeList.size(), nullptr,
               GL_DYNAMIC_DRAW);
}

void Octree::bindSubData() {
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Header), &_header);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(Header),
                  sizeof(Node) * _nodeList.size(), _nodeList.data());
}

void Octree::write(const std::string& filename, float threshold) {
  char header[80] = { 0 };
  std::vector<std::array<glm::vec3, 3>> triangles = writeNode(0, threshold);
  std::ofstream outFile(filename, std::ios::binary);
  uint32_t numTris = triangles.size();

  outFile.write(header, sizeof(header));
  outFile.write((char*) &numTris, sizeof(uint32_t));
  for (auto& tri : triangles) {
    glm::vec3 normal = glm::cross(tri[0] - tri[1], tri[0] - tri[1]);
    outFile.write((char*) glm::value_ptr(normal), 3 * sizeof(float));
    for (auto& vertex : tri)
      outFile.write((char*) glm::value_ptr(vertex), 3 * sizeof(float));
    uint16_t attributeByteCount = 0;
    outFile.write((char*) &attributeByteCount, sizeof(uint16_t));
  }
}

std::vector<std::array<glm::vec3, 3>> Octree::writeNode(size_t idx,
                                                        float threshold) {
  Node& node = _nodeList[idx];
  std::vector<std::array<glm::vec3, 3>> triangles;
  if (isPartOf(idx, threshold)) {
    glm::vec4 offset = node.maxPoint - node.minPoint;
    glm::vec4 center = 0.5f * (_nodeList[0].maxPoint + _nodeList[0].minPoint);
    glm::vec3 offsetX(offset.x, 0.0, 0.0);
    glm::vec3 offsetY(0.0, offset.y, 0.0);
    glm::vec3 offsetZ(0.0, 0.0, offset.z);
    glm::vec3 min(node.minPoint.x - center.x, node.minPoint.y - center.y,
                  node.minPoint.z - center.z);
    glm::vec3 max(node.maxPoint.x - center.x, node.maxPoint.y - center.y,
                  node.maxPoint.z - center.z);
    size_t neighbor = -1;
    // East  (+x)
    neighbor = search(min + offsetX);
    if (neighbor == -1 || !isPartOf(neighbor, threshold)) {
      triangles.push_back({ max, max - offsetY, min + offsetX });
      triangles.push_back({ max, min + offsetX, max - offsetZ });
    }
    // North (+y)
    neighbor = search(min + offsetY);
    if (neighbor == -1 || !isPartOf(neighbor, threshold)) {
      triangles.push_back({ max, max - offsetZ, min + offsetY });
      triangles.push_back({ max, min + offsetY, max - offsetX });
    }
    // Up    (+z)
    neighbor = search(min + offsetZ);
    if (neighbor == -1 || !isPartOf(neighbor, threshold)) {
      triangles.push_back({ max, max - offsetX, min + offsetZ });
      triangles.push_back({ max, min + offsetZ, max - offsetY });
    }
    // West  (-x)
    neighbor = search(min - offsetX);
    if (neighbor == -1 || !isPartOf(neighbor, threshold)) {
      triangles.push_back({ min, min + offsetZ, max - offsetX });
      triangles.push_back({ min, max - offsetX, min + offsetY });
    }
    // South (-y)
    neighbor = search(min - offsetY);
    if (neighbor == -1 || !isPartOf(neighbor, threshold)) {
      triangles.push_back({ min, min + offsetX, max - offsetY });
      triangles.push_back({ min, max - offsetY, min + offsetZ });
    }
    // Down  (-z)
    neighbor = search(min - offsetZ);
    if (neighbor == -1 || !isPartOf(neighbor, threshold)) {
      triangles.push_back({ min, min + offsetY, max - offsetZ });
      triangles.push_back({ min, max - offsetZ, min + offsetX });
    }
  } else if (node.depth != _header.depth) {
    std::array<std::vector<std::array<glm::vec3, 3>>, 8> childrenTris = {
      writeNode(8 * idx + 1, threshold), writeNode(8 * idx + 2, threshold),
      writeNode(8 * idx + 3, threshold), writeNode(8 * idx + 4, threshold),
      writeNode(8 * idx + 5, threshold), writeNode(8 * idx + 6, threshold),
      writeNode(8 * idx + 7, threshold), writeNode(8 * idx + 8, threshold)
    };
    size_t size = 0;
    for (auto& triList : childrenTris)
      size += triList.size();
    triangles.reserve(size);
    for (auto& triList : childrenTris)
      triangles.insert(triangles.end(), triList.begin(), triList.end());
  }
  return triangles;
}

size_t Octree::search(glm::vec3 point, size_t current) {
  if (_nodeList[current].depth == _header.depth)
    return current;
  for (size_t i = 8 * current + 1; i <= 8 * current + 8; ++i) {
    Node& node = _nodeList[i];
    if (node.minPoint.x <= point.x && point.x < node.maxPoint.x &&
        node.minPoint.y <= point.y && point.y < node.maxPoint.y &&
        node.minPoint.z <= point.z && point.z < node.maxPoint.z)
      return search(point, i);
  }
  return -1;
}

bool Octree::isPartOf(size_t idx, float threshold) {
  return (float) _nodeList[idx].hits / _nodeList[idx].total >= threshold;
}

size_t Octree::depthToSize(int depth) {
  return (1 - std::pow(8, depth + 1)) / -7;
}

}  // namespace model_scanner
