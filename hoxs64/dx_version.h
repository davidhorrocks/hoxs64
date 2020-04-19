#pragma once

#define DIRECTSOUND_VERSION 0x0700
#define DIRECTINPUT_VERSION 0x0700

#include <d3d11_1.h>
#include <dxgi1_5.h>
#include <d2d1.h>
#include <dinput.h>
#include <dsound.h>
#include <DirectXTK/WICTextureLoader.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);