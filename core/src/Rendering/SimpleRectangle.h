#pragma once
#include <string>
#include <vector>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vma/vk_mem_alloc.h>

#include <glm/glm.hpp>

#include<ktx.h>

#include<ktxvulkan.h>

namespace H2D
{
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData);

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pMessenger
	);

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator
	);

	class SimpleRectangle
	{
	public:
		SimpleRectangle(uint32_t width, uint32_t height);
		void Run();
		void SetResolution(uint32_t width, uint32_t height);
		void SetUpdateSwapchainsFlagToTrue();

	private:
		void Setup();
		void DrawLoop();
		void Cleanup();

	private:
		bool Check(VkResult condition, std::string trueStatus, std::string falseStatus);
		bool Check(VkResult condition);
		bool CheckSwapchain(VkResult condition);

	private:
		uint32_t m_Width;
		uint32_t m_Height;
		bool m_EnableValidationLayers;
		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		bool m_UpdateSwapchain = false;
		GLFWwindow* m_Window;
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		std::vector<VkPhysicalDevice> m_PhysicalDevices;
		int m_RequiredPhysicalDeviceIndex = -1;
		int m_QueueFamilyIndex = -1;
		VkDevice m_LogicalDevice;
		VkQueue m_Queue;
	};
}

