#include "vulkan_boilerplate.h"

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

/* Globals to enable vulkan's valication layer */
const std::vector<const char*> validation_layers =
{
	"VK_LAYER_KHRONOS_validation"
};


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
	VkQueue graphics_queue;
	VkDevice device;
	VkPhysicalDevice physical_device;
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
		create_instance(); // Internal function to handle vulkan bookend
		setup_debug_messenger();
		pick_physical_device();
		create_logical_device();
	}

	void create_instance()
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
	void setup_debug_messenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info;
		if(!enable_validation_layers) return;

		populate_dbg_msgr_create_info(create_info);

		if (CreateDebugUtilsMessengerEXT(this->instance, &create_info, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to setup debug messenger!");
		}

	}

	void create_logical_device()
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

	void populate_dbg_msgr_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
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

	void pick_physical_device()
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
		vkDestroyDevice(device, nullptr);

		if(enable_validation_layers)
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
