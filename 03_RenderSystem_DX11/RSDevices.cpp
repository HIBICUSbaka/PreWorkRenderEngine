﻿//---------------------------------------------------------------
// File: RSDevices.cpp
// Proj: RenderSystem_DX11
// Info: 保存并提供与DirectX直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma comment(lib, "dxgi")

#include "RSDevices.h"
#include "RSRoot_DX11.h"

RSDevices::RSDevices() :
    mRootPtr(nullptr), mRenderDeivceConfig({}),
    mDriveType(D3D_DRIVER_TYPE_HARDWARE),
    mFeatureLevel(D3D_FEATURE_LEVEL_11_1),
    mDevice(nullptr), mImmediateContext(nullptr),
    mDevice1(nullptr), mImmediateContext1(nullptr),
    mDXGISwapChain(nullptr), mDXGISwapChain1(nullptr),
    mSwapChainRtv(nullptr), mFullWindowViewPort({}),
    mConcurrentCreateSupport(false), mCommandListSupport(false),
    mWndWidth(0), mWndHeight(0)
{

}

RSDevices::~RSDevices()
{

}

bool RSDevices::StartUp(RSRoot_DX11* _root, HWND _wnd)
{
    if (!_root) { return false; }

    mRootPtr = _root;

    UINT wndWidth = 1280;
    UINT wndHeight = 720;

    mWndWidth = wndWidth;
    mWndHeight = wndHeight;

    mFullWindowViewPort.Width = (FLOAT)wndWidth;
    mFullWindowViewPort.Height = (FLOAT)wndHeight;
    mFullWindowViewPort.MinDepth = 0.f;
    mFullWindowViewPort.MaxDepth = 1.f;
    mFullWindowViewPort.TopLeftX = 0.f;
    mFullWindowViewPort.TopLeftY = 0.f;

    if (!CreateDevices(_wnd, wndWidth, wndHeight))
    {
        return false;
    }

    ApplyViewPort();

    D3D11_FEATURE_DATA_THREADING threadSupport = {};
    HRESULT hr = mDevice->CheckFeatureSupport(
        D3D11_FEATURE_THREADING,
        &threadSupport, sizeof(threadSupport));
    FAIL_HR_RETURN(hr);

    if (threadSupport.DriverConcurrentCreates == TRUE)
    {
        mConcurrentCreateSupport = true;
    }
    if (threadSupport.DriverCommandLists == TRUE)
    {
        mCommandListSupport = true;
    }

    return true;
}

void RSDevices::CleanAndStop()
{
    if (mImmediateContext)
    {
        mImmediateContext->ClearState();
    }
    SAFE_RELEASE(mSwapChainRtv);
    SAFE_RELEASE(mImmediateContext1);
    SAFE_RELEASE(mImmediateContext);
    SAFE_RELEASE(mDXGISwapChain1);
    SAFE_RELEASE(mDXGISwapChain);

#ifdef _DEBUG
    ID3D11Debug* pDebug = nullptr;
    HRESULT hr = mDevice->QueryInterface(
        IID_PPV_ARGS(&pDebug));
    if (SUCCEEDED(hr))
    {
        hr = pDebug->ReportLiveDeviceObjects(
            D3D11_RLDO_DETAIL);
        pDebug->Release();
    }
#endif // _DEBUG

    SAFE_RELEASE(mDevice1);
    SAFE_RELEASE(mDevice);
}

ID3D11Device* RSDevices::GetDevice() const
{
    return mDevice;
}

ID3D11DeviceContext* RSDevices::GetSTContext() const
{
    return mImmediateContext;
}

ID3D11RenderTargetView* RSDevices::GetSwapChainRtv() const
{
    return mSwapChainRtv;
}

void RSDevices::PresentSwapChain()
{
    mDXGISwapChain->Present(0, 0);
}

