#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <cstdint>
#include <map>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOECore/IOEManagerContainer.h>

#include <IOE/IOEMaths/Vector.h>

#include <IOE/IOERenderer/IOETexture.h>

#include RPI_INCLUDE(IOEVertexFormat_Platform.h)

//////////////////////////////////////////////////////////////////////////

using std::vector;
using std::string;
using std::map;
using IOE::Maths::IOEVector;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	// The structure for all vertices within the application. Should this be
	// made
	// to be material specific or are we enforcing the same vertex structure to
	// be inherited by all vertices
	struct ModelVertex
	{
		ModelVertex()
			: m_vPosition(0.0f, 0.0f, 0.0f, 1.0f)
			, m_vNormal(0.0f, 0.0f, 0.0f, 0.0f)
			, m_vColour(1.0f, 1.0f, 1.0f, 1.0f)
			, m_vUV(0.0f, 0.0f)
			, m_vTangent(0.0f, 0.0f, 0.0f, 0.0f)
			, m_vBinormal(0.0f, 0.0f, 0.0f, 0.0f)
		{
		}
		ModelVertex(ModelVertex &&rhs) = default;
		ModelVertex(const ModelVertex &rhs) = default;

		ModelVertex &operator=(ModelVertex &&rhs) = default;
		ModelVertex &operator=(const ModelVertex &rhs) = default;

		XMFLOAT4 m_vPosition;
		XMFLOAT4 m_vNormal;
		XMFLOAT4 m_vColour;
		XMFLOAT2 m_vUV;
		XMFLOAT4 m_vTangent;
		XMFLOAT4 m_vBinormal;
	};

	//////////////////////////////////////////////////////////////////////////

	struct DebugVertex
	{
		DebugVertex()
			: vPos(Maths::g_vIdentity)
			, vColour(Maths::ColourList::Black)
		{
		}

		DebugVertex(const IOEVector &vPos_, const IOEVector &vColour_)
			: vPos(vPos_)
			, vColour(vColour_)
		{
		}

		void Set(const IOEVector &vPos_, const IOEVector &vColour_)
		{
			vPos	= vPos_;
			vColour = vColour_;
		}

		IOEVector vPos;
		IOEVector vColour;
	};

	//////////////////////////////////////////////////////////////////////////

	struct DebugVertexWidth
	{
		DebugVertexWidth()
			: vPos(Maths::g_vIdentity)
			, vColour(Maths::ColourList::Black)
			, fWidth(1.0f)
		{
		}

		DebugVertexWidth(const IOEVector &vPos_, const IOEVector &vColour_,
						 float fWidth_)
			: vPos(vPos_)
			, vColour(vColour_)
			, fWidth(fWidth_)
		{
		}

		void Set(const IOEVector &vPos_, const IOEVector &vColour_,
				 float fWidth_)
		{
			vPos	= vPos_;
			vColour = vColour_;
			fWidth  = fWidth_;
		}

		IOEVector vPos;
		IOEVector vColour;
		float fWidth;
	};

	//////////////////////////////////////////////////////////////////////////

	struct FontVertex
	{
		FontVertex()
			: vPos(Maths::g_vIdentity)
			, vColour(Maths::ColourList::White)
			, fUV(0.0f, 0.0f)
		{
		}

		FontVertex(const IOEVector &vPos_, const IOEVector &vColour_,
				   float fU_, float fV_)
			: vPos(vPos_)
			, vColour(vColour_)
			, fUV(fU_, fV_)
		{
		}

		FontVertex(const FontVertex &rhs)
			: vPos(rhs.vPos)
			, vColour(rhs.vColour)
			, fUV(rhs.fUV)
		{
		}

		FontVertex(FontVertex &&rhs)
			: vPos(std::move(rhs.vPos))
			, vColour(std::move(rhs.vColour))
			, fUV(std::move(rhs.fUV))
		{
		}

		FontVertex &operator=(const FontVertex &rhs)
		{
			vPos	= rhs.vPos;
			vColour = rhs.vColour;
			fUV		= rhs.fUV;

			return *this;
		}

		FontVertex &operator=(FontVertex &&rhs)
		{
			vPos	= std::move(rhs.vPos);
			vColour = std::move(rhs.vColour);
			fUV		= std::move(rhs.fUV);

			return *this;
		}

		void Set(const IOEVector &vPos_, const IOEVector &vColour_, float fU_,
				 float fV_)
		{
			vPos	= vPos_;
			vColour = vColour_;
			fUV.x   = fU_;
			fUV.y   = fV_;
		}

		XMFLOAT4 vPos;
		XMFLOAT4 vColour;
		XMFLOAT2 fUV;
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EVertexClassification
	{
		PerVertex,
		PerInstance,
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOESemanticFormat
	{
		IOESemanticFormat(std::string szSemanticName,
						  std::uint32_t uSemanticIndex,
						  ETextureFormat eTextureFormat,
						  std::uint32_t uSlotIndex, std::size_t uOffset,
						  EVertexClassification eClassification,
						  std::uint32_t uInstanceDataStepRate);

		IOESemanticFormat(const IOESemanticFormat &rhs) = default;
		IOESemanticFormat(IOESemanticFormat &&rhs) = default;

		IOESemanticFormat &operator=(const IOESemanticFormat &rhs) = default;
		IOESemanticFormat &operator=(IOESemanticFormat &&rhs) = default;

		std::string m_szSemanticName;
		std::uint32_t m_uSemanticIndex;
		ETextureFormat m_eTextureFormat;
		std::uint32_t m_uSlotIndex;
		std::size_t m_uOffset;
		EVertexClassification m_eClassification;
		std::uint32_t m_uInstanceDataStepRate;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEVertexFormat
	{
		friend class IOEVertexFormat_Platform;

	public:
		IOEVertexFormat()
			: m_tPlatform(*this)
		{
		}

		void AddSemanticFormat(std::string szSemanticName,
							   std::uint32_t uSemanticIndex,
							   ETextureFormat eTextureFormat,
							   std::uint32_t uSlotIndex, std::size_t uOffset,
							   EVertexClassification eClassification,
							   std::uint32_t uInstanceDataStepRate)
		{
			m_arrSemantics.emplace_back(
				szSemanticName, uSemanticIndex, eTextureFormat, uSlotIndex,
				uOffset, eClassification, uInstanceDataStepRate);
		}

		std::uint32_t GetNumSemantics() const
		{
			return static_cast<std::uint32_t>(m_arrSemantics.size());
		}

	public:
		IOEVertexFormat_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOEVertexFormat_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

	private:
		std::vector<IOESemanticFormat> m_arrSemantics;

		IOEVertexFormat_Platform m_tPlatform;
	};

	//////////////////////////////////////////////////////////////////////////

	// Contains all vertex formats
	class IOEVertexFormatContainer
		: public IOE::Core::IOEManager<IOEVertexFormatContainer>
	{
	public:
		IOEVertexFormatContainer()
			: IOEManager("VertexFormatContainer")
		{
		}

		virtual void OnManagerPreInit() override;

		FORCEINLINE_DEBUGGABLE shared_ptr<IOEVertexFormat>
		GetVertexFormatShared(std::string szName)
		{
			auto pIterator(m_arrVertexFormats.find(szName));
			if (pIterator == m_arrVertexFormats.end())
			{
				return nullptr;
			}
			return pIterator->second;
		}

		FORCEINLINE_DEBUGGABLE IOEVertexFormat *
		GetVertexFormat(std::string szName)
		{
			auto pIterator(m_arrVertexFormats.find(szName));
			if (pIterator == m_arrVertexFormats.end())
			{
				return nullptr;
			}
			return pIterator->second.get();
		}

		FORCEINLINE shared_ptr<IOEVertexFormat>
		CreateVertexFormat(std::string szName)
		{
			return m_arrVertexFormats.emplace(
										 szName,
										 std::make_shared<IOEVertexFormat>())
				.first->second;
		}

	private:
		std::map<std::string, shared_ptr<IOEVertexFormat> > m_arrVertexFormats;
	};

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE