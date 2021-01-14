#ifndef VULKAN_BOILERPLATE_H
#define VULKAN_BOILERPLATE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <optional>

#include <cstring>
#include <cstdlib>

#ifdef NDEBUG
  const bool enable_validation_layers = false;
#else
	const bool enable_validation_layers = true;
#endif

struct queue_family_indices_t
{
	std::optional<uint32_t> graphics_family;

	bool is_complete();
};

queue_family_indices_t find_queue_families(VkPhysicalDevice device);

std::vector<const char*> get_required_extensions();

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

bool device_is_suitable(VkPhysicalDevice device);

/* Follows vulkan style b/c it's part of
 * vulkan api */
VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pcreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT DebugMessenger,
		const VkAllocationCallbacks* pAllocator);

bool check_validation_layer_support(const std::vector<const char*> validation_layers);
#endif /* !VULKAN_BOILERPLATE_H */
