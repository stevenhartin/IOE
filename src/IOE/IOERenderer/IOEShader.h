#pragma once

#include <string>
#include <memory>
#include <cstdint>

#include <IOE/IOECore/IOEDefines.h>
#include "IOEShader_PlatformBase.h"
#include RPI_INCLUDE(IOEShader_Platform.h)

#include <IOE/IOECore/IOEResourceCounter.h>

namespace IOE
{
namespace Maths
{
	class IOEVector;
	class IOEMatrix;
}
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	using std::shared_ptr;
	class IOERPI;
	class IOEVertexFormat;

	//////////////////////////////////////////////////////////////////////////

	class IOEShader : public IOE::Core::IOEResourceCounter<IOEMaterial>
	{
	public:
		IOEShader();

		void Compile(const IOEShaderCompilationOptions &rOptions);

		EShaderType::Enum GetShaderType() const
		{
			return m_eShaderType;
		}

		void Set(IOERPI *pRPI);

	public:
		IOEShader_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOEShader_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

	public:
		void FindShaderVariables();
		void *MapCBuffer(IOERPI *pRPI, const char *szName);
		void *MapCBuffer(IOERPI *pRPI, std::int32_t nSlotIdx,
						 ShaderConstantBufferDescription *pBuffer);
		void UnmapCBuffer(IOERPI *pRPI, const char *szName);
		void UnmapCBuffer(IOERPI *pRPI, std::int32_t nSlotIdx,
						  ShaderConstantBufferDescription *pBuffer);

		ShaderConstantBufferDescription *
		FindConstantBuffer(const char *szName);
		ShaderVariableDescription *
		FindGlobalVariable(const char *szName,
						   EShaderVariableType eVariableType);
		ShaderVariableDescription *
		FindGlobalVariable(const char *szName,
						   EShaderVariableCategories::Enum eVariableCategory);

		const ShaderVariableDescription *
		FindGlobalVariable(const char *szName,
						   EShaderVariableType eVariableType) const;
		const ShaderVariableDescription *FindGlobalVariable(
			const char *szName,
			EShaderVariableCategories::Enum eVariableCategory) const;

		ShaderVariableDescription *
		FindConstantBufferVariable(const char *szCBufferName,
								   const char *szName);
		ShaderVariableDescription *
		FindConstantBufferVariable(ShaderConstantBufferDescription *pBuffer,
								   const char *szName);

		std::uint32_t GetStructuredBufferSize(const char *szName) const;

		void SetVertexFormat(shared_ptr<IOEVertexFormat> pVertexFormat)
		{
			m_pVertexFormat = pVertexFormat;
		}
		
		shared_ptr<IOEVertexFormat> GetVertexFormatShared()
		{
			return m_pVertexFormat;
		}
		
		IOEVertexFormat *GetVertexFormat()
		{
			return m_pVertexFormat.get();
		}

		FORCEINLINE void Dispatch(IOERPI *pRPI, int32_t nNumThreadGroupsX,
								  int32_t nNumThreadGroupsY,
								  int32_t nNumThreadGroupsZ)
		{
			IOE_ASSERT(m_eShaderType == EShaderType::Compute,
					   "Invalid shader type");

			m_tPlatform.Dispatch(pRPI, nNumThreadGroupsX, nNumThreadGroupsY,
								 nNumThreadGroupsZ);
		}

	public:
		FORCEINLINE bool SetTextureVariable(IOERPI *pRPI,
											const char *szTextureName,
											IOETexture *pTexture,
											int arrOffset = 0)
		{
			return m_tPlatform.SetTextureVariable(pRPI, szTextureName,
												  pTexture, arrOffset);
		}

		FORCEINLINE bool SetSamplerVariable(IOERPI *pRPI,
											const char *szSamplerName,
											IOESamplerState *pSampler)
		{
			return m_tPlatform.SetSamplerVariable(pRPI, szSamplerName,
												  pSampler);
		}

		FORCEINLINE bool
		SetSamplers(IOERPI *pRPI,
					const vector<pair<string, IOESamplerState*> >
						&arrSamplers)
		{
			return m_tPlatform.SetSamplers(pRPI, arrSamplers);
		}

	public:
		void SetShaderVariableValue(void *pAddress, float fU);
		void SetShaderVariableValue(void *pAddress, std::int32_t nValue);
		void SetShaderVariableValue(void *pAddress, std::uint32_t nValue);
		void SetShaderVariableValue(void *pAddress, float fU, float fV);
		void SetShaderVariableValue(void *pAddress, float fX, float fY,
									float fZ);
		void SetShaderVariableValue(void *pAddress, float fX, float fY,
									float fZ, float fW);
		void SetShaderVariableValue(void *pAddress,
									const IOE::Maths::IOEVector &rvVector);
		void SetShaderVariableValue(void *pAddress,
									const IOE::Maths::IOEMatrix &rmMatrix);

	private:
		IOEShader_Platform m_tPlatform;
		shared_ptr<IOEVertexFormat> m_pVertexFormat;

		EShaderType::Enum m_eShaderType;
		std::wstring m_szPath;

		ShaderCategories m_arrGlobalVariables;
		ShaderConstantBuffers m_arrConstantBuffers;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE