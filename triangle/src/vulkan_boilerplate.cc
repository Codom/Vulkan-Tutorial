/*
 * vulkan_boilerplate.cc
 * Copyright (C) 2021 codom <codom@sol>
 *
 * Distributed under terms of the MIT license.
 */
#include "vulkan_boilerplate.h"

/* Globals to enable vulkan's valication layer */
const std::vector<const char*> validation_layers =
{
	"VK_LAYER_KHRONOS_validation"
};

bool queue_family_indices_t::is_complete()
{
	return graphics_family.has_value() && present_family.has_value()
}

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
}


bool Vk_Wrapper::device_is_suitable(VkPhysicalDevice device)
{
	return indices.is_complete();
}

/* Query the instance layer properties for validation layer support */
bool check_validation_layer_support(const std::vector<const char*> validation_layers)
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
	}

	return extensions;
}

/* generic debug callback function that
 * hooks into the validation layers
 */
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
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
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if(func != nullptr)
	{
		func(instance, DebugMessenger, pAllocator);
	}
}

void Vk_Wrapper::init()
{
	this->init_window();
	this->create_instance(); // Internal function to handle vulkan bookend
	this->setup_debug_messenger();
	this->surface_init();
	this->pick_physical_device();
	this->create_logical_device();
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
	appInfo.apiVersion = VK_API_VERSION_1_0; // Why version 1?

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

	if (CreateDebugUtilsMessengerEXT(this->instance, &create_info, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to setup debug messenger!");
	}

}

void Vk_Wrapper::create_logical_device()
{
	this->indices = find_queue_families(physical_device, this->surface);

	std::set<uint32_t> unique_queue_families = {
		this->indices.graphics_family.value(),
		this->indices.present_family.value()
	};

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	float queue_priority = 1.0f;
	for (uint32_t queue_family_index : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info;
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
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	device_create_info.pEnabledFeatures = &device_features;
	// no need to load validation layers in the physical device
	// implementations will not differentiate between instance or
	// device validation layers
	device_create_info.enabledExtensionCount = 0;

	// Potential issue: Use validation layers anyway for backwards compatability
	// if (enable_validation_layers)
	// {
	// 	device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
	// 	device_create_info.ppEnabledExtensionNames = validation_layers.data();
	// }

	if(vkCreateDevice(physical_device, &device_create_info, nullptr, &device) !=VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}

	vkGetDeviceQueue(this->device, this->indices.present_family.value(), 0, &this->present_queue);
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
