#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <optional>

#include <cstring>
#include <cstdlib>

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

/* Globals to enable vulkan's valication layer */
const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

struct queue_family_indices
{
	std::optional<uint32_t> graphics_family;

	bool is_complete()
	{
		return graphics_family.has_value();
	}
};

struct queue_family_indices find_queue_families(VkPhysicalDevice device)
{
	uint32_t queue_family_count = 0;
	struct queue_family_indices indices;

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
	struct queue_family_indices indices = find_queue_families(device);

	return indices.is_complete();
}

/* Query the instance layer properties for validation layer support */
bool checkValidationLayerSupport()
{
	bool layerFound = false;
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // Will set layerCount to the count
	
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName: validationLayers)
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
std::vector<const char*> getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions+glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

/* generic debug callback function that
 * hooks into the validation layers
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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

/*
 * If I want to query physical devices for specific features,
 * those checks would go here.
 */

class HelloTriangleApplication
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	GLFWwindow* window;

	void initWindow()
	{
		glfwInit(); // Init GLFW
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not initialize an opengl context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Tell GLFW to not automatically resize the window
		this->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance(); // Internal function to handle vulkan bookend
		setupDebugMessenger();
		pickPhysicalDevice();
	}

	void createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport())
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
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		/* Setup instance constructor/destructor debugging */
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if(enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		/* Finally the vulkan instance creation */
		VkResult result = vkCreateInstance(&createInfo, nullptr, &this->instance);
		if(result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vk instance!");
		}
	}

	/* Tells vulkan validation layers where and when
	 * to use the callback function defined earlier in
	 * this file */
	void setupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		if(!enableValidationLayers) return;

		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to setup debug messenger!");
		}

	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		/* Struct contains callback and messenger details */
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

	}

	void pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Should this be an object member? probably.

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
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void mainLoop()
	{
		/* Simple event loop */
		while(!glfwWindowShouldClose(this->window))
		{
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		if(enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
		}

		vkDestroyInstance(this->instance, nullptr);

		glfwDestroyWindow(this->window);

		glfwTerminate();
	}

};

int main()
{
	HelloTriangleApplication app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
