#pragma once

#include "volk.hpp"

#include <glm/glm.hpp>

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
          std::array attributeDescriptions {
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
