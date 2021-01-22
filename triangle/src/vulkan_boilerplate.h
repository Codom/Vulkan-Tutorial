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

#ifdef DEBUG
	const bool enable_validation_layers = true;
#else
  const bool enable_validation_layers = false;
#endif

struct queue_family_indices_t
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool is_complete();
};

struct swap_chain_support_details_t;

/* Wrap all vulkan setup inside an object
 * and selectively expose the attributes that
 * a future game my actually need to use */
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
	VkSwapchainKHR swap_chain;
	VkFormat sc_image_fmt;
	VkExtent2D sc_extent;
	VkPipelineLayout pipe_layout;
	std::vector<VkImageView> sc_image_views;
	std::vector<VkImage> sc_images;
	/* sc_images needs to be the last member
	 * in this struct, otherwise there will be stack issues*/

	/* A whole bunch of functions that ideally
	 * should be inlined but I want separated
	 * for readability. */
	void surface_init();
	void init_window();
	void create_instance();
	void setup_debug_messenger();
	void create_logical_device();
	void populate_dbg_msgr_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	void pick_physical_device();
	void create_swap_chain();
	void create_image_views();
	void create_graphics_pipeline();

	/* These functions are used to query potential devices,
	 * however they need access to surface or device properties so
	 * they execute in the same object space as the above functions
	 *
	 * Potential_TODO: These might be worth deobjectifying
	 * */
	VkShaderModule create_shader_module(const std::vector<char>& code);
	VkSurfaceFormatKHR pick_sc_surface_format(const std::vector<VkSurfaceFormatKHR>&);
	VkPresentModeKHR pick_sc_present_format(const std::vector<VkPresentModeKHR>&);
	swap_chain_support_details_t query_swap_chain_support(VkPhysicalDevice);
	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
	bool device_is_suitable(VkPhysicalDevice);
};
#endif /* !VULKAN_BOILERPLATE_H */
