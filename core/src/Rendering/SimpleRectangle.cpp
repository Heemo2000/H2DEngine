#pragma once
#include <iostream>
#include "SimpleRectangle.h"
#include <stddef.h>


H2D::SimpleRectangle::SimpleRectangle(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
{

}

void H2D::SimpleRectangle::Run()
{
	Setup();
	DrawLoop();
	Cleanup();
}

void H2D::SimpleRectangle::SetResolution(uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;
}

void H2D::SimpleRectangle::SetUpdateSwapchainsFlagToTrue()
{
	m_EnableValidationLayers = true;
}

void H2D::SimpleRectangle::Setup()
{
	//Window creation

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW Library!" << std::endl;
		exit(1);
		return;
	}

	if (!glfwVulkanSupported())
	{
		std::cout << "GLFW does not Vulkan at this point!!" << std::endl;
		exit(1);
		return;
	}

	//Since GLFW is specifically made for OpenGL and not for Vulkan, we will make sure that GLFW doesn't initialize itself for OpenGL.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(m_Width, m_Height, "Simple Rectangle", nullptr, nullptr);
	if (!m_Window)
	{
		std::cout << "Failed to initialize a window!" << std::endl;
		exit(1);
		return;
	}

	glfwSetWindowUserPointer(m_Window, this);

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			auto app = reinterpret_cast<SimpleRectangle*>(glfwGetWindowUserPointer(window));
			app->SetResolution(width, height);
			app->SetUpdateSwapchainsFlagToTrue();
		});



	//Instance creation and activating validation layers when on debug mode.
	#if NDEBUG
		m_EnableValidationLayers = false;
	#else
		m_EnableValidationLayers = true;
	#endif

	if (m_EnableValidationLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

		for (const char* validationLayer : m_ValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layer : layers)
			{
				if (strcmp(validationLayer, layer.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				std::cout << "Validation layers requested but not found!!" << std::endl;
				exit(1);
				return;
			}
		}
	}

	VkApplicationInfo appInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "H2D Engine",
		.engineVersion = VK_MAKE_API_VERSION(0,1,0,0),
		.apiVersion = VK_API_VERSION_1_3
	};

	const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

		.pfnUserCallback = DebugCallback,
		.pUserData = this
	};

	uint32_t instanceExtensionsCount = 0;
	const char** instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionsCount);

	std::vector<const char*> extensions(instanceExtensions, instanceExtensions + instanceExtensionsCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VkInstanceCreateInfo instanceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,

		.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size()),
		.ppEnabledLayerNames = m_ValidationLayers.data(),

		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

	Check(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance), "Instance created successfully", "Instance creation failed!");

	Check(CreateDebugUtilsMessengerEXT(m_Instance, &debugMessengerCreateInfo, nullptr, &m_DebugMessenger), "Debug Messenger Created successfully", "Failed to create debug messenger!");

	//Now we will find all the availaible GPUs in a computer.

	uint32_t deviceCount = 0;
	Check(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr), "Found some GPU devices.", "Not able to find any GPU!");

	m_PhysicalDevices.resize(deviceCount);

	Check(vkEnumeratePhysicalDevices(m_Instance, &deviceCount, m_PhysicalDevices.data()), "Found some GPU devices.", "Not able to find any GPU!");

	for (int i = 0; i < deviceCount; i++)
	{
		VkPhysicalDeviceProperties2 deviceProp
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		};

		vkGetPhysicalDeviceProperties2(m_PhysicalDevices[i], &deviceProp);

		std::cout << "Device: " << deviceProp.properties.deviceName << std::endl;
	}

	for (int i = 0; i < deviceCount; i++)
	{
		VkPhysicalDeviceProperties2 deviceProp
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		};

		vkGetPhysicalDeviceProperties2(m_PhysicalDevices[i], &deviceProp);

		if (deviceProp.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			m_RequiredPhysicalDeviceIndex = i;
			break;
		}
	}

	if (m_RequiredPhysicalDeviceIndex == -1)
	{
		std::cout << "Can't find Dedicated GPU in your system" << std::endl;
		exit(1);
		return;
	}

	//Now, we will choose graphics queue as a gateway for sending commands to the GPU.

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevices[m_RequiredPhysicalDeviceIndex], &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevices[m_RequiredPhysicalDeviceIndex], &queueFamilyCount, queueFamilies.data());

	for (int i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_QueueFamilyIndex = i;
			break;
		}
	}

	const float queueFamilyPriorities = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = static_cast<uint32_t>(m_QueueFamilyIndex),
		.queueCount = 1,
		.pQueuePriorities = &queueFamilyPriorities
	};

	//We will also enable some Vulkan 1.2 and 1.3 Features.

	const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkPhysicalDeviceVulkan12Features vk12Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.descriptorIndexing = VK_TRUE,
		.descriptorBindingVariableDescriptorCount = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE,
		.bufferDeviceAddress = VK_TRUE
	};

	const VkPhysicalDeviceVulkan13Features vk13Features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &vk12Features,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE
	};

	const VkPhysicalDeviceFeatures deviceFeatures
	{
		.samplerAnisotropy = VK_TRUE
	};


	//We will also create logical device along the way, which will allow us to control what we want to do with the GPU.
	//Then, we will store the queue for doing some operations afterwards.

	VkDeviceCreateInfo logicalDeviceCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &vk13Features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures
	};

	Check(vkCreateDevice(m_PhysicalDevices[m_RequiredPhysicalDeviceIndex], &logicalDeviceCreateInfo, nullptr, &m_LogicalDevice), "Logical Device Created.", "Logical Device Creation Failed");

	vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndex, 0, &m_Queue);
}

void H2D::SimpleRectangle::DrawLoop()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
	}
}

void H2D::SimpleRectangle::Cleanup()
{
}

bool H2D::SimpleRectangle::Check(VkResult condition, std::string trueStatus, std::string falseStatus)
{
	if (condition != VK_SUCCESS)
	{
		std::cout << falseStatus << std::endl;
		exit(1);
		return false;
	}

	std::cout << std::endl;
	std::cout << trueStatus << std::endl;
	return true;
}

bool H2D::SimpleRectangle::Check(VkResult condition)
{
	if (condition != VK_SUCCESS)
	{
		exit(1);
		return false;
	}

	return true;
}

bool H2D::SimpleRectangle::CheckSwapchain(VkResult condition)
{
	if (condition < VK_SUCCESS)
	{
		if (condition == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_UpdateSwapchain = true;
			return true;
		}

		std::cout << "Vulkan call return an error: " << condition << std::endl;
		exit(condition);
		return false;
	}

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL H2D::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
	const bool isError = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0;
	const bool isWarning = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0;

	if (isError)
	{
		std::cout << std::endl;
		std::cout << "Error: " << callbackData->pMessage << std::endl;
		std::cout << std::endl;
	}

	if (isWarning)
	{
		std::cout << std::endl;
		std::cout << "Warning: " << callbackData->pMessage << std::endl;
		std::cout << std::endl;
	}

	return VK_FALSE;
}

VkResult H2D::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pMessenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void H2D::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, messenger, pAllocator);
	}
}