bool RSDevices::CreateDevices(HWND _wnd,
    UINT _width, UINT _height)
{
    HRESULT hr = S_OK;

    UINT deviceCreateFlag = 0;
#ifdef _DEBUG
    deviceCreateFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_WARP
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    UINT numFeatLevels = ARRAYSIZE(featureLevels);
    mFeatureLevel = featureLevels[0];

    std::vector<std::pair<IDXGIAdapter*, std::wstring>> allAdapters = {};
    allAdapters.clear();
    {
        IDXGIFactory1* dxgiFactory = nullptr;
        UINT index = 0;
        IDXGIAdapter* adapter = nullptr;

        hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
        FAIL_HR_RETURN(hr);

        while (dxgiFactory->EnumAdapters(index, &adapter) !=
            DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC adapDesc = {};
            ZeroMemory(&adapDesc, sizeof(adapDesc));
            std::wstring wstr = L"";

            adapter->GetDesc(&adapDesc);
            wstr = L"--------------------\nAdapter Info in Index : " +
                std::to_wstring(index++) + L"\n";
            wstr += adapDesc.Description;
            wstr += L"\n--------------------\n";
            OutputDebugStringW(wstr.c_str());
            allAdapters.push_back({ adapter,adapDesc.Description });
        }

        index = 0;
        for (auto& adapInfo : allAdapters)
        {
            std::wstring info =
                L"--------------------\nAdapter Info in Index : " +
                std::to_wstring(index++) + L"\n" + adapInfo.second +
                L"\n--------------------\n";
        }
    }

    UINT adapterIndex = (UINT)(allAdapters.size() - 1);
    if (allAdapters[adapterIndex].second == L"Microsoft Basic Render Driver")
    {
        --adapterIndex;
    }
    if (mRenderDeivceConfig.mForceAdapterIndex != (UINT)(-1))
    {
        adapterIndex = mRenderDeivceConfig.mForceAdapterIndex;
    }

    if (adapterIndex >= 0 && adapterIndex < (UINT)allAdapters.size())
    {
        hr = D3D11CreateDevice(allAdapters[adapterIndex].first,
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr, deviceCreateFlag, featureLevels, numFeatLevels,
            D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mImmediateContext);
        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(allAdapters[adapterIndex].first,
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr, deviceCreateFlag, featureLevels + 1, numFeatLevels - 1,
                D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mImmediateContext);
        }
    }

    if (!mDevice && !mImmediateContext)
    {
        for (UINT i = 0; i < numDriverTypes; i++)
        {
            mDriveType = driverTypes[i];
            hr = D3D11CreateDevice(nullptr, mDriveType,
                nullptr, deviceCreateFlag, featureLevels,
                numFeatLevels, D3D11_SDK_VERSION,
                &mDevice, &mFeatureLevel,
                &mImmediateContext);

            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, mDriveType,
                    nullptr, deviceCreateFlag,
                    featureLevels + 1, numFeatLevels - 1,
                    D3D11_SDK_VERSION, &mDevice,
                    &mFeatureLevel, &mImmediateContext);
            }

            if (SUCCEEDED(hr)) { break; }
        }
        FAIL_HR_RETURN(hr);
    }

    if (!mDevice || !mImmediateContext) { return false; }

    IDXGIFactory1* dxgiFactory1 = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = mDevice->QueryInterface(
            IID_PPV_ARGS(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(
                    IID_PPV_ARGS(&dxgiFactory1));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    FAIL_HR_RETURN(hr);

    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory1->QueryInterface(
        IID_PPV_ARGS(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // 11.1+
        hr = mDevice->QueryInterface(
            IID_PPV_ARGS(&mDevice1));
        if (SUCCEEDED(hr))
        {
            mImmediateContext->QueryInterface(
                IID_PPV_ARGS(&mImmediateContext1));
        }
        DXGI_SWAP_CHAIN_DESC1 dc = {};
        dc.Width = _width;
        dc.Height = _height;
        dc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        dc.SampleDesc.Count = 1;
        dc.SampleDesc.Quality = 0;
        dc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        dc.BufferCount = 2;
        dc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        hr = dxgiFactory2->CreateSwapChainForHwnd(
            mDevice, _wnd,
            &dc, nullptr, nullptr, &mDXGISwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = mDXGISwapChain1->QueryInterface(
                IID_PPV_ARGS(&mDXGISwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        // 11.0
        DXGI_SWAP_CHAIN_DESC dc = {};
        dc.BufferCount = 2;
        dc.BufferDesc.Width = _width;
        dc.BufferDesc.Height = _height;
        dc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        dc.BufferDesc.RefreshRate.Numerator = 60;
        dc.BufferDesc.RefreshRate.Denominator = 1;
        dc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        dc.OutputWindow = _wnd;
        dc.SampleDesc.Count = 1;
        dc.SampleDesc.Quality = 0;
        dc.Windowed = TRUE;
        dc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        hr = dxgiFactory1->CreateSwapChain(
            mDevice, &dc, &mDXGISwapChain);
    }

    dxgiFactory1->Release();
    FAIL_HR_RETURN(hr);

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = mDXGISwapChain->GetBuffer(
        0, IID_PPV_ARGS(&pBackBuffer));
    FAIL_HR_RETURN(hr);

    hr = mDevice->CreateRenderTargetView(
        pBackBuffer, nullptr, &mSwapChainRtv);
    pBackBuffer->Release();
    FAIL_HR_RETURN(hr);

    return true;
}

void RSDevices::ApplyViewPort()
{
    mImmediateContext->RSSetViewports(1, &mFullWindowViewPort);
}

bool RSDevices::GetConcurrentCreateSupport() const
{
    return mConcurrentCreateSupport;
}

bool RSDevices::GetCommandListSupport() const
{
    return mCommandListSupport && (!mRenderDeivceConfig.mForceSingleThread);
}

UINT RSDevices::GetCurrWndWidth() const
{
    return mWndWidth;
}

UINT RSDevices::GetCurrWndHeight() const
{
    return mWndHeight;
}
