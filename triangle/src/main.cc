#include "vulkan_boilerplate.h"
/*
 * Should only manage window event loop and
 * handle top level bookends. For coherency this should never
 * directly call any vulkanapi functions.
 */
struct Hello_Triangle_App
{
	Vk_Wrapper vulkan;
	GLFWwindow* window;
	void run()
	{
		this->vulkan.init();
		this->window = this->vulkan.window;
		this->mainLoop();
		this->vulkan.cleanup();
	}

	void mainLoop()
	{
		/* Simple event loop */
		while(!glfwWindowShouldClose(this->window))
		{
			glfwPollEvents();
		}
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
