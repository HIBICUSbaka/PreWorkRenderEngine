﻿//---------------------------------------------------------------
// File: RSRoot_DX11.h
// Proj: RenderSystem_DX11
// Info: 保存并提供此RenderSystem相关的重要内容引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSRoot_DX11* GetRSRoot_DX11_Singleton();

class RSRoot_DX11
{
public:
    RSRoot_DX11();
    ~RSRoot_DX11();

    bool StartUp(HWND _wndHandle);
    void CleanAndStop();

    class RSDevices* Devices() const;
    class RSPipelinesManager* PipelinesManager() const;
    class RSDrawCallsPool* DrawCallsPool() const;
    class RSResourceManager* ResourceManager() const;
    class RSStaticResources* StaticResources() const;
    class RSCamerasContainer* CamerasContainer() const;
    class RSLightsContainer* LightsContainer() const;
    class RSParticlesContainer* ParticlesContainer() const;
    class RSMeshHelper* MeshHelper() const;

private:
    class RSDevices* mDevicesPtr;
    class RSPipelinesManager* mPipelinesManagerPtr;
    class RSDrawCallsPool* mDrawCallsPoolPtr;
    class RSResourceManager* mResourceManagerPtr;
    class RSStaticResources* mStaticResourcesPtr;
    class RSCamerasContainer* mCamerasContainerPtr;
    class RSLightsContainer* mLightsContainerPtr;
    class RSParticlesContainer* mParticlesContainerPtr;
    class RSMeshHelper* mMeshHelperPtr;
};
