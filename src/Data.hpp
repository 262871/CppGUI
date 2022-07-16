#pragma once

#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "Core.hpp"
#include "Device.hpp"

#include <glm/glm.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <array>
#include <vector>

struct Vertex {
     glm::vec3 position;
     glm::vec3 color;
     glm::vec2 textureCoordinate;

     static VkVertexInputBindingDescription bindingDescription() {
          VkVertexInputBindingDescription bindingDescription {
               .binding   = 0,
               .stride    = sizeof(Vertex),
               .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
          };
          return bindingDescription;
     }

     static auto attributeDescriptions() {
          std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {
               VkVertexInputAttributeDescription {
                  .location = 0,
                  .binding  = 0,
                  .format   = VK_FORMAT_R32G32B32_SFLOAT,
                  .offset   = offsetof(Vertex, position) },
               VkVertexInputAttributeDescription {
                  .location = 1,
                  .binding  = 0,
                  .format   = VK_FORMAT_R32G32B32_SFLOAT,
                  .offset   = offsetof(Vertex, color) },
               VkVertexInputAttributeDescription {
                  .location = 2,
                  .binding  = 0,
                  .format   = VK_FORMAT_R32G32_SFLOAT,
                  .offset   = offsetof(Vertex, textureCoordinate) }
          };
          return attributeDescriptions;
     }
};

const auto MODEL_PATH   = "models\\viking_room.obj";

std::vector<Vertex>   vertecies;
std::vector<uint32_t> indecies;

void loadModel() {
     tinyobj::attrib_t                attrib;
     std::vector<tinyobj::shape_t>    shapes;
     std::vector<tinyobj::material_t> materials;
     std::string                      warn, err;

     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH))
          throw std::runtime_error(warn + err);

     for (const auto& shape : shapes)
          for (const auto& idx : shape.mesh.indices) {
               Vertex vert {
                    .position = {
                       attrib.vertices[3 * idx.vertex_index + 0],
                       attrib.vertices[3 * idx.vertex_index + 1],
                       attrib.vertices[3 * idx.vertex_index + 2] },
                    .color             = { 1.f, 1.f, 1.f },
                    .textureCoordinate = { attrib.texcoords[2 * idx.texcoord_index + 0], 1.f - attrib.texcoords[2 * idx.texcoord_index + 1] }
               };
               vertecies.push_back(std::move(vert));
               indecies.push_back(indecies.size());
          }
}
