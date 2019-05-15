#include "IOEMaterial.h"

#include <fstream>
#include <locale>
#include <codecvt>

#include <rapidxml/rapidxml.hpp>

#include <IOE/IOECore/IOEManagerContainer.h>

#include <IOE/IOEExceptions/IOEException_Renderer.h>
#include <IOE/IOEExceptions/IOEException_Enum.h>

#include <IOE/IOEMaths/Math.h>
#include <IOE/IOEMaths/Matrix.h>
#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEMaths/Camera.h>

#include <IOE/IOERenderer/IOEShader.h>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOEMaterial::IOEMaterial()
	{
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMaterial::IOEMaterial(const std::wstring &szPath)
	{
		LoadMaterial(szPath);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::LoadMaterial(const std::wstring &szPath)
	{
		std::ifstream fStream(szPath, std::ios::in);
		if (fStream.is_open())
		{
			fStream.seekg(0, std::ios::end);

			std::streamsize uNumBytes(fStream.tellg());

			// + 1 for null terminator (gets written in after we read from
			// stream).
			char *szData = new char[static_cast<std::uint32_t>(uNumBytes) + 1];

			fStream.seekg(0, std::ios::beg);
			fStream.read(&szData[0], uNumBytes);

			// The runtime converts all \r\n to \n so for every
			// newline character in the original tellg size, there
			// is 1 less byte (on NT). This actually tells us how
			// many bytes were read so we can be precise.
			std::streamsize uRead(fStream.gcount());
			fStream.close();

			szData[uRead] = '\0';

			rapidxml::xml_document<char> document;
			try
			{
				document.parse<0>(szData);
			}
			catch (rapidxml::parse_error &e)
			{
				char *szPosition = "end of document";
				if (e.where<char>() < szData + uRead)
				{
					szPosition = e.where<char>();
				}
				THROW_IOE_RENDERER_EXCEPTION_VARGS(
					"Failed to parse asset: %s at %s",
					E_ERROR_FAILED_TO_READ_MATERIAL, e.what(), szPosition);
			}
			// Parsed fine, let's validate the document
			if (document.first_node() != document.last_node())
			{
				THROW_IOE_RENDERER_EXCEPTION_VARGS(
					"There can only be 1 root not, it must be IOE. Instead, "
					"found: %s",
					E_ERROR_FAILED_TO_READ_ASSET,
					document.first_node()->name());
			}
			const auto *pRoot(document.first_node());
			for (auto *pChild = pRoot->first_node(); pChild != nullptr;
				 pChild = pChild->next_sibling())
			{
				if (_strnicmp(pChild->name(), "material",
							  pChild->name_size()) == 0)
				{
					LoadMaterial(pChild);
				}
			}

			delete[] szData;
		}
		else
		{
			THROW_IOE_RENDERER_EXCEPTION_VARGS("Failed to open material: %ls",
											   E_ERROR_FAILED_TO_READ_MATERIAL,
											   szPath.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::LoadMaterial(rapidxml::xml_node<char> *pNode)
	{
		IOE_ASSERT(_strnicmp(pNode->name(), "material", pNode->name_size()) ==
					   0,
				   "Node is not a material!");

#if (IOE_SHADER_DEBUG == IOE_ON)
		AddDefinition("IOE_DEBUG", "1");
#endif

		m_szMaterialName = pNode->first_attribute("name", 4, false)->value();

		for (auto *pChild = pNode->first_node(); pChild != nullptr;
			 pChild = pChild->next_sibling())
		{
			if (_strnicmp(pChild->name(), "define", pChild->name_size()) == 0)
			{
				auto *pNameNode(pChild->first_attribute("name", 4, false));
				if (pNameNode == nullptr)
				{
					THROW_IOE_RENDERER_EXCEPTION(
						"Name in defintion element has not been assigned",
						E_ERROR_FAILED_TO_READ_MATERIAL);
				}
				auto *pValueNode(pChild->first_attribute("value", 5, false));
				if (pValueNode == nullptr)
				{
					THROW_IOE_RENDERER_EXCEPTION(
						"Value in defintion element has not been assigned",
						E_ERROR_FAILED_TO_READ_MATERIAL);
				}

				m_arrDefinitions.emplace_back(pNameNode->value(),
											  pValueNode->value());
			}
			else if (_strnicmp(pChild->name(), "shader",
							   pChild->name_size()) == 0)
			{
				auto *pPlatformNode(
					pChild->first_attribute("platform", 8, false));
				if (pPlatformNode == nullptr)
				{
					THROW_IOE_RENDERER_EXCEPTION(
						"Platform in shader element has not been assigned",
						E_ERROR_FAILED_TO_READ_MATERIAL);
				}
				// Test if we should compile this shader on this platform
				if (_strnicmp(pPlatformNode->value(),
							  STRINGIFY_EXPAND(IOE_RPI),
							  pPlatformNode->value_size()) == 0)
				{
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> >
						tUTF8ToWide;

					IOE::Renderer::IOEShaderCompilationOptions tOptions;
					tOptions.pVertexFormat = nullptr;

					tOptions.szPath = tUTF8ToWide.from_bytes(pChild->value());

					auto *pEntryNode(
						pChild->first_attribute("entry", 5, false));
					if (pEntryNode != nullptr)
					{
						tOptions.szEntryPoint = pEntryNode->value();
					}
					else
					{
						tOptions.szEntryPoint = "main";
					}

					auto *pVersionNode(
						pChild->first_attribute("version", 7, false));
					if (pVersionNode == nullptr)
					{
						THROW_IOE_RENDERER_EXCEPTION(
							"Version in shader element has not been assigned",
							E_ERROR_FAILED_TO_READ_MATERIAL);
					}
					tOptions.szProfile = pVersionNode->value();

					auto *pShaderTypeNode(
						pChild->first_attribute("type", 4, false));
					if (pShaderTypeNode == nullptr)
					{
						THROW_IOE_RENDERER_EXCEPTION(
							"Type in shader element has not been assigned",
							E_ERROR_FAILED_TO_READ_MATERIAL);
					}
					tOptions.eShaderType =
						EShaderType::FromName(pShaderTypeNode->value());
					if (tOptions.eShaderType == EShaderType::Unknown)
					{
						THROW_IOE_RENDERER_EXCEPTION_VARGS(
							"Type '%s' is an invalid shader type",
							E_ERROR_FAILED_TO_READ_MATERIAL,
							pShaderTypeNode->value());
					}
					tOptions.pMacros = &m_arrDefinitions;

					auto *pVertexFormat(
						pChild->first_attribute("vertexformat", 12, false));
					if (pVertexFormat != nullptr)
					{
						const char *szVertexFormatName(pVertexFormat->value());
						auto pFoundVertex(
							IOEVertexFormatContainer::GetSingletonPtr()
								->GetVertexFormatShared(szVertexFormatName));
						if (pFoundVertex == nullptr)
						{
							THROW_IOE_RENDERER_EXCEPTION_VARGS(
								"Vertex format '%s' is an invalid vertex "
								"format",
								E_ERROR_FAILED_TO_READ_MATERIAL,
								szVertexFormatName);
						}
						tOptions.pVertexFormat = pFoundVertex;
					}
					else
					{
						tOptions.pVertexFormat =
							IOEVertexFormatContainer::GetSingletonPtr()
								->GetVertexFormatShared("Default");
					}

					auto pShader(std::make_shared<IOEShader>());
					pShader->Compile(tOptions);
					m_arrShaders.push_back(pShader);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
											 EShaderVariableType eType,
											 float fU)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, fU);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
											 const char *szVariableName,
											 float fU)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, fU);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
											 EShaderVariableType eType,
											 std::int32_t nValue)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, nValue);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
											 const char *szVariableName,
											 std::int32_t nValue)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, nValue);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
											 EShaderVariableType eType,
											 std::uint32_t nValue)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, nValue);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
											 const char *szVariableName,
											 std::uint32_t nValue)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, nValue);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
											 EShaderVariableType eType,
											 float fU, float fV)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, fU, fV);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
											 const char *szVariableName,
											 float fU, float fV)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, fU, fV);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
											 EShaderVariableType eType,
											 float fX, float fY, float fZ)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, fX, fY, fZ);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
											 const char *szVariableName,
											 float fX, float fY, float fZ)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, fX, fY, fZ);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
											 EShaderVariableType eType,
											 float fX, float fY, float fZ,
											 float fW)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, fX, fY, fZ, fW);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
											 const char *szVariableName,
											 float fX, float fY, float fZ,
											 float fW)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, fX, fY, fZ,
								fW);
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
										EShaderVariableType eType,
										const IOE::Maths::IOEVector &rvVector)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, rvVector);
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
										const char *szVariableName,
										const IOE::Maths::IOEVector &rvVector)
	{
		SetCBufferVariableValue(szConstantBuffer, szVariableName, rvVector);
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEMaterial::SetShaderVariableValue(const char *szGlobalVariable,
										EShaderVariableType eType,
										const IOE::Maths::IOEMatrix &rmMatrix)
	{
		SetGlobalVariableValue(szGlobalVariable, eType, rmMatrix.Transpose());
	}

	//////////////////////////////////////////////////////////////////////////

	void
	IOEMaterial::SetShaderVariableValue(const char *szConstantBuffer,
										const char *szVariableName,
										const IOE::Maths::IOEMatrix &rmMatrix)
	{
		// WHY TRANSPOSED!!!!!!! 5 hours debugging this shit to find out HLSL
		// accepts matrix differently to directx math... Go figure...
		SetCBufferVariableValue(szConstantBuffer, szVariableName,
								rmMatrix.Transpose());
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::Set(IOE::Renderer::IOERPI *pRPI,
						  bool bSetShaderParams /*=true*/)
	{
		pRPI->ResetState();
		if (bSetShaderParams /*&& pRPI->GetMaterialIndex() != GetResourceIndex()*/)
		{
			const IOE::Maths::IOECamera &rCamera(pRPI->GetCamera());
			IOE::Maths::IOEMatrix mWorld(IOE::Maths::g_mIdentity);

			IOE::Maths::IOEMatrix wvp(mWorld * rCamera.GetViewMatrix() *
									  rCamera.GetProjectionMatrix());

			IOE::Maths::IOEMatrix mInvXPose(mWorld.Inverse());
			mInvXPose.TransposeSet();

			const IOEViewport &rViewport(pRPI->GetViewport(0));
			MapCBuffer("GlobalSettings");
			SetShaderVariableValue("GlobalSettings", "View",
								   rCamera.GetTransform().GetTranslation());
			SetShaderVariableValue("GlobalSettings", "WorldViewProj", wvp);
			SetShaderVariableValue("GlobalSettings", "WorldInvTranspose",
								   mInvXPose);
			SetShaderVariableValue("GlobalSettings", "ViewportSize",
								   rViewport.GetWidth(),
								   rViewport.GetHeight());
			UnmapCBuffer("GlobalSettings");

			pRPI->SetMaterialIndex(GetResourceIndex());
		}

		for (auto pShader : m_arrShaders)
		{
			pShader->Set(pRPI);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::MapCBuffer(const char *szConstantBufferName)
	{
		for (auto pShader : m_arrShaders)
		{
			ShaderConstantBufferDescription *pCBufferDescription(
				pShader->FindConstantBuffer(szConstantBufferName));
			if (pCBufferDescription != nullptr)
			{
				void *pBaseAddress(pShader->MapCBuffer(
					IOERPI::GetSingletonPtr(), 0, pCBufferDescription));
				pCBufferDescription->pMappedAddress = pBaseAddress;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::UnmapCBuffer(const char *szConstantBufferName)
	{
		for (auto pShader : m_arrShaders)
		{
			ShaderConstantBufferDescription *pCBufferDescription(
				pShader->FindConstantBuffer(szConstantBufferName));
			if (pCBufferDescription != nullptr)
			{
				pShader->UnmapCBuffer(IOERPI::GetSingletonPtr(), 0,
									  pCBufferDescription);
				pCBufferDescription->pMappedAddress = nullptr;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEMaterial::SetTextureVariable(IOERPI *pRPI, const char *szTextureName, IOETexture *pTexture, int arrOffset /*=0*/)
	{
		bool bAnySet(false);
		for (std::shared_ptr<IOEShader> pShader : m_arrShaders)
		{
			if (pShader->SetTextureVariable(pRPI, szTextureName, pTexture, arrOffset))
			{
				bAnySet = true;
			}
		}
		return bAnySet;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEMaterial::SetSamplerVariable(const char *szSamplerName,
										 IOESamplerState *pSampler)
	{
		bool bAnySet(false);
		for (auto pShader : m_arrShaders)
		{
			if (pShader->SetSamplerVariable(IOERPI::GetSingletonPtr(),
											szSamplerName, pSampler))
			{
				bAnySet = true;
			}
		}
		return bAnySet;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEMaterial::SetSamplers(
		const vector<pair<string, IOESamplerState *> > &arrSamplers)
	{
		bool bAnySet(false);
		for (auto pShader : m_arrShaders)
		{
			if (pShader->SetSamplers(IOERPI::GetSingletonPtr(), arrSamplers))
			{
				bAnySet = true;
			}
		}
		return bAnySet;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::AddDefinition(const std::string &szName,
									const std::string &szDefinition)
	{
		m_arrDefinitions.emplace_back(szName, szDefinition);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEMaterial::Dispatch(IOERPI *pRPI, int32_t nNumThreadGroupsX,
							   int32_t nNumThreadGroupsY,
							   int32_t nNumThreadGroupsZ)
	{
		IOE_ASSERT(m_arrShaders.size() == 1 &&
					   m_arrShaders.at(0)->GetShaderType() ==
						   EShaderType::Compute,
				   "Can only dispatch materials with a single compute shader");
		if (m_arrShaders.size() == 1 &&
			m_arrShaders.at(0)->GetShaderType() == EShaderType::Compute)
		{
			m_arrShaders.at(0)->Dispatch(pRPI, nNumThreadGroupsX,
										 nNumThreadGroupsY, nNumThreadGroupsZ);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	std::uint32_t
	IOEMaterial::GetStructuredBufferSize(const char *szName) const
	{
		for (auto &rShader : m_arrShaders)
		{
			std::uint32_t uSize(rShader->GetStructuredBufferSize(szName));
			if (uSize > 0)
			{
				return uSize;
			}
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE

//////////////////////////////////////////////////////////////////////////