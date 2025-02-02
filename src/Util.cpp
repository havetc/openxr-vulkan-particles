#include "Util.h"

#include <glm/gtx/quaternion.hpp>

#include <boxer/boxer.h>

#include <cstring>
#include <fstream>
#include <sstream>

void util::error(Error error, const std::string& details)
{
  std::stringstream s;

  switch (error)
  {
  case Error::FeatureNotSupported:
    s << "Required feature is not supported";
    break;
  case Error::FileMissing:
    s << "Failed to find file";
    break;
  case Error::GenericGLFW:
    s << "Program encountered a generic GLFW error";
    break;
  case Error::GenericOpenXR:
    s << "Program encountered a generic OpenXR error";
    break;
  case Error::GenericVulkan:
    s << "Program encountered a generic Vulkan error";
    break;
  case Error::HeadsetNotConnected:
    s << "No headset detected.\nPlease make sure that your headset is connected and running";
    break;
  case Error::ModelLoadingFailure:
    s << "Failed to load model";
    break;
  case Error::OutOfMemory:
    s << "Program ran out of memory";
    break;
  case Error::VulkanNotSupported:
    s << "Vulkan is not supported";
    break;
  case Error::WindowFailure:
    s << "Failed to create window";
    break;
  }

  if (details.empty())
  {
    s << ".";
  }
  else
  {
    s << ": " << details << ".";
  }

  boxer::show(s.str().c_str(), "Error", boxer::Style::Error);
}

bool util::loadXrExtensionFunction(XrInstance instance, const std::string& name, PFN_xrVoidFunction* function)
{
  const XrResult result = xrGetInstanceProcAddr(instance, name.c_str(), function);
  if (XR_FAILED(result))
  {
    return false;
  }

  return true;
}

PFN_vkVoidFunction util::loadVkExtensionFunction(VkInstance instance, const std::string& name)
{
  return vkGetInstanceProcAddr(instance, name.c_str());
}

std::vector<const char*> util::unpackExtensionString(const std::string& string)
{
  std::vector<const char*> out;
  std::istringstream stream(string);
  std::string extension;
  while (getline(stream, extension, ' '))
  {
    const size_t len = extension.size() + 1u;
    char* str = new char[len];
    memcpy(str, extension.c_str(), len);
    out.push_back(str);
  }

  return out;
}

bool util::loadShaderFromFile(VkDevice device, const std::string& filename, VkShaderModule& shaderModule)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    return false;
  }

  const size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> code(fileSize);
  file.seekg(0);
  file.read(code.data(), fileSize);
  file.close();

  VkShaderModuleCreateInfo shaderModuleCreateInfo;
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pNext = nullptr;
  shaderModuleCreateInfo.flags = 0u;
  shaderModuleCreateInfo.codeSize = code.size();
  shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
  if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    std::stringstream s;
    s << "compilation of shader \"" << filename << " failed\"";
    util::error(Error::GenericVulkan, s.str());

    return false;
  }

  return true;
}

bool util::findSuitableMemoryTypeIndex(VkPhysicalDevice physicalDevice,
                                       VkMemoryRequirements requirements,
                                       VkMemoryPropertyFlags properties,
                                       uint32_t& typeIndex)
{
  VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &supportedMemoryProperties);

  const VkMemoryPropertyFlags typeFilter = requirements.memoryTypeBits;
  for (uint32_t memoryTypeIndex = 0u; memoryTypeIndex < supportedMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
  {
    const VkMemoryPropertyFlags propertyFlags = supportedMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
    if (typeFilter & (1u << memoryTypeIndex) && (propertyFlags & properties) == properties)
    {
      typeIndex = memoryTypeIndex;
      return true;
    }
  }

  return false;
}

VkDeviceSize util::align(VkDeviceSize value, VkDeviceSize alignment)
{
  if (value == 0u)
  {
    return value;
  }

  return (value + alignment - 1u) & ~(alignment - 1u);
}

XrPath util::stringToPath(XrInstance instance, const std::string& string)
{
  XrPath path;
  const XrResult result = xrStringToPath(instance, string.c_str(), &path);
  if (XR_FAILED(result))
  {
    return XR_NULL_PATH;
  }

  return path;
}

bool util::createAction(XrActionSet actionSet,
                        const std::vector<XrPath>& paths,
                        const std::string& actionName,
                        const std::string& localizedActionName,
                        XrActionType type,
                        XrAction& action)
{
  XrActionCreateInfo actionCreateInfo{ XR_TYPE_ACTION_CREATE_INFO };
  actionCreateInfo.actionType = type;
  actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(paths.size());
  actionCreateInfo.subactionPaths = paths.data();

  memcpy(actionCreateInfo.actionName, actionName.data(), actionName.length() + 1u);
  memcpy(actionCreateInfo.localizedActionName, localizedActionName.data(), localizedActionName.length() + 1u);

  XrResult result = xrCreateAction(actionSet, &actionCreateInfo, &action);
  if (XR_FAILED(result))
  {
    return false;
  }

  return true;
}

XrPosef util::makeIdentity()
{
  XrPosef identity;
  identity.position = { 0.0f, 0.0f, 0.0f };
  identity.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
  return identity;
}

glm::mat4 util::poseToMatrix(const XrPosef& pose)
{
  const glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(pose.position.x, pose.position.y, pose.position.z));

  const glm::mat4 rotation =
    glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));

  return translation * rotation;
}

glm::mat4 util::createProjectionMatrix(XrFovf fov, float nearClip, float farClip)
{
  const float l = glm::tan(fov.angleLeft);
  const float r = glm::tan(fov.angleRight);
  const float d = glm::tan(fov.angleDown);
  const float u = glm::tan(fov.angleUp);

  const float w = r - l;
  const float h = d - u;

  glm::mat4 projectionMatrix;
  projectionMatrix[0] = { 2.0f / w, 0.0f, 0.0f, 0.0f };
  projectionMatrix[1] = { 0.0f, 2.0f / h, 0.0f, 0.0f };
  projectionMatrix[2] = { (r + l) / w, (u + d) / h, -(farClip + nearClip) / (farClip - nearClip), -1.0f };
  projectionMatrix[3] = { 0.0f, 0.0f, -(farClip * (nearClip + nearClip)) / (farClip - nearClip), 0.0f };
  return projectionMatrix;
}

bool util::updateActionStatePose(XrSession session, XrAction action, XrPath path, XrActionStatePose& state)
{
  XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
  actionStateGetInfo.action = action;
  actionStateGetInfo.subactionPath = path;

  const XrResult result = xrGetActionStatePose(session, &actionStateGetInfo, &state);
  if (XR_FAILED(result))
  {
    return false;
  }

  return true;
}

bool util::updateActionStateFloat(XrSession session, XrAction action, XrPath path, XrActionStateFloat& state)
{
  XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
  actionStateGetInfo.action = action;
  actionStateGetInfo.subactionPath = path;

  const XrResult result = xrGetActionStateFloat(session, &actionStateGetInfo, &state);
  if (XR_FAILED(result))
  {
    return false;
  }

  return true;
}