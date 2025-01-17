﻿//---------------------------------------------------------------
// File: RSDevices.h
// Proj: RenderSystem_DX11
// Info: 保存并提供与DirectX直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

struct RENDER_DEVICE_CONFIG
{
    UINT mForceAdapterIndex = (UINT)-1;
    bool mForceSingleThread = false;
};

class RSDevices
{
public:
    RSDevices();
    ~RSDevices();

    bool StartUp(class RSRoot_DX11* _root, HWND _wnd);
    void CleanAndStop();

    ID3D11Device* GetDevice() const;
    ID3D11DeviceContext* GetSTContext() const;
    ID3D11RenderTargetView* GetSwapChainRtv() const;

    bool GetConcurrentCreateSupport() const;
    bool GetCommandListSupport() const;

    UINT GetCurrWndWidth() const;
    UINT GetCurrWndHeight() const;

    void PresentSwapChain();

private:
    bool CreateDevices(HWND _wnd, UINT _width, UINT _height);
    void ApplyViewPort();

private:
    class RSRoot_DX11* mRootPtr;

    RENDER_DEVICE_CONFIG mRenderDeivceConfig;

    D3D_DRIVER_TYPE mDriveType;
    D3D_FEATURE_LEVEL mFeatureLevel;

    ID3D11Device* mDevice;
    ID3D11DeviceContext* mImmediateContext;
    ID3D11Device1* mDevice1;
    ID3D11DeviceContext1* mImmediateContext1;

    IDXGISwapChain* mDXGISwapChain;
    IDXGISwapChain1* mDXGISwapChain1;
    ID3D11RenderTargetView* mSwapChainRtv;
    D3D11_VIEWPORT mFullWindowViewPort;

    bool mConcurrentCreateSupport;
    bool mCommandListSupport;

    UINT mWndWidth;
    UINT mWndHeight;
};
