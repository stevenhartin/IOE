#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <map>

#include <IOE/IOECore/IOEResourceCounter.h>

#include "IOEShader_PlatformBase.h"
#include "IOERPI.h"

using std::make_shared;
using std::shared_ptr;
using std::vector;
using std::pair;
using std::string;

namespace rapidxml
{
template <typename ch>
class xml_node;
}

namespace IOE
{
namespace Maths
{
	class IOEVector;
	class IOEMatrix;
} // namespace Maths
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEShader;
	class IOETexture;

	//////////////////////////////////////////////////////////////////////////

	class IOEMaterial : public IOE::Core::IOEResourceCounter<IOEMaterial>
	{
	public:
		IOEMaterial();
		IOEMaterial(const std::wstring &szPath);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType, float fU);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName, float fU);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType,
									std::int32_t nValue);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName,
									std::int32_t nValue);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType,
									std::uint32_t nValue);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName,
									std::uint32_t nValue);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType, float fU,
									float fV);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName, float fU,
									float fV);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType, float fX,
									float fY, float fZ);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName, float fX,
									float fY, float fZ);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType, float fX,
									float fY, float fZ, float fW);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName, float fX,
									float fY, float fZ, float fW);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType,
									const IOE::Maths::IOEVector &rvVector);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName,
									const IOE::Maths::IOEVector &rvVector);

		void SetShaderVariableValue(const char *szGlobalVariable,
									EShaderVariableType eType,
									const IOE::Maths::IOEMatrix &rmMatrix);
		void SetShaderVariableValue(const char *szConstantBuffer,
									const char *szVariableName,
									const IOE::Maths::IOEMatrix &rmMatrix);

		bool SetTextureVariable(IOERPI *pRPI, const char *szTextureName,
								IOETexture *pTexture, int arrOffset = 0);
		bool SetSamplerVariable(const char *szTextureName,
								IOESamplerState *pSampler);
		bool SetSamplers(const vector<pair<string, struct IOESamplerState *> >
							 &arrSamplers);

		std::uint32_t GetStructuredBufferSize(const char *szName) const;

		void Set(IOE::Renderer::IOERPI *pRPI, bool bSetShaderParams = true);

		void LoadMaterial(const std::wstring &szPath);
		void LoadMaterial(rapidxml::xml_node<char> *pNode);

		void MapCBuffer(const char *szConstantBufferName);
		void UnmapCBuffer(const char *szConstantBufferName);

		void Dispatch(IOERPI *pRPI, int32_t nNumThreadGroupsX,
					  int32_t nNumThreadGroupsY, int32_t nNumThreadGroupsZ);

		void AddDefinition(const std::string &szName,
						   const std::string &szDefinition);

	private:
		template <class... TArgs>
		FORCEINLINE void
		SetCBufferVariableValue(const char *szConstantBufferName,
								const char *szVariableName, TArgs &&... tArgs)
		{
			for (auto pShader : m_arrShaders)
			{
				ShaderConstantBufferDescription *pCBufferDescription(
					pShader->FindConstantBuffer(szConstantBufferName));
				if (pCBufferDescription != nullptr)
				{
					ShaderVariableDescription *pVariableDescription(
						pShader->FindConstantBufferVariable(
							pCBufferDescription, szVariableName));
					if (pVariableDescription != nullptr)
					{
						// Map the buffer
						IOE_ASSERT(pCBufferDescription->pMappedAddress !=
									   nullptr,
								   "CBuffer must be mapped");
						pShader->SetShaderVariableValue(
							static_cast<std::uint8_t *>(
								pCBufferDescription->pMappedAddress) +
								pVariableDescription->nSlot,
							tArgs...);
					}
				}
			}
		}

		template <class... TArgs>
		FORCEINLINE void SetGlobalVariableValue(const char *szVariableName,
												EShaderVariableType eType,
												TArgs &&... tArgs)
		{
			for (auto pShader : m_arrShaders)
			{
				ShaderVariableDescription *pVariableDescription(
					pShader->FindGlobalVariable(szVariableName, eType));
				if (pVariableDescription != nullptr)
				{
					// Map the buffer
					// TODO: Implement custom buffers for global variables here
					// like samplers etc
					// pShader->SetShaderVariableValue(std::forward<void*,
					// TArgs>(nullptr, tArgs...)...);
					pShader->SetShaderVariableValue(nullptr, tArgs...);
				}
			}
		}

	private:
		std::string m_szMaterialName;
		std::vector<std::shared_ptr<IOEShader> > m_arrShaders;
		std::vector<IOEShaderMacroDefinition> m_arrDefinitions;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEMaterialVariableBase
	{
		IOEMaterialVariableBase(const std::string &szBuffer_,
								const std::string &szVariable_)
			: szBuffer(szBuffer_)
			, szVariable(szVariable_)
		{
		}

		virtual void SetShaderValue(IOEMaterial *pMaterial) = 0;

		std::string szBuffer;
		std::string szVariable;
	};

	struct IOEMaterialVariableInstance_Float : public IOEMaterialVariableBase
	{
		IOEMaterialVariableInstance_Float(const std::string szBuffer,
										  const std::string &szVariable,
										  float fValue_)
			: IOEMaterialVariableBase(szBuffer, szVariable)
			, fValue(fValue_)
		{
		}

		void UpdateValue(float fValue_)
		{
			fValue = fValue_;
		}

		virtual void SetShaderValue(IOEMaterial *pMaterial) override final
		{
			pMaterial->SetShaderVariableValue(szBuffer.c_str(),
											  szVariable.c_str(), fValue);
		}
		float fValue;
	};

	struct IOEMaterialVariableInstance_UV : public IOEMaterialVariableBase
	{
		IOEMaterialVariableInstance_UV(const std::string szBuffer,
									   const std::string &szVariable,
									   float fU_, float fV_)
			: IOEMaterialVariableBase(szBuffer, szVariable)
			, fU(fU_)
			, fV(fV_)
		{
		}

		void UpdateValue(float fU_, float fV_)
		{
			fU = fU_;
			fV = fV_;
		}

		virtual void SetShaderValue(IOEMaterial *pMaterial) override final
		{
			pMaterial->SetShaderVariableValue(szBuffer.c_str(),
											  szVariable.c_str(), fU, fV);
		}
		float fU;
		float fV;
	};

	struct IOEMaterialVariableInstance_XYZ : public IOEMaterialVariableBase
	{
		IOEMaterialVariableInstance_XYZ(const std::string szBuffer,
										const std::string &szVariable,
										float fX_, float fY_, float fZ_)
			: IOEMaterialVariableBase(szBuffer, szVariable)
			, fX(fX_)
			, fY(fY_)
			, fZ(fZ_)
		{
		}

		void UpdateValue(float fX_, float fY_, float fZ_)
		{
			fX = fX_;
			fY = fY_;
			fZ = fZ_;
		}

		virtual void SetShaderValue(IOEMaterial *pMaterial) override final
		{
			pMaterial->SetShaderVariableValue(szBuffer.c_str(),
											  szVariable.c_str(), fX, fY, fZ);
		}
		float fX;
		float fY;
		float fZ;
	};

	struct IOEMaterialVariableInstance_Vector : public IOEMaterialVariableBase
	{
		IOEMaterialVariableInstance_Vector(const std::string szBuffer,
										   const std::string &szVariable,
										   float fX_, float fY_, float fZ_,
										   float fW_)
			: IOEMaterialVariableBase(szBuffer, szVariable)
			, vVector(fX_, fY_, fZ_, fW_)
		{
		}

		IOEMaterialVariableInstance_Vector(
			const std::string szBuffer, const std::string &szVariable,
			const IOE::Maths::IOEVector &vVector_)
			: IOEMaterialVariableBase(szBuffer, szVariable)
			, vVector(vVector_)
		{
		}

		void UpdateValue(float fX_, float fY_, float fZ_, float fW_)
		{
			vVector.Set(fX_, fY_, fZ_, fW_);
		}

		void UpdateValue(const IOE::Maths::IOEVector &vVector_)
		{
			vVector = vVector_;
		}

		virtual void SetShaderValue(IOEMaterial *pMaterial) override final
		{
			pMaterial->SetShaderVariableValue(szBuffer.c_str(),
											  szVariable.c_str(), vVector);
		}

		IOE::Maths::IOEVector vVector;
	};
	struct IOEMaterialVariableInstance_Matrix : public IOEMaterialVariableBase
	{
		IOEMaterialVariableInstance_Matrix(
			const std::string szBuffer, const std::string &szVariable,
			const IOE::Maths::IOEMatrix &mMatrix_)
			: IOEMaterialVariableBase(szBuffer, szVariable)
			, mMatrix(mMatrix_)
		{
		}

		void UpdateValue(const IOE::Maths::IOEMatrix &mMatrix_)
		{
			mMatrix = mMatrix_;
		}

		virtual void SetShaderValue(IOEMaterial *pMaterial) override final
		{
			pMaterial->SetShaderVariableValue(szBuffer.c_str(),
											  szVariable.c_str(), mMatrix);
		}

		IOE::Maths::IOEMatrix mMatrix;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEMaterialInstance
	{
	public:
		IOEMaterialInstance(shared_ptr<IOEMaterial> pMaterial)
			: m_pMaterial(pMaterial)
		{
		}

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEMaterialVariableBase>
		AddVariableInstance(const std::string &szBuffer,
							const std::string &szData, float fU)
		{
			m_arrVariableInstances.push_back(
				make_shared<IOEMaterialVariableInstance_Float>(szBuffer,
															   szData, fU));
			return m_arrVariableInstances.back();
		}

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEMaterialVariableBase>
		AddVariableInstance(const std::string &szBuffer,
							const std::string &szData, float fU, float fV)
		{
			m_arrVariableInstances.push_back(
				make_shared<IOEMaterialVariableInstance_UV>(szBuffer, szData,
															fU, fV));
			return m_arrVariableInstances.back();
		}

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEMaterialVariableBase>
		AddVariableInstance(const std::string &szBuffer,
							const std::string &szData, float fX, float fY,
							float fZ)
		{
			m_arrVariableInstances.push_back(
				make_shared<IOEMaterialVariableInstance_XYZ>(szBuffer, szData,
															 fX, fY, fZ));
			return m_arrVariableInstances.back();
		}

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEMaterialVariableBase>
		AddVariableInstance(const std::string &szBuffer,
							const std::string &szData, float fX, float fY,
							float fZ, float fW)
		{
			m_arrVariableInstances.push_back(
				make_shared<IOEMaterialVariableInstance_Vector>(
					szBuffer, szData, fX, fY, fZ, fW));
			return m_arrVariableInstances.back();
		}

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEMaterialVariableBase>
		AddVariableInstance(const std::string &szBuffer,
							const std::string &szData,
							const IOE::Maths::IOEVector &vVector)
		{
			m_arrVariableInstances.push_back(
				make_shared<IOEMaterialVariableInstance_Vector>(
					szBuffer, szData, vVector));
			return m_arrVariableInstances.back();
		}

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEMaterialVariableBase>
		AddVariableInstance(const std::string &szBuffer,
							const std::string &szData,
							const IOE::Maths::IOEMatrix &mMatrix)
		{
			m_arrVariableInstances.push_back(
				make_shared<IOEMaterialVariableInstance_Matrix>(
					szBuffer, szData, mMatrix));
			return m_arrVariableInstances.back();
		}

		FORCEINLINE_DEBUGGABLE void
		AddVariableInstance(const std::string &szTexture,
							shared_ptr<IOETexture> pTexture)
		{
			m_arrTextures.push_back(std::make_pair(szTexture, pTexture));
		}

		FORCEINLINE_DEBUGGABLE void
		AddVariableInstance(const std::string &szSampler,
							shared_ptr<IOESamplerState> pSampler)
		{
			m_arrSamplers.push_back(std::make_pair(szSampler, pSampler));
		}

		FORCEINLINE_DEBUGGABLE void SetAllVariables(IOERPI *pRPI)
		{
			m_pMaterial->SetTextureVariable(pRPI, "DiffuseTexture", nullptr);
			m_pMaterial->SetTextureVariable(pRPI, "BumpTexture", nullptr);

			for (auto pMaterialVariable : m_arrVariableInstances)
			{
				pMaterialVariable->SetShaderValue(m_pMaterial.get());
			}
			for (auto &rPair : m_arrTextures)
			{
				m_pMaterial->SetTextureVariable(pRPI, rPair.first.c_str(),
												rPair.second.get());
			}
			for (auto &rPair : m_arrSamplers)
			{
				m_pMaterial->SetSamplerVariable(rPair.first.c_str(), rPair.second.get());
			}
		}

		FORCEINLINE_DEBUGGABLE bool
		HasTextureInstance(const std::string &szTextureName)
		{
			for (auto &rPair : m_arrTextures)
			{
				if (rPair.first == szTextureName)
				{
					return true;
				}
			}
			return false;
		}

		const IOEMaterial *GetMaterial() const
		{
			return m_pMaterial.get();
		}

	private:
		vector<shared_ptr<IOEMaterialVariableBase> > m_arrVariableInstances;
		vector<pair<string, shared_ptr<IOETexture> > > m_arrTextures;
		vector<pair<string, shared_ptr<IOESamplerState> > > m_arrSamplers;
		shared_ptr<IOEMaterial> m_pMaterial;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE