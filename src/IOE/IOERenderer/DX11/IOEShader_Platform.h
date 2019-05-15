#pragma once

#include <cstdint>
#include <array>

#include <d3d11.h>

#include <IOE/IOERenderer/IOEModel.h>
#include <IOE/IOERenderer/IOEShader_PlatformBase.h>
#include "IOEComSharedPtr.h"

struct ID3D11InputLayout;
struct ID3D11DeviceChild;
struct ID3D11ShaderReflection;
struct ID3D11Buffer;
struct _D3D11_SHADER_INPUT_BIND_DESC;
struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
typedef _D3D11_SHADER_INPUT_BIND_DESC D3D11_SHADER_INPUT_BIND_DESC;

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOERPI;

	//////////////////////////////////////////////////////////////////////////

	class IOEShader_Platform : public IOEShader_PlatformBase
	{
	public:
		IOEShader_Platform(IOEShader &rShader);

	public:
		virtual void
		Compile(const IOEShaderCompilationOptions &rOptions) override final;
		virtual void Cleanup() override final;

		virtual void Set(IOERPI *pRPI) override final;

		virtual void
		FindShaderVariables(ShaderConstantBuffers &arrConstantBuffers,
							ShaderCategories &arrDescriptions) override final;
		virtual void *MapConstantBuffers(
			IOERPI *pRPI, std::int32_t nSlotIdx,
			ShaderConstantBufferDescription *pBuffer) override final;
		virtual void UnmapConstantBuffers(
			IOERPI *pRPI, std::int32_t nSlotIdx,
			ShaderConstantBufferDescription *pBuffer) override final;

		virtual bool SetTextureVariable(IOERPI *pRPI,
										const char *szTextureName,
										IOETexture *pTexture,
										int arrOffset = 0) override final;
		virtual bool SetSamplerVariable(
			IOERPI *pRPI, const char *szSamplerName,
			struct IOESamplerState *pSamplers) override final;
		virtual bool SetSamplers(
			IOERPI *pRPI,
			const vector<pair<string, struct IOESamplerState*> >
				&arrSamplers) override final;

		virtual void Dispatch(IOERPI *pRPI, int32_t nNumThreadGroupsX,
							  int32_t nNumThreadGroupsY,
							  int32_t nNumThreadGroupsZ) override final;

	private:
		virtual bool SetShaderResourceVariable(
			IOERPI *pRPI, const char *szTextureName,
			IOETexture *pTexture, int arrOffset = 0);
		virtual bool SetBufferVariable(IOERPI *pRPI,
									   const char *szTextureName,
									   IOETexture *pTexture,
									   int arrOffset = 0);

	private:
		void CreateInputLayout();

		void FindGlobalResource(ID3D11ShaderReflection *pShaderReflection,
								int nInputType,
								EShaderVariableType eVariableType,
								ShaderCategories &arrGlobalDescriptions);

	private:
		void InsertTexture(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
						   ShaderCategories &arrGlobalDescriptions);
		void InsertSampler(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
						   ShaderCategories &arrGlobalDescriptions);
		void InsertRawBuffer(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
							 ShaderCategories &arrGlobalDescriptions);
		void InsertStructuredBuffer(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
									ShaderCategories &arrGlobalDescriptions);

	private:
		std::array<TComSharedPtr<ID3D11Buffer>,
				   ShaderConstantBufferDescription::ms_nMaxCBufferSlots>
			m_arrConstantD3D11Buffers;

	private:
		std::uint32_t m_uNumCBuffers;
		std::uint32_t m_uNumBoundResources;
		TComSharedPtr<ID3DBlob> m_pCompiledShader;
		TComSharedPtr<ID3D11DeviceChild> m_pCreatedShader;
		TComSharedPtr<ID3D11InputLayout> m_pInputLayout;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE