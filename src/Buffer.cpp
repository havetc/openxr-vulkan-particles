#include "Buffer.h"

#include <cstring>

Buffer::Buffer(VkDevice device,
               VkPhysicalDevice physicalDevice,
               VkBufferUsageFlags bufferUsageFlags,
               VkMemoryPropertyFlags memoryProperties,
               VkDeviceSize size,
               const void* data)
: device(device), size(size)
{
  VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = bufferUsageFlags;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS)
  {
    valid = false;
    return;
  }

  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

  VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &supportedMemoryProperties);

  const VkMemoryPropertyFlags typeFilter = memoryRequirements.memoryTypeBits;
  uint32_t memoryTypeIndex = 0u;
  bool memoryTypeFound = false;
  for (uint32_t i = 0u; i < supportedMemoryProperties.memoryTypeCount; ++i)
  {
    const VkMemoryPropertyFlags propertyFlags = supportedMemoryProperties.memoryTypes[i].propertyFlags;
    if (typeFilter & (1 << i) && (propertyFlags & memoryProperties) == memoryProperties)
    {
      memoryTypeIndex = i;
      memoryTypeFound = true;
      break;
    }
  }

  if (!memoryTypeFound)
  {
    valid = false;
    return;
  }

  VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
  if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory) != VK_SUCCESS)
  {
    valid = false;
    return;
  }

  if (vkBindBufferMemory(device, buffer, deviceMemory, 0u) != VK_SUCCESS)
  {
    valid = false;
    return;
  }

  // Fill the buffer with data
  if (data)
  {
    void* bufferData = map();
    if (!bufferData)
    {
      valid = false;
      return;
    }

    memcpy(bufferData, data, size);
    unmap();
  }
}

void Buffer::destroy() const
{
  vkFreeMemory(device, deviceMemory, nullptr);
  vkDestroyBuffer(device, buffer, nullptr);
}

bool Buffer::copyTo(const Buffer& target, VkCommandBuffer commandBuffer, VkQueue queue) const
{
  VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
  {
    return false;
  }

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, buffer, target.getBuffer(), 1u, &copyRegion);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
  {
    return false;
  }

  VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
  submitInfo.commandBufferCount = 1u;
  submitInfo.pCommandBuffers = &commandBuffer;
  if (vkQueueSubmit(queue, 1u, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
  {
    return false;
  }

  if (vkQueueWaitIdle(queue) != VK_SUCCESS)
  {
    return false;
  }

  return true;
}

void* Buffer::map() const
{
  void* data;
  VkResult result = vkMapMemory(device, deviceMemory, 0u, size, 0, &data);
  if (result != VK_SUCCESS)
  {
    return nullptr;
  }

  return data;
}

void Buffer::unmap() const
{
  vkUnmapMemory(device, deviceMemory);
}

bool Buffer::isValid() const
{
  return valid;
}

VkBuffer Buffer::getBuffer() const
{
  return buffer;
}