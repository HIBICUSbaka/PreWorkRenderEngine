﻿//---------------------------------------------------------------
// File: RSLight.h
// Proj: RenderSystem_DX11
// Info: 对一个光源的基本内容进行描述及相关处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSLight.h"
#include "RSCamerasContainer.h"
#include "RSRoot_DX11.h"
#include "RSResourceManager.h"
#include "RSDrawCallsPool.h"
#include "RSMeshHelper.h"
#include "RSCamera.h"

RSLight::RSLight(LIGHT_INFO* _info) :
    mLightType(_info->mType),
    mWithShadow(_info->mWithShadow),
    mLightStrength(_info->mStrength),
    mLightDirection(_info->mDirection),
    mLightPosition(_info->mPosition),
    mLightFallOffStart(_info->mFalloffStart),
    mLightFallOffEnd(_info->mFalloffEnd),
    mLightSpotPower(_info->mSpotPower),
    mRSLightInfo({
        mLightStrength, mLightFallOffStart, mLightDirection,
        mLightFallOffEnd, mLightPosition, mLightSpotPower
        }),
    mRSLightCamera(nullptr), mBloomLightFlg(false),
    mLightMeshData({}), mLightInstanceData({}),
    mLightDrawCallData({})
{

}

RSLight::~RSLight()
{

}

RS_LIGHT_INFO* RSLight::GetRSLightInfo()
{
    return &mRSLightInfo;
}

LIGHT_TYPE RSLight::GetRSLightType()
{
    return mLightType;
}

void RSLight::ResetRSLight(LIGHT_INFO* _info)
{
    mLightType = _info->mType;
    SetRSLightStrength(_info->mStrength);
    SetRSLightDirection(_info->mDirection);
    SetRSLightPosition(_info->mPosition);
    SetRSLightFallOff(_info->mFalloffStart, _info->mFalloffEnd);
    SetRSLightSpotPower(_info->mSpotPower);
}

void RSLight::SetRSLightStrength(DirectX::XMFLOAT3 _strength)
{
    mLightStrength = _strength;
    mRSLightInfo.mStrength = _strength;
}

void RSLight::SetRSLightDirection(DirectX::XMFLOAT3 _direction)
{
    mLightDirection = _direction;
    mRSLightInfo.mDirection = _direction;
}

void RSLight::SetRSLightPosition(DirectX::XMFLOAT3 _position)
{
    mLightPosition = _position;
    mRSLightInfo.mPosition = _position;

    if (mRSLightCamera)
    {
        mRSLightCamera->ChangeRSCameraPosition(_position);
    }
    if (mBloomLightFlg)
    {
        static DirectX::XMMATRIX mat = {};
        mat = DirectX::XMMatrixTranslation(
            mLightPosition.x, mLightPosition.y, mLightPosition.z);
        DirectX::XMStoreFloat4x4(&(mLightInstanceData[0].mWorldMat), mat);
    }
}

void RSLight::SetRSLightFallOff(float _start, float _end)
{
    mLightFallOffStart = _start;
    mLightFallOffEnd = _end;
    mRSLightInfo.mFalloffStart = _start;
    mRSLightInfo.mFalloffEnd = _end;
}

void RSLight::SetRSLightSpotPower(float _power)
{
    mLightSpotPower = _power;
    mRSLightInfo.mSpotPower = _power;
}

RSCamera* RSLight::CreateLightCamera(std::string& _lightName,
    CAM_INFO* _info, RSCamerasContainer* _camContainer)
{
    if (!_info || !_camContainer) { return nullptr; }

    std::string name = _lightName + "-light-cam";
    mRSLightCamera = _camContainer->CreateRSCamera(name, _info);

    return mRSLightCamera;
}

RSCamera* RSLight::GetRSLightCamera()
{
    return mRSLightCamera;
}

void RSLight::SetLightBloom(RS_SUBMESH_DATA& _meshData)
{
    mBloomLightFlg = true;
    mLightMeshData = _meshData;
    mLightInstanceData.resize(1);
    mLightInstanceData[0].mCustomizedData1.x =
        mLightStrength.x * 1.6f;
    mLightInstanceData[0].mCustomizedData1.y =
        mLightStrength.y * 1.6f;
    mLightInstanceData[0].mCustomizedData1.z =
        mLightStrength.z * 1.6f;
    mLightInstanceData[0].mCustomizedData1.w = 1.f;
    static DirectX::XMMATRIX mat = {};
    mat = DirectX::XMMatrixTranslation(
        mLightPosition.x, mLightPosition.y, mLightPosition.z);
    DirectX::XMStoreFloat4x4(&(mLightInstanceData[0].mWorldMat),
        mat);
    mLightDrawCallData.mInstanceData.mDataPtr = &mLightInstanceData;
    mLightDrawCallData.mMeshData.mIndexBuffer =
        mLightMeshData.mIndexBuffer;
    mLightDrawCallData.mMeshData.mVertexBuffer =
        mLightMeshData.mVertexBuffer;
    mLightDrawCallData.mMeshData.mIndexCount =
        mLightMeshData.mIndexCount;
    mLightDrawCallData.mMeshData.mLayout =
        mLightMeshData.mLayout;
    mLightDrawCallData.mMeshData.mTopologyType =
        mLightMeshData.mTopologyType;
}

void RSLight::UpdateBloomColor()
{
    if (mBloomLightFlg)
    {
        mLightInstanceData[0].mCustomizedData1.x =
            mLightStrength.x * 1.6f;
        mLightInstanceData[0].mCustomizedData1.y =
            mLightStrength.y * 1.6f;
        mLightInstanceData[0].mCustomizedData1.z =
            mLightStrength.z * 1.6f;
        mLightInstanceData[0].mCustomizedData1.w = 1.f;
    }
}

void RSLight::UploadLightDrawCall()
{
    static auto pool = GetRSRoot_DX11_Singleton()->
        DrawCallsPool();
    if (mBloomLightFlg)
    {
        pool->AddDrawCallToPipe(DRAWCALL_TYPE::LIGHT,
            mLightDrawCallData);
    }
}

void RSLight::ReleaseLightBloom(bool _deleteByFrame)
{
    if (mBloomLightFlg && !_deleteByFrame)
    {
        GetRSRoot_DX11_Singleton()->MeshHelper()->ReleaseSubMesh(
            mLightMeshData);
    }
}

DirectX::XMFLOAT4X4* RSLight::GetLightWorldMat()
{
    if (!mLightInstanceData.size()) { return nullptr; }
    return &(mLightInstanceData[0].mWorldMat);
}
