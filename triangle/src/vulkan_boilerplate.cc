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
	return graphics_family.has_value();
}

queue_family_indices_t find_queue_families(VkPhysicalDevice device)
{
	uint32_t queue_family_count = 0;
	queue_family_indices_t indices;

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	int i = 0;
	for (const auto& queueFamily : queue_families) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		if (indices.is_complete()) break;

		i++;
	}

	return indices;
}

bool device_is_suitable(VkPhysicalDevice device)
{
	queue_family_indices_t indices = find_queue_families(device);

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
	this->create_instance(); // Internal function to handle vulkan bookend
	this->setup_debug_messenger();
	this->pick_physical_device();
	this->create_logical_device();
}

void Vk_Wrapper::cleanup()
{
	vkDestroyDevice(this->device, nullptr);

	if(enable_validation_layers)
	{
		DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
	}

	vkDestroyInstance(this->instance, nullptr);

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
	queue_family_indices_t indices = find_queue_families(physical_device);

	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = indices.graphics_family.value();
	queue_create_info.queueCount = 1;
	// float queue_priority = 1.0f;
	// queue_create_info.pQueuePriorities = &queue_priority;

	// Device feature struct, linked in the device_create_info struct below
	// For now we just want to instance the lib so no feature requests
	VkPhysicalDeviceFeatures device_features{};

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pEnabledFeatures = &device_features;
	// no need to load validation layers in the physical device
	// implementations will not differentiate between instance or
	// device validation layers, however we do it anyway for backwards
	// compatability
	device_create_info.enabledExtensionCount = 0;
	// FIXME?
	// if (enable_validation_layers)
	// {
	// 	device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
	// 	device_create_info.ppEnabledExtensionNames = validation_layers.data();
	// }

	if(vkCreateDevice(physical_device, &device_create_info, nullptr, &device) !=VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}
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
		if(device_is_suitable(device))
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
