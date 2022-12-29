#include "Controllers.h"

#include "Context.h"
#include "Util.h"

#include <glm/mat4x4.hpp>

#include <array>
#include <cstring>

namespace
{
constexpr size_t controllerCount = 2u;

const std::string actionSetName = "actionset";
const std::string localizedActionSetName = "Actions";
const std::string actionName = "handpose";
const std::string localizedActionName = "Hand Pose";
} // namespace

Controllers::Controllers(XrInstance instance, XrSession session) : session(session)
{
  // Create an action set
  XrActionSetCreateInfo actionSetCreateInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };

  memcpy(actionSetCreateInfo.actionSetName, actionSetName.data(), actionSetName.length() + 1u);
  memcpy(actionSetCreateInfo.localizedActionSetName, localizedActionSetName.data(),
         localizedActionSetName.length() + 1u);

  XrResult result = xrCreateActionSet(instance, &actionSetCreateInfo, &actionSet);
  if (XR_FAILED(result))
  {
    util::error(Error::GenericOpenXR);
    valid = false;
    return;
  }

  // Create paths
  paths.resize(controllerCount);
  paths.at(0u) = util::stringToPath(instance, "/user/hand/left");
  paths.at(1u) = util::stringToPath(instance, "/user/hand/right");

  // Create action
  XrActionCreateInfo actionCreateInfo{ XR_TYPE_ACTION_CREATE_INFO };
  actionCreateInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
  actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(paths.size());
  actionCreateInfo.subactionPaths = paths.data();

  memcpy(actionCreateInfo.actionName, actionName.data(), actionName.length() + 1u);
  memcpy(actionCreateInfo.localizedActionName, localizedActionName.data(), localizedActionName.length() + 1u);

  result = xrCreateAction(actionSet, &actionCreateInfo, &action);
  if (XR_FAILED(result))
  {
    util::error(Error::GenericOpenXR);
    valid = false;
  }

  // Create spaces
  spaces.resize(controllerCount);
  for (size_t controllerIndex = 0u; controllerIndex < controllerCount; ++controllerIndex)
  {
    const XrPath& path = paths.at(controllerIndex);

    XrActionSpaceCreateInfo actionSpaceCreateInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    actionSpaceCreateInfo.action = action;
    actionSpaceCreateInfo.poseInActionSpace = util::makeIdentity();
    actionSpaceCreateInfo.subactionPath = path;

    result = xrCreateActionSpace(session, &actionSpaceCreateInfo, &spaces.at(controllerIndex));
    if (XR_FAILED(result))
    {
      util::error(Error::GenericOpenXR);
      valid = false;
      return;
    }
  }

  // Suggest simple controller binding (generic)
  const std::array<XrActionSuggestedBinding, 2u> bindings = {
    { { action, util::stringToPath(instance, "/user/hand/left/input/aim/pose") },
      { action, util::stringToPath(instance, "/user/hand/right/input/aim/pose") } }
  };

  XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding{
    XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING
  };
  interactionProfileSuggestedBinding.interactionProfile =
    util::stringToPath(instance, "/interaction_profiles/khr/simple_controller");
  interactionProfileSuggestedBinding.suggestedBindings = bindings.data();
  interactionProfileSuggestedBinding.countSuggestedBindings = static_cast<uint32_t>(bindings.size());

  result = xrSuggestInteractionProfileBindings(instance, &interactionProfileSuggestedBinding);
  if (XR_FAILED(result))
  {
    util::error(Error::GenericOpenXR);
    valid = false;
    return;
  }

  // Attach the controller action set
  XrSessionActionSetsAttachInfo sessionActionSetsAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
  sessionActionSetsAttachInfo.countActionSets = 1u;
  sessionActionSetsAttachInfo.actionSets = &actionSet;

  result = xrAttachSessionActionSets(session, &sessionActionSetsAttachInfo);
  if (XR_FAILED(result))
  {
    util::error(Error::GenericOpenXR);
    valid = false;
    return;
  }

  transforms.resize(controllerCount);
}

Controllers::~Controllers()
{
  for (const XrSpace& space : spaces)
  {
    xrDestroySpace(space);
  }

  xrDestroyAction(action);

  xrDestroyActionSet(actionSet);
}

bool Controllers::sync(XrSpace space, XrTime time)
{
  // Sync the actions
  XrActiveActionSet activeActionSet;
  activeActionSet.actionSet = actionSet;
  activeActionSet.subactionPath = XR_NULL_PATH; // Wildcard for all

  XrActionsSyncInfo actionsSyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
  actionsSyncInfo.countActiveActionSets = 1u;
  actionsSyncInfo.activeActionSets = &activeActionSet;

  XrResult result = xrSyncActions(session, &actionsSyncInfo);
  if (XR_FAILED(result))
  {
    util::error(Error::GenericOpenXR);
    return false;
  }

  // Update the transforms
  for (size_t controllerIndex = 0u; controllerIndex < controllerCount; ++controllerIndex)
  {
    const XrPath& path = paths.at(controllerIndex);

    XrActionStatePose actionStatePose{ XR_TYPE_ACTION_STATE_POSE };

    XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    actionStateGetInfo.action = action;
    actionStateGetInfo.subactionPath = path;

    result = xrGetActionStatePose(session, &actionStateGetInfo, &actionStatePose);
    if (XR_FAILED(result))
    {
      util::error(Error::GenericOpenXR);
      return false;
    }

    if (!actionStatePose.isActive)
    {
      continue;
    }

    XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };

    result = xrLocateSpace(spaces.at(controllerIndex), space, time, &spaceLocation);
    if (XR_FAILED(result))
    {
      util::error(Error::GenericOpenXR);
      return false;
    }

    const bool positionValid = (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
    const bool positionTracking = (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) != 0;

    const bool orientationValid = (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
    const bool orientationTracking = (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT) != 0;

    const bool active = positionValid && positionTracking && orientationValid && orientationTracking;

    if (!active)
    {
      continue;
    }

    transforms.at(controllerIndex) = util::poseToMatrix(spaceLocation.pose);
  }

  return true;
}

bool Controllers::isValid() const
{
  return valid;
}

glm::mat4 Controllers::getTransform(size_t controllerIndex) const
{
  return transforms.at(controllerIndex);
}