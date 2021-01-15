#ifndef VULKAN_BOILERPLATE_H
#define VULKAN_BOILERPLATE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <optional>

#include <cstring>
#include <cstdlib>

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

#ifdef NDEBUG
  const bool enable_validation_layers = false;
#else
	const bool enable_validation_layers = true;
#endif

struct queue_family_indices_t
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool is_complete();
};

struct Vk_Wrapper
{
	VkInstance instance;
	VkSurfaceKHR surface;
	GLFWwindow* window;

	void init();
	void cleanup();
private:
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkDevice device;
	VkPhysicalDevice physical_device;
	VkDebugUtilsMessengerEXT debugMessenger;
	queue_family_indices_t indices;

	/* A whole bunch of functions that ideally
	 * should be inlined but I want separated
	 * for readability */
	void surface_init();
	void init_window();
	void create_instance();
	void setup_debug_messenger();
	void create_logical_device();
	void populate_dbg_msgr_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	void pick_physical_device();

	bool device_is_suitable(VkPhysicalDevice);
};
#endif /* !VULKAN_BOILERPLATE_H */
