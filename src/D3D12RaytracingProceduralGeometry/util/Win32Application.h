//	LOOKAT-1.1:
//	* Class that communicates between the Windows app window and the actual DXProject.
//	* Tells the DXProject to initiate itself, update its state, render.
//	* Passes key press information to the DXProject.
//	* You shouldn't spend time here. Just understand that this is the "second" app point of contact after main() :)

#pragma once

#include "DXProject.h"

class DXProject;

class Win32Application
{
public:
    static int Run(DXProject* pProject, HINSTANCE hInstance, int nCmdShow);
    static void ToggleFullscreenWindow(IDXGISwapChain* pOutput = nullptr);
    static void SetWindowZorderToTopMost(bool setToTopMost);
    static HWND GetHwnd() { return m_hwnd; }
    static bool IsFullscreen() { return m_fullscreenMode; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
    static bool m_fullscreenMode;
    static const UINT m_windowStyle = WS_OVERLAPPEDWINDOW;
    static RECT m_windowRect;
};