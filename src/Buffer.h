#pragma once

#include <vulkan/vulkan.h>

class Buffer final
{
public:
  Buffer(VkDevice device,
         VkPhysicalDevice physicalDevice,
         VkBufferUsageFlags bufferUsageFlags,
         VkMemoryPropertyFlags memoryProperties,
         VkDeviceSize size,
         const void* data = nullptr);

  void destroy() const; // Only call when construction succeeded

  bool copyTo(const Buffer& target, VkCommandBuffer commandBuffer, VkQueue queue) const;
  void* map() const;
  void unmap() const;

  bool isValid() const;
  VkBuffer getBuffer() const;

private:
  bool valid = true;

  VkDevice device = nullptr;
  VkBuffer buffer = nullptr;
  VkDeviceMemory deviceMemory = nullptr;
  VkDeviceSize size = 0u;
};