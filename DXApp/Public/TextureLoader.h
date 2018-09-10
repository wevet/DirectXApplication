#pragma once
#include "FrameResource.h"
#include <wincodec.h>
#include <DirectXMath.h>

namespace DirectX
{
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(BYTE ** imageData, D3D12_RESOURCE_DESC & textureDesc, LPCWSTR filename, int & bytesPerRow);
	HRESULT CreateImageDataTextureFromFile(ID3D12Device * device, ID3D12GraphicsCommandList * cmdList, const wchar_t * szFileName, Microsoft::WRL::ComPtr<ID3D12Resource>& texture, Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap);
}