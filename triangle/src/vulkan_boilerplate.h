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

struct Vk_Wrapper
{
public:
	void init();
	void cleanup();
private:
	VkQueue graphics_queue;
	VkDevice device;
	VkPhysicalDevice physical_device;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;


	void create_instance();
	void setup_debug_messenger();
	void create_logical_device();
	void populate_dbg_msgr_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	void pick_physical_device();
};
#endif /* !VULKAN_BOILERPLATE_H */
