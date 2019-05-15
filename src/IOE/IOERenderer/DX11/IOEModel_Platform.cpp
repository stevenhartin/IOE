#include <IOE/IOEExceptions/IOEException_Renderer.h>
#include <IOE/IOEExceptions/IOEException_Enum.h>

#include <IOE/IOERenderer/IOERPI.h>

#include "IOEModel_Platform.h"
#include "../IOEShader.h"
#include "../IOEVisibilityBounds.h"

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	void IOEModel_Platform::OnSetup(class IOERPI *pRPI)
	{
		m_pIndexBuffer  = nullptr;
		m_pVertexBuffer = nullptr;
		if (GetModel().GetVertexBuffer().size() <= 0 ||
			GetModel().GetIndexBuffer().size() <= 0)
		{
			THROW_IOE_RENDERER_EXCEPTION(
				"Model must have at least 1 vertex and index",
				E_ERROR_FAILED_TO_LOAD_MODEL);
		}
		ID3D11Device *pDevice(pRPI->GetPlatform().GetDevice());

		D3D11_BUFFER_DESC tVertexBufferDesc;
		tVertexBufferDesc.ByteWidth =
			static_cast<UINT>(GetModel().GetVertexBuffer().size() *
							  sizeof(GetModel().GetVertexBuffer().at(0)));
		tVertexBufferDesc.Usage				  = D3D11_USAGE_IMMUTABLE;
		tVertexBufferDesc.BindFlags			  = D3D11_BIND_VERTEX_BUFFER;
		tVertexBufferDesc.MiscFlags			  = 0;
		tVertexBufferDesc.CPUAccessFlags	  = 0;
		tVertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA tVertexSetupData;
		tVertexSetupData.pSysMem	 = &GetModel().GetVertexBuffer().at(0);
		tVertexSetupData.SysMemPitch = 0;
		tVertexSetupData.SysMemSlicePitch = 0;

		if (FAILED(pDevice->CreateBuffer(&tVertexBufferDesc, &tVertexSetupData,
										 &m_pVertexBuffer)))
		{
			THROW_IOE_RENDERER_EXCEPTION(
				"Failed to create model vertex buffer",
				E_ERROR_FAILED_TO_LOAD_MODEL);
		}

		D3D11_BUFFER_DESC tIndexBufferDesc;
		tIndexBufferDesc.ByteWidth =
			static_cast<UINT>(GetModel().GetIndexBuffer().size() *
							  sizeof(GetModel().GetIndexBuffer().at(0)));
		tIndexBufferDesc.Usage				 = D3D11_USAGE_IMMUTABLE;
		tIndexBufferDesc.BindFlags			 = D3D11_BIND_INDEX_BUFFER;
		tIndexBufferDesc.CPUAccessFlags		 = 0;
		tIndexBufferDesc.MiscFlags			 = 0;
		tIndexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA tIndexSetupData;
		tIndexSetupData.pSysMem			 = &GetModel().GetIndexBuffer().at(0);
		tIndexSetupData.SysMemPitch		 = 0;
		tIndexSetupData.SysMemSlicePitch = 0;

		if (FAILED(pDevice->CreateBuffer(&tIndexBufferDesc, &tIndexSetupData,
										 &m_pIndexBuffer)))
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create model index buffer",
										 E_ERROR_FAILED_TO_LOAD_MODEL);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModel_Platform::OnRender(IOERPI *pRPI,
									 bool bSetShaderVariables /*=true*/)
	{
		if (m_pVertexBuffer != nullptr && m_pIndexBuffer != nullptr)
		{
			ID3D11DeviceContext *pDeviceContext(
				pRPI->GetPlatform().GetDeviceContext());

			UINT uOffset(0);
			UINT uStride(static_cast<UINT>(sizeof(ModelVertex)));
			pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer.p,
											   &uStride, &uOffset);
			pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.get(),
											 DXGI_FORMAT_R32_UINT, 0);

			for (std::int32_t nIdx(0); nIdx < GetModel().GetNumSubsets();
				 ++nIdx)
			{
				const MeshIndex &rIndex(GetModel().GetSubset(nIdx));

				// if (bSetShaderVariables)
				{
					rIndex.m_pMaterial->SetAllVariables(pRPI);
				}

				pRPI->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);
				pDeviceContext->DrawIndexed(rIndex.m_nNumIndices,
											rIndex.m_nStartIdx, 0);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModel_Platform::OnRender(IOERPI *pRPI,
									 IOEVisibilityLink *pHead,
									 bool bSetShaderVariables /*=true*/)
	{
		if (m_pVertexBuffer != nullptr && m_pIndexBuffer != nullptr)
		{
			ID3D11DeviceContext *pDeviceContext(
				pRPI->GetPlatform().GetDeviceContext());

			UINT uOffset(0);
			UINT uStride(static_cast<UINT>(sizeof(ModelVertex)));
			pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer.p,
											   &uStride, &uOffset);
			pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.get(),
											 DXGI_FORMAT_R32_UINT, 0);

			while (pHead != nullptr)
			{
				const MeshIndex &rIndex(*(pHead->pMeshIndex));

				// if (bSetShaderVariables)
				{
					rIndex.m_pMaterial->SetAllVariables(pRPI);
				}

				pRPI->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);
				pDeviceContext->DrawIndexed(rIndex.m_nNumIndices,
											rIndex.m_nStartIdx, 0);

				pHead = pHead->pNext;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE