#include "vulkan_boilerplate.h"

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

/*
 * Should only manage window event loop and
 * handle top level bookends. For coherency this should never
 * directly call any vulkanapi functions.
 */
struct Hello_Triangle_App
{
	Vk_Wrapper instance;
	GLFWwindow* window;

	void run()
	{
		this->init_window();
		this->instance.init();
		this->mainLoop();
		this->instance.cleanup();
		this->cleanup();
	}

	void mainLoop()
	{
		/* Simple event loop */
		while(!glfwWindowShouldClose(this->window))
		{
			glfwPollEvents();
		}
	}

	void init_window()
	{
		glfwInit(); // Init GLFW
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not initialize an opengl context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Tell GLFW to not automatically resize the window
		this->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void cleanup()
	{
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

};

int main()
{
	Hello_Triangle_App app;
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
