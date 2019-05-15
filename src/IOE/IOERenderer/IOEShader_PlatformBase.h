#pragma once

#include <string>
#include <utility>
#include <vector>
#include <array>
#include <memory>
#include <map>

#include <IOE/IOECore/IOEDefines.h>

using std::shared_ptr;
using std::vector;
using std::pair;
using std::string;

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOERPI;
	class IOEShader;
	class IOEVertexFormat;

	//////////////////////////////////////////////////////////////////////////

	namespace EShaderType
	{
		enum Enum
		{
			Unknown,
			Vertex,
			Geometry,
			Hull,
			Domain,
			Fragment,
			Compute,

			NumShaders
		};

		inline Enum FromName(const char *szName)
		{
			if (_stricmp(szName, "Vertex") == 0)
			{
				return Vertex;
			}
			else if (_stricmp(szName, "Geometry") == 0)
			{
				return Geometry;
			}
			else if (_stricmp(szName, "Hull") == 0)
			{
				return Hull;
			}
			else if (_stricmp(szName, "Domain") == 0)
			{
				return Domain;
			}
			else if (_stricmp(szName, "Fragment") == 0)
			{
				return Fragment;
			}
			else if (_stricmp(szName, "Compute") == 0)
			{
				return Compute;
			}
			return Unknown;
		}

		inline const char *ToName(Enum eType)
		{
			switch (eType)
			{
			case Vertex:
				return "Vertex";
			case Geometry:
				return "Geometry";
			case Hull:
				return "Hull";
			case Domain:
				return "Hull";
			case Fragment:
				return "Fragment";
			case Compute:
				return "Compute";
			case Unknown:
			default:
				return "Unknown";
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	struct IOEShaderMacroDefinition
	{
		IOEShaderMacroDefinition(std::string szName, std::string szDefinition)
			: m_szName(std::move(szName))
			, m_szDefinition(std::move(szDefinition))
		{
		}
		IOEShaderMacroDefinition(const IOEShaderMacroDefinition &) = default;
		IOEShaderMacroDefinition(IOEShaderMacroDefinition &&rhs)
			: m_szName(std::move(rhs.m_szName))
			, m_szDefinition(std::move(rhs.m_szDefinition))
		{
		}

		IOEShaderMacroDefinition &
		operator=(const IOEShaderMacroDefinition &rhs) = default;
		IOEShaderMacroDefinition &operator=(IOEShaderMacroDefinition &&rhs)
		{
			m_szName	   = std::move(rhs.m_szName);
			m_szDefinition = std::move(rhs.m_szDefinition);
		}

	public:
		std::string m_szName;
		std::string m_szDefinition;
	};

	//////////////////////////////////////////////////////////////////////////

	// Implicit cast to int needed
	namespace EShaderVariableCategories
	{
		enum Enum
		{
			Texture,
			Sampler,

			RawBuffer,
			StructuredBuffer,

			NumCategories
		};
	}

	//////////////////////////////////////////////////////////////////////////

	enum class EShaderVariableType
	{
		None,

		Float,
		Float2,
		Float3,
		Float4,
		Float4x4,
		Int,
		UInt,

		Texture,
		Texture1D,
		Texture2D,
		Texture3D,
		TextureCube,
		Texture1DArray,
		Texture2DArray,
		TextureCubeArray,

		SamplerState,

		RawBuffer,

		StructuredBufferBasic,
		StructuredBufferAppend,
		StructuredBufferConsume,
		StructuredBufferCounter,
	};

	static inline bool IsBasicShaderVariableType(EShaderVariableType eType)
	{
		switch (eType)
		{
		case EShaderVariableType::Float:
		case EShaderVariableType::Float2:
		case EShaderVariableType::Float3:
		case EShaderVariableType::Float4:
		case EShaderVariableType::Float4x4:
		case EShaderVariableType::Int:
		case EShaderVariableType::UInt:
			return true;
		default:
			return false;
		}
	}

	static inline bool IsTextureShaderVariableType(EShaderVariableType eType)
	{
		switch (eType)
		{
		case EShaderVariableType::Texture:
		case EShaderVariableType::Texture1D:
		case EShaderVariableType::Texture2D:
		case EShaderVariableType::Texture3D:
		case EShaderVariableType::TextureCube:
		case EShaderVariableType::Texture1DArray:
		case EShaderVariableType::Texture2DArray:
		case EShaderVariableType::TextureCubeArray:
			return true;
		default:
			return false;
		}
	}

	static inline bool IsSamplerShaderVariableType(EShaderVariableType eType)
	{
		switch (eType)
		{
		case EShaderVariableType::SamplerState:
			return true;
		default:
			return false;
		}
	}

	static inline bool IsRawBufferShaderVariableType(EShaderVariableType eType)
	{
		switch (eType)
		{
		case EShaderVariableType::RawBuffer:
			return true;
		default:
			return false;
		}
	}

	static inline bool
	IsStructuredBufferShaderVariableType(EShaderVariableType eType)
	{
		switch (eType)
		{
		case EShaderVariableType::StructuredBufferBasic:
		case EShaderVariableType::StructuredBufferAppend:
		case EShaderVariableType::StructuredBufferConsume:
		case EShaderVariableType::StructuredBufferCounter:
			return true;
		default:
			return false;
		}
	}

	static inline EShaderVariableCategories::Enum
	GetShaderCategory(EShaderVariableType eVariableType)
	{
		if (IsBasicShaderVariableType(eVariableType))
		{
			return EShaderVariableCategories::NumCategories;
		}
		else if (IsTextureShaderVariableType(eVariableType))
		{
			return EShaderVariableCategories::Texture;
		}
		else if (IsSamplerShaderVariableType(eVariableType))
		{
			return EShaderVariableCategories::Sampler;
		}
		else if (IsRawBufferShaderVariableType(eVariableType))
		{
			return EShaderVariableCategories::RawBuffer;
		}
		else if (IsStructuredBufferShaderVariableType(eVariableType))
		{
			return EShaderVariableCategories::StructuredBuffer;
		}
		return EShaderVariableCategories::NumCategories;
	}

	//////////////////////////////////////////////////////////////////////////

	struct ShaderVariableDescription
	{
		ShaderVariableDescription()
			: pNext(nullptr)
			, eType(EShaderVariableType::None)
			, nSlot(0)
			, nSizeOrSamples(0)
			, bGPUWriteable(false)
		{
			szName[0] = '\0';
		}

		ShaderVariableDescription(const ShaderVariableDescription &rhs) =
			default;
		ShaderVariableDescription(ShaderVariableDescription &&rhs) = default;
		ShaderVariableDescription &
		operator=(const ShaderVariableDescription &rhs) = default;
		ShaderVariableDescription &
		operator=(ShaderVariableDescription &&rhs) = default;

		char szName[64];
		EShaderVariableType eType;
		std::int32_t nSlot;
		std::int32_t nSizeOrSamples;
		ShaderVariableDescription *pNext;
		bool bGPUWriteable;
	};

	typedef std::array<std::vector<ShaderVariableDescription>,
					   EShaderVariableCategories::NumCategories>
		ShaderCategories;
	typedef std::vector<ShaderVariableDescription> ShaderDescription;

	//////////////////////////////////////////////////////////////////////////

	struct ShaderConstantBufferDescription
	{
		static const std::int32_t ms_nMaxCBufferSlots = 16;

		ShaderConstantBufferDescription()
			: nSizeBytes(0)
			, nSlotIdx(0)
			, pMappedAddress(nullptr)
		{
			szName[0] = '\0';
		}

		ShaderConstantBufferDescription(
			const ShaderConstantBufferDescription &rhs) = default;
		ShaderConstantBufferDescription(
			ShaderConstantBufferDescription &&rhs) = default;
		ShaderConstantBufferDescription &
		operator=(const ShaderConstantBufferDescription &rhs) = default;
		ShaderConstantBufferDescription &
		operator=(ShaderConstantBufferDescription &&rhs) = default;

		void *pMappedAddress;
		char szName[64];
		std::size_t nSizeBytes;
		std::size_t nSlotIdx;
		ShaderDescription arrVariables;
	};

	typedef std::array<ShaderConstantBufferDescription,
					   ShaderConstantBufferDescription::ms_nMaxCBufferSlots>
		ShaderConstantBuffers;

	//////////////////////////////////////////////////////////////////////////

	struct IOEShaderCompilationOptions
	{
		std::wstring szPath;
		std::string szProfile;
		std::string szEntryPoint;
		EShaderType::Enum eShaderType;
		std::vector<IOEShaderMacroDefinition> *pMacros;
		std::shared_ptr<IOEVertexFormat> pVertexFormat;
	};

	//////////////////////////////////////////////////////////////////////////

	interface_class IOEShader_PlatformBase
	{
	public:
		IOEShader_PlatformBase(IOEShader & rShader)
			: m_rShader(rShader)
		{
		}

		virtual void Compile(const IOEShaderCompilationOptions &rOptions) = 0;
		virtual void Cleanup()
		{
		}

		void AddMacro(IOEShaderMacroDefinition tMacro)
		{
			m_arrMacros.push_back(tMacro);
		}
		void AddMacro(std::string szName, std::string szDefinition)
		{
			m_arrMacros.emplace_back(szName, szDefinition);
		}

		virtual void FindShaderVariables(
			ShaderConstantBuffers & arrConstantBuffers,
			ShaderCategories & arrDescriptions) = 0;
		virtual void *MapConstantBuffers(
			IOERPI *pRPI, std::int32_t nSlotIdx,
			ShaderConstantBufferDescription * pBuffer) = 0;
		virtual void UnmapConstantBuffers(
			IOERPI *pRPI, std::int32_t nSlotIdx,
			ShaderConstantBufferDescription * pBuffer) = 0;

		virtual bool SetTextureVariable(
			IOERPI *pRPI, const char *szTextureName,
			class IOETexture *pTexture, int arrOffset = 0) = 0;
		virtual bool SetSamplerVariable(
			IOERPI *pRPI, const char *szSamplerName,
			struct IOESamplerState *pSamplers) = 0;
		virtual bool SetSamplers(
			IOERPI *pRPI,
			const vector<pair<string, struct IOESamplerState*> >
				&arrSamplers) = 0;

		virtual void Set(IOERPI *pRPI) = 0;

		virtual void Dispatch(IOERPI * pRPI, int32_t nNumThreadGroupsX,
							  int32_t nNumThreadGroupsY,
							  int32_t nNumThreadGroupsZ) = 0;

	protected:
		const std::vector<IOEShaderMacroDefinition> &GetMacros() const
		{
			return m_arrMacros;
		}

		std::vector<IOEShaderMacroDefinition> m_arrMacros;

		IOEShader &m_rShader;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE