//	LOOKAT-1.2:
//	* Basic class that holds all the abstractions a DirectX project needs.
//	* Allows easy communication with the main() application.
//	* Controls application flow: see OnInit(), OnUpdate(), etc..
//	* Receives I/O (keyboard) inputs: see OnKeyDown(), etc..
//	* Main access point for the device resources required by D3D12. See DeviceResources.h.
//	* Holds all the D3D12 abstractions: swapchain, command list, command queue etc..

#pragma once

#include "DXProjectHelper.h"
#include "Win32Application.h"
#include "DeviceResources.h"

class DXProject : public DX::IDeviceNotify
{
public:
	DXProject(UINT width, UINT height, std::wstring name);
    virtual ~DXProject();

	// LOOKAT-1.2: Application flow functions.
    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized) = 0;
    virtual void OnDestroy() = 0;

    // LOOKAT-1.2: DXProject override the event handlers to handle specific messages.
    virtual void OnKeyDown(UINT8 /*key*/) {}
    virtual void OnKeyUp(UINT8 /*key*/) {}
    virtual void OnWindowMoved(int /*x*/, int /*y*/) {}
    virtual void OnMouseMove(UINT /*x*/, UINT /*y*/) {}
    virtual void OnLeftButtonDown(UINT /*x*/, UINT /*y*/) {}
    virtual void OnLeftButtonUp(UINT /*x*/, UINT /*y*/) {}
    virtual void OnDisplayChanged() {}

    // Overridable members.
    virtual void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

    // Accessors.
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    const WCHAR* GetTitle() const { return m_title.c_str(); }
    RECT GetWindowsBounds() const { return m_windowBounds; }
    virtual IDXGISwapChain* GetSwapchain() { return nullptr; }
    DX::DeviceResources* GetDeviceResources() const { return m_deviceResources.get(); }

    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void SetWindowBounds(int left, int top, int right, int bottom);
    std::wstring GetAssetFullPath(LPCWSTR assetName);

protected:
    void SetCustomWindowText(LPCWSTR text);

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    // Window bounds
    RECT m_windowBounds;
    
    // Override to be able to start without Dx11on12 UI for PIX. PIX doesn't support 11 on 12. 
    bool m_enableUI;

    // LOOKAT-1.2: A DXProject has access to the D3D device resources.
    UINT m_adapterIDoverride;
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;
};
