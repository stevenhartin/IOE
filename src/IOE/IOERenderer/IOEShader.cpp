#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEMaths/Matrix.h>

#include <IOE/IOERenderer/IOEShader.h>
#include <IOE/IOERenderer/IOEVertexFormat.h>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOEShader::IOEShader()
		: m_tPlatform(*this)
		, m_eShaderType(IOE::Renderer::EShaderType::Unknown)
		, m_pVertexFormat(nullptr)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::Compile(const IOEShaderCompilationOptions &rOptions)
	{
		if (m_pVertexFormat == nullptr)
		{
			m_pVertexFormat = IOEVertexFormatContainer::GetSingletonPtr()
								  ->GetVertexFormatShared("Default");
		}
		m_eShaderType = rOptions.eShaderType;
		m_szPath	  = rOptions.szPath;
		m_tPlatform.Compile(rOptions);
		FindShaderVariables();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::FindShaderVariables()
	{
		m_tPlatform.FindShaderVariables(m_arrConstantBuffers,
										m_arrGlobalVariables);
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderConstantBufferDescription *
	IOEShader::FindConstantBuffer(const char *szName)
	{
		for (ShaderConstantBufferDescription &rDescription :
			 m_arrConstantBuffers)
		{
			if (_strnicmp(rDescription.szName, szName, 64) == 0)
			{
				return &rDescription;
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableDescription *
	IOEShader::FindGlobalVariable(const char *szName,
								  EShaderVariableType eVariableType)
	{
		return FindGlobalVariable(szName, GetShaderCategory(eVariableType));
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableDescription *IOEShader::FindGlobalVariable(
		const char *szName, EShaderVariableCategories::Enum eVariableCategory)
	{
		if (eVariableCategory != EShaderVariableCategories::NumCategories)
		{
			for (ShaderVariableDescription &rDescription :
				 m_arrGlobalVariables[eVariableCategory])
			{
				if (_strnicmp(rDescription.szName, szName, 64) == 0)
				{
					return &rDescription;
				}
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	const ShaderVariableDescription *
	IOEShader::FindGlobalVariable(const char *szName,
								  EShaderVariableType eVariableType) const
	{
		return FindGlobalVariable(szName, GetShaderCategory(eVariableType));
	}

	//////////////////////////////////////////////////////////////////////////

	const ShaderVariableDescription *IOEShader::FindGlobalVariable(
		const char *szName,
		EShaderVariableCategories::Enum eVariableCategory) const
	{
		if (eVariableCategory != EShaderVariableCategories::NumCategories)
		{
			for (const ShaderVariableDescription &rDescription :
				 m_arrGlobalVariables[eVariableCategory])
			{
				if (_strnicmp(rDescription.szName, szName, 64) == 0)
				{
					return &rDescription;
				}
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableDescription *
	IOEShader::FindConstantBufferVariable(const char *szCBufferName,
										  const char *szName)
	{
		ShaderConstantBufferDescription *pCBufferDescription(
			FindConstantBuffer(szCBufferName));
		if (pCBufferDescription != nullptr)
		{
			return FindConstantBufferVariable(pCBufferDescription, szName);
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	ShaderVariableDescription *IOEShader::FindConstantBufferVariable(
		ShaderConstantBufferDescription *pBuffer, const char *szName)
	{
		for (ShaderVariableDescription &rDescription : pBuffer->arrVariables)
		{
			if (_strnicmp(rDescription.szName, szName, 64) == 0)
			{
				return &rDescription;
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	void *IOEShader::MapCBuffer(IOERPI *pRPI, const char *szName)
	{
		for (std::int32_t nIdx(0);
			 nIdx < ShaderConstantBufferDescription::ms_nMaxCBufferSlots;
			 ++nIdx)
		{
			if (_strnicmp(m_arrConstantBuffers[nIdx].szName, szName, 64) == 0)
			{
				return MapCBuffer(pRPI, nIdx, &m_arrConstantBuffers[nIdx]);
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	void *IOEShader::MapCBuffer(IOERPI *pRPI, std::int32_t nSlotIdx,
								ShaderConstantBufferDescription *pBuffer)
	{
		if (pBuffer != nullptr)
		{
			return m_tPlatform.MapConstantBuffers(pRPI, nSlotIdx, pBuffer);
		}
		return nullptr;
	}

	void IOEShader::UnmapCBuffer(IOERPI *pRPI, const char *szName)
	{
		for (std::int32_t nIdx(0);
			 nIdx < ShaderConstantBufferDescription::ms_nMaxCBufferSlots;
			 ++nIdx)
		{
			if (_strnicmp(m_arrConstantBuffers[nIdx].szName, szName, 64) == 0)
			{
				UnmapCBuffer(pRPI, nIdx, &m_arrConstantBuffers[nIdx]);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::UnmapCBuffer(IOERPI *pRPI,
								 std::int32_t nSlotIdx,
								 ShaderConstantBufferDescription *pBuffer)
	{
		if (pBuffer != nullptr)
		{
			m_tPlatform.UnmapConstantBuffers(pRPI, nSlotIdx, pBuffer);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::SetShaderVariableValue(void *pAddress, float fU)
	{
		(static_cast<float *>(pAddress)[0]) = fU;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::SetShaderVariableValue(void *pAddress, std::int32_t nValue)
	{
		(static_cast<std::int32_t *>(pAddress)[0]) = nValue;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::SetShaderVariableValue(void *pAddress,
										   std::uint32_t uValue)
	{
		(static_cast<std::uint32_t *>(pAddress)[0]) = uValue;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::SetShaderVariableValue(void *pAddress, float fU, float fV)
	{
		(static_cast<float *>(pAddress)[0]) = fU;
		(static_cast<float *>(pAddress)[1]) = fV;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::SetShaderVariableValue(void *pAddress, float fX, float fY,
										   float fZ)
	{
		(static_cast<float *>(pAddress)[0]) = fX;
		(static_cast<float *>(pAddress)[1]) = fY;
		(static_cast<float *>(pAddress)[2]) = fZ;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::SetShaderVariableValue(void *pAddress, float fX, float fY,
										   float fZ, float fW)
	{
		(static_cast<float *>(pAddress)[0]) = fX;
		(static_cast<float *>(pAddress)[1]) = fY;
		(static_cast<float *>(pAddress)[2]) = fZ;
		(static_cast<float *>(pAddress)[3]) = fW;
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEShader::SetShaderVariableValue(void *pAddress,
									  const IOE::Maths::IOEVector &rvVector)
	{
		float fX, fY, fZ, fW;
		rvVector.Get(fX, fY, fZ, fW);
		SetShaderVariableValue(pAddress, fX, fY, fZ, fW);
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEShader::SetShaderVariableValue(void *pAddress,
									  const IOE::Maths::IOEMatrix &rmMatrix)
	{
		float arrData[16];
		rmMatrix.Get(arrData[0], arrData[1], arrData[2], arrData[3],
					 arrData[4], arrData[5], arrData[6], arrData[7],
					 arrData[8], arrData[9], arrData[10], arrData[11],
					 arrData[12], arrData[13], arrData[14], arrData[15]);
		memcpy(static_cast<float *>(pAddress), arrData, 16 * sizeof(float));
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader::Set(IOERPI *pRPI)
	{
		// if (pRPI->GetShaderIndex(m_eShaderType) != GetResourceIndex())
		{
			m_tPlatform.Set(pRPI);

			pRPI->SetShaderIndex(m_eShaderType, GetResourceIndex());
		}
	}

	//////////////////////////////////////////////////////////////////////////

	std::uint32_t IOEShader::GetStructuredBufferSize(const char *szName) const
	{
		const ShaderVariableDescription *pDescription(FindGlobalVariable(
			szName, EShaderVariableCategories::StructuredBuffer));
		if (pDescription != nullptr)
		{
			return static_cast<std::uint32_t>(pDescription->nSizeOrSamples);
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE