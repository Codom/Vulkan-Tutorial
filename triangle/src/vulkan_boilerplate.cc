/*
 * vulkan_boilerplate.cc
 * Copyright (C) 2021 codom <codom@sol>
 *
 * Distributed under terms of the MIT license.
 *
 * Implements an object around the vulkan instance
 * in order to facillitate faster bare vulkan project
 * initialization.
 *
 * TODO: Setup image view
 */

#include "vulkan_boilerplate.h"
#include <cstdint>

/* Implementation types */
struct swap_chain_support_details_t
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

/* Globals */
const std::vector<const char*> validation_layers =
{
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> device_extenstions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

/* Helper functions */
queue_family_indices_t find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t queue_family_count = 0;
	queue_family_indices_t indices;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	int i = 0;
	for (const auto& queueFamily : queue_families) {
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
		if (present_support)
		{
			indices.present_family = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		if (indices.is_complete()) break;

		i++;
	}
	return indices;
}

bool check_dev_ext_support(VkPhysicalDevice device)
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

	std::set<std::string> required_extensions(device_extenstions.begin(), device_extenstions.end());

	for (const auto& extension : available_extensions)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

/* Query the instance layer properties for validation layer support */
bool check_validation_layer_support(const std::vector<const char*>& validation_layers)
{
	bool layerFound = false;
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // Will set layerCount to the count
	
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName: validation_layers)
	{

		for (const auto& layerProps: availableLayers)
		{
			if(strcmp(layerName, layerProps.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
	}
	return layerFound;
}

/* Function will generate an extension list
 * conditionally based off of whether validation layers
 * are enabled or not */
std::vector<const char*> get_required_extensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions+glfwExtensionCount);

	if (enable_validation_layers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

/* generic debug callback function that
 * hooks into the validation layers
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

/* Load the vkCreateDebugUtilsMessengerEXT function
 * from it's extension, and then execute it */
VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pcreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if(func != nullptr)
	{
		return func(instance, pcreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT DebugMessenger,
		const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if(func != nullptr)
	{
		func(instance, DebugMessenger, pAllocator);
	}
}

/* Object Functions */
bool queue_family_indices_t::is_complete()
{
	return graphics_family.has_value() && present_family.has_value();
}


VkExtent2D Vk_Wrapper::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	} else
	{
		int width, height;
		glfwGetFramebufferSize(this->window, &width, &height);
		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actual_extent.width = std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actual_extent.height));
		return actual_extent;
	}
}

bool Vk_Wrapper::device_is_suitable(VkPhysicalDevice device)
{
	auto m_indices = find_queue_families(device, this->surface);
	/* Query for extension support */
	bool extension_supported = check_dev_ext_support(device);
	/* Query for swap chain support */
	bool swap_chain = false;
	if(extension_supported)
	{
		swap_chain_support_details_t sc_support = query_swap_chain_support(device);
		swap_chain = !sc_support.formats.empty() && !sc_support.present_modes.empty();
	}
	return m_indices.is_complete() && extension_supported && swap_chain;
}

/* Queries device for it's supported swap chain formats and present modes */
swap_chain_support_details_t Vk_Wrapper::query_swap_chain_support(VkPhysicalDevice device)
{
	swap_chain_support_details_t details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

	/* Query for swap chain formats */
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &format_count, nullptr);
	if(format_count != 0)
	{
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
				device,
				this->surface,
				&format_count,
				details.formats.data());
	}

	/* Query for swap chain present modes */
	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &present_mode_count, nullptr);
	if(present_mode_count != 0)
	{
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				this->surface,
				&present_mode_count,
				details.present_modes.data());
	}

	return details;
}

VkSurfaceFormatKHR Vk_Wrapper::pick_sc_surface_format(const std::vector<VkSurfaceFormatKHR>& available_fmts)
{
	for (const auto& available_fmt : available_fmts)
	{
		if (available_fmt.format == VK_FORMAT_B8G8R8_SRGB
				&& available_fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return available_fmt;
		}
	}
	return available_fmts[0];
}

VkPresentModeKHR Vk_Wrapper::pick_sc_present_format(const std::vector<VkPresentModeKHR>& pres_modes)
{
	for (const auto& pres_mode : pres_modes)
	{
		if (pres_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return pres_mode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR; // Guarunteed to be available
}

void Vk_Wrapper::init()
{
	this->init_window();
	this->create_instance(); // Internal function to handle vulkan bookend
	this->setup_debug_messenger();
	this->surface_init();
	this->pick_physical_device();
	this->create_logical_device();
	this->create_swap_chain();
}

void Vk_Wrapper::surface_init()
{
	if (glfwCreateWindowSurface(
		this->instance,
		this->window,
		nullptr,
		&this->surface
		) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to make surface");
	}
}

void Vk_Wrapper::init_window()
{
	glfwInit(); // Init GLFW
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not initialize an opengl context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Tell GLFW to not automatically resize the window
	this->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void Vk_Wrapper::cleanup()
{
	vkDestroySwapchainKHR(this->device, this->swap_chain, nullptr);
	vkDestroyDevice(this->device, nullptr);
	if(enable_validation_layers)
	{
		DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
	glfwDestroyWindow(this->window);
	vkDestroyInstance(this->instance, nullptr);
	glfwTerminate();
}

void Vk_Wrapper::create_instance()
{
	if (enable_validation_layers && !check_validation_layer_support(validation_layers))
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	/* Vulkan takes in structs as arguments instead of arg lists,
	 * this first struct defines application data */
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0); // Probably best to use build system defines here, but I'm not using a BS
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1,0,0); // Ditto from above
	appInfo.apiVersion = VK_API_VERSION_1_2; // Why version 1?

	/* This second struct defines global extension info */
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &appInfo;

	auto extensions = get_required_extensions();
	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

	/* Setup instance constructor/destructor debugging */
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
	if(enable_validation_layers)
	{
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();

		populate_dbg_msgr_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	/* Finally the vulkan instance creation */
	VkResult result = vkCreateInstance(&create_info, nullptr, &this->instance);
	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vk instance!");
	}
}

/* Tells vulkan validation layers where and when
 * to use the callback function defined earlier in
 * this file */
void Vk_Wrapper::setup_debug_messenger()
{
	VkDebugUtilsMessengerCreateInfoEXT create_info;
	if(!enable_validation_layers) return;

	populate_dbg_msgr_create_info(create_info);

	if (CreateDebugUtilsMessengerEXT(this->instance, &create_info, nullptr, &this->debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to setup debug messenger!");
	}

}

void Vk_Wrapper::create_logical_device()
{
	queue_family_indices_t indices = find_queue_families(physical_device, this->surface);

	std::set<uint32_t> unique_queue_families = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	float queue_priority = 1.0f;
	for (uint32_t queue_family_index : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family_index;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	// Device feature struct, linked in the device_create_info struct below
	// For now we just want to instance the lib so no feature requests
	VkPhysicalDeviceFeatures device_features{};

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extenstions.size());
	device_create_info.ppEnabledExtensionNames = device_extenstions.data();

	// Potential issue: Use validation layers anyway for backwards compatability
	if (enable_validation_layers)
	{
		device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		device_create_info.ppEnabledLayerNames = validation_layers.data();
	} else {
		device_create_info.enabledLayerCount = 0;
	}

	if(vkCreateDevice(this->physical_device, &device_create_info, nullptr, &this->device) !=VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}

	vkGetDeviceQueue(this->device, indices.present_family.value(), 0, &this->graphics_queue);
	vkGetDeviceQueue(this->device, indices.present_family.value(), 0, &this->present_queue);
}

void Vk_Wrapper::populate_dbg_msgr_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
	/* Struct contains callback and messenger details */
	create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;
	create_info.pUserData = nullptr;
}

void Vk_Wrapper::pick_physical_device()
{
	uint32_t deviceCount = 0;
	this->physical_device = VK_NULL_HANDLE;

	vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
	if(deviceCount == 0 )
	{
		throw std::runtime_error("failed to find GPUs with vulkan support");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());
	
	for (const auto& device : devices)
	{
		if(this->device_is_suitable(device))
		{
			this->physical_device = device;
			break;
		}
	}

	if (this->physical_device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Vk_Wrapper::create_swap_chain()
{
	/* Functions to select format options for the SC */
	swap_chain_support_details_t sc_support = query_swap_chain_support(this->physical_device);

	VkSurfaceFormatKHR surface_fmt = pick_sc_surface_format(sc_support.formats);
	VkPresentModeKHR present_mode = pick_sc_present_format(sc_support.present_modes);
	VkExtent2D extent = choose_swap_extent(sc_support.capabilities);

	/* Select the depth of the SC */
	uint32_t image_cnt = sc_support.capabilities.minImageCount + 1;
	/* If there is a maximum, and we've (somehow) exceeded it, use that instead */
	if(sc_support.capabilities.maxImageCount > 0 && image_cnt > sc_support.capabilities.maxImageCount)
	{
		image_cnt = sc_support.capabilities.maxImageCount;
	}

	/* Actual struct message */
	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = this->surface;
	create_info.minImageCount = image_cnt;
	create_info.imageFormat = surface_fmt.format;
	create_info.imageColorSpace = surface_fmt.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	/* Handle potentially differing graphics and presentation queues */
	queue_family_indices_t indices = find_queue_families(this->physical_device, this->surface);
	uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};
	
	if(indices.graphics_family != indices.present_family)
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}

	create_info.preTransform = sc_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	
	/* Actually create the swap chain */
	if (vkCreateSwapchainKHR(this->device, &create_info, nullptr, &this->swap_chain)
			!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(this->device, this->swap_chain, &image_cnt, nullptr);
	this->sc_images.resize(image_cnt);
	vkGetSwapchainImagesKHR(this->device, this->swap_chain, &image_cnt, this->sc_images.data());

	this->sc_image_fmt = surface_fmt.format;
	this->sc_extent = extent;
}
