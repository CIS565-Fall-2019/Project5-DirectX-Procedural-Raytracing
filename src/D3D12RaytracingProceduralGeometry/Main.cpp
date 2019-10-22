//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// LOOKAT-1.0: Good old application entry point! This is where it all begins.
// WinMain() is like a main() function.
// This will construct a DXProceduralProject object and pass that to Win32Application.

#include "stdafx.h"
#include "DXProceduralProject.h"

#define CPU_CODE_COMPLETE 1

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    DXProceduralProject project(1280, 720, L"DXR Project - Procedural Geometry");
#if CPU_CODE_COMPLETE
    return Win32Application::Run(&project, hInstance, nCmdShow);
#endif
}
