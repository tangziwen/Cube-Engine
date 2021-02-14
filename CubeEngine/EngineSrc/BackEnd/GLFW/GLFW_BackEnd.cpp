﻿
#include "GLFW_BackEnd.h"
#include "../../Scene/SceneMgr.h"
#include "../RenderBackEnd.h"
#include "EngineSrc/BackEnd/AbstractDevice.h"
#include "GL/glew.h"
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "vulkan/vk_sdk_platform.h"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include <algorithm>
#include <strstream>
#include "EngineSrc/BackEnd/VkRenderBackEnd.h"

const bool g_isuesVulkan = true;
namespace tzw {

static void
error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

static void
cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  AbstractDevice::shared()->mouseMoveEvent(vec2(xpos, ypos));
}

static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  switch (action) {
    case GLFW_PRESS:
      AbstractDevice::shared()->mousePressEvent(button, vec2(x, y));
      break;
    case GLFW_RELEASE:
      AbstractDevice::shared()->mouseReleaseEvent(button, vec2(x, y));
      break;
    default:
      break;
  }
}

static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  std::string theStr = "";
  switch (action) {
    case GLFW_PRESS: {
      AbstractDevice::shared()->keyPressEvent(key);
    } break;
    case GLFW_RELEASE: {
      AbstractDevice::shared()->keyReleaseEvent(key);
    } break;
    default:
      break;
  }
  // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  //    glfwSetWindowShouldClose(window, GL_TRUE);
}

static void
char_callback(GLFWwindow*, unsigned int theChar)
{
  AbstractDevice::shared()->charInputEvent(theChar);
}

static void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  AbstractDevice::shared()->scrollEvent(xoffset, yoffset);
}



void
GLFW_BackEnd::prepare(int width, int height, bool isFullScreen)
{
	if (!glfwInit()) 
	{
		exit(EXIT_FAILURE);
	}
    bool isVulkanAvabile = false;
    if (glfwVulkanSupported())
    {
        // Vulkan is available, at least for compute
        printf("vulkan is avalable\n");
        isVulkanAvabile = true;
    }

	int w = width;
	int h = height;
	glfwWindowHint(GLFW_RESIZABLE, 0);
	GLFWmonitor * monitor;
	if(isFullScreen)
	{
		monitor = glfwGetPrimaryMonitor();
	} else
	{
		monitor = nullptr;
	}
    if(isVulkanAvabile && EngineDef::isUseVulkan)
    {
        //disable init opengl
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
	m_window = glfwCreateWindow(w, h, EngineDef::versionStr, monitor, NULL);
    if(isVulkanAvabile && EngineDef::isUseVulkan)
    {
        AbstractDevice::shared()->setRenderDevice(RenderDeviceType::Vulkan_Device);
    }else{
        AbstractDevice::shared()->setRenderDevice(RenderDeviceType::OpenGl_Device);
    }
	m_w = w;
	m_h = h;
	
    AbstractDevice::shared()->createRenderBackEnd(m_window);
	glfwSetWindowCenter(m_window);
	GLint flags;

	glfwSwapInterval(0);
	glfwSetKeyCallback(m_window, key_callback);
	glfwSetCharCallback(m_window, char_callback);
	glfwSetScrollCallback(m_window, scroll_callback);

	glfwSetMouseButtonCallback(m_window, mouse_button_callback);
	glfwSetCursorPosCallback(m_window, cursor_position_callback);
	AbstractDevice::shared()->init(w, h);

	//disable input method
	auto winHandle = glfwGetWin32Window(m_window);
	auto ime_handle = ImmAssociateContext(winHandle,0);
}

void
GLFW_BackEnd::run()
{
  while (!glfwWindowShouldClose(m_window)) {
      if(AbstractDevice::shared()->getRenderDeviceType() == RenderDeviceType::OpenGl_Device)
      {
          AbstractDevice::shared()->update();
          glfwSwapBuffers(m_window);
      }
      else{
        AbstractDevice::shared()->update();
      }
    glfwPollEvents();
  }
  glfwDestroyWindow(m_window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

GLFW_BackEnd::GLFW_BackEnd()
  : m_window(nullptr),m_w(1024),m_h(1024)
{}

void
GLFW_BackEnd::setUnlimitedCursor(bool enable)
{
  if (enable) {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void
GLFW_BackEnd::getMousePos(double* posX, double* posY)
{
  glfwGetCursorPos(m_window, posX, posY);
}

int
GLFW_BackEnd::getMouseButton(int buttonMode)
{
  return glfwGetMouseButton(m_window, buttonMode);
}

void
GLFW_BackEnd::setWinSize(int width, int height)
{
	m_w = width;
	m_h = height;
	glfwSetWindowSize(m_window, width, height);
}

void
GLFW_BackEnd::setIsFullScreen(bool isFullScreen)
{
  // glfws
  if (isFullScreen) {
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, 0);
  }
	else 
  {

  }
}

void GLFW_BackEnd::changeScreenSetting(int w, int h, bool isFullScreen)
{

	GLFWmonitor * monitor = nullptr;
    if (isFullScreen) {
    monitor = glfwGetPrimaryMonitor();
    }
    else 
    {
	    monitor = nullptr;
    }
    glfwSetWindowMonitor(m_window, monitor, 0, 0, w, h, 0);
	glfwSetWindowCenter(m_window);
}



bool
GLFW_BackEnd::glfwSetWindowCenter(GLFWwindow* window)
{
  if (!window)
    return false;

  int sx = 0, sy = 0;
  int px = 0, py = 0;
  int mx = 0, my = 0;
  int monitor_count = 0;
  int best_area = 0;
  int final_x = 0, final_y = 0;

  glfwGetWindowSize(window, &sx, &sy);
  glfwGetWindowPos(window, &px, &py);

  // Iterate throug all monitors
  GLFWmonitor** m = glfwGetMonitors(&monitor_count);
  if (!m)
    return false;

  for (int j = 0; j < monitor_count; ++j) {

    glfwGetMonitorPos(m[j], &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
    if (!mode)
      continue;

    // Get intersection of two rectangles - screen and window
    int minX = std::max(mx, px);
    int minY = std::max(my, py);

    int maxX = std::min(mx + mode->width, px + sx);
    int maxY = std::min(my + mode->height, py + sy);

    // Calculate area of the intersection
    int area = std::max(maxX - minX, 0) * std::max(maxY - minY, 0);

    // If its bigger than actual (window covers more space on this monitor)
    if (area > best_area) {
      // Calculate proper position in this monitor
      final_x = mx + (mode->width - sx) / 2;
      final_y = my + (mode->height - sy) / 2;

      best_area = area;
    }
  }

  // We found something
  if (best_area)
    glfwSetWindowPos(window, final_x, final_y);

  // Something is wrong - current window has NOT any intersection with any
  // monitors. Move it to the default one.
  else {
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
      const GLFWvidmode* desktop = glfwGetVideoMode(primary);

      if (desktop)
        glfwSetWindowPos(
          window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
      else
        return false;
    } else
      return false;
  }

  return true;
}
} // namespace tzw
