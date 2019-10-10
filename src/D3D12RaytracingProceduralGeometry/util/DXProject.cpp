#include "stdafx.h"
#include "DXProject.h"

using namespace Microsoft::WRL;
using namespace std;

DXProject::DXProject(UINT width, UINT height, std::wstring name) :
    m_width(width),
    m_height(height),
    m_windowBounds{ 0,0,0,0 },
    m_title(name),
    m_aspectRatio(0.0f),
    m_enableUI(true),
    m_adapterIDoverride(UINT_MAX)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;

    UpdateForSizeChange(width, height);
}

DXProject::~DXProject()
{
}

void DXProject::UpdateForSizeChange(UINT clientWidth, UINT clientHeight)
{
    m_width = clientWidth;
    m_height = clientHeight;
    m_aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
}

// Helper function for resolving the full path of assets.
std::wstring DXProject::GetAssetFullPath(LPCWSTR assetName)
{
    return m_assetsPath + assetName;
}


// Helper function for setting the window's title text.
void DXProject::SetCustomWindowText(LPCWSTR text)
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}

// LOOKAT-1.2.0: (optional) add your own command line arguments here if you wish to customize how the app is initialized!
// Might be able to pass a scene file to parse here.
_Use_decl_annotations_
void DXProject::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        // -disableUI
        if (_wcsnicmp(argv[i], L"-disableUI", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/disableUI", wcslen(argv[i])) == 0)
        {
            m_enableUI = false;
        }
        // -forceAdapter [id]
        else if (_wcsnicmp(argv[i], L"-forceAdapter", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/forceAdapter", wcslen(argv[i])) == 0)
        {
            ThrowIfFalse(i + 1 < argc, L"Incorrect argument format passed in.");
            
            m_adapterIDoverride = _wtoi(argv[i + 1]);
            i++;
        }
    }

}

void DXProject::SetWindowBounds(int left, int top, int right, int bottom)
{
    m_windowBounds.left = static_cast<LONG>(left);
    m_windowBounds.top = static_cast<LONG>(top);
    m_windowBounds.right = static_cast<LONG>(right);
    m_windowBounds.bottom = static_cast<LONG>(bottom);
}