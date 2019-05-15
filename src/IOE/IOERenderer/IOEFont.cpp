#include "IOEFont.h"

#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOERenderer/IOEMaterial.h>
#include <IOE/IOEMaths/Camera.h>

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOEFontStyle::IOEFontStyle(const IOEVector &vColour_, float fScaleX_,
							   float fScaleY_)
		: vColour(vColour_)
		, fScaleX(fScaleX_)
		, fScalyY(fScaleY_)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	IOEFont::IOEFont()
		: m_tIndexBuffer(IOEText::ms_uMaxNumChars * 6)
		, m_tPlatform(this)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEFont::Setup(IOERPI *pRPI)
	{
		m_tPlatform.Setup(pRPI);

		m_pTexture = std::make_shared<IOETexture>(
			ETextureFormat::B8G8R8A8_UNORM, m_fTextureWidth, m_fTextureHeight);
		m_pTexture->SetBufferAccess(ECPUAccess::GPUReadOnlyConst);
		m_pTexture->SetBufferDescription(EBufferDescription::ShaderResource);

		IOETextureSource tSource;
		tSource.pSrc = (void *)(m_arrTextureSource.data());
		tSource.uNumMipMaps		= 1;
		tSource.uPixelSizeBytes = 4;

		m_pTexture->Setup(&tSource);

		m_tIndexBuffer.SetBufferDescription(EBufferDescription::Index);
		m_tIndexBuffer.SetCPUAccess(ECPUAccess::GPUReadOnlyConst);
		for (std::uint32_t uIdx(0); uIdx < IOEText::ms_uMaxNumChars; ++uIdx)
		{
			m_tIndexBuffer.Set(uIdx * 6 + 0, uIdx * 4 + 0);
			m_tIndexBuffer.Set(uIdx * 6 + 1, uIdx * 4 + 2);
			m_tIndexBuffer.Set(uIdx * 6 + 2, uIdx * 4 + 1);

			m_tIndexBuffer.Set(uIdx * 6 + 3, uIdx * 4 + 1);
			m_tIndexBuffer.Set(uIdx * 6 + 4, uIdx * 4 + 2);
			m_tIndexBuffer.Set(uIdx * 6 + 5, uIdx * 4 + 3);
		}
		m_tIndexBuffer.Setup(pRPI);
	}

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IOEFont>
	IOEFont::CreateFontFromName(const std::string &szFontName,
								std::uint32_t uHeight,
								EFontCreationFlags::Enum eFlags /*= 0*/)
	{
		auto pFont = std::make_shared<IOEFont>();

		pFont->m_uHeight	   = uHeight;
		pFont->m_eCreationFlag = eFlags;
		pFont->m_tPlatform.SetFontName(szFontName);

		return pFont;
	}

	//////////////////////////////////////////////////////////////////////////

	IOEText::IOEText(
		const std::string &szData, float fX, float fY,
		std::shared_ptr<IOEFont> pFont,
		const IOEVector &vColourBegin /*=Maths::ColourList::White*/,
		const IOEVector &vColourEnd /*=Maths::ColourList::White*/)
		: m_pFont(pFont)
		, m_szData(szData)
		, m_fX(fX)
		, m_fY(fY)
		, m_bSetup(false)
		, m_vColourBegin(vColourBegin)
		, m_vColourEnd(vColourEnd)
		, m_tVertexBuffer(ms_uMaxNumChars * 4)
		, m_fScaleX(1.0f)
		, m_fScaleY(1.0f)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::Setup(IOERPI *pRPI)
	{
		UpdateData();

		m_tVertexBuffer.SetCPUAccess(ECPUAccess::GPUReadCPUWrite);
		m_tVertexBuffer.SetBufferDescription(EBufferDescription::Vertex);
		m_tVertexBuffer.Setup(pRPI);

		m_bSetup = true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::UpdateScale(float fScaleX, float fScaleY)
	{
		m_fScaleX = fScaleX;
		m_fScaleY = fScaleY;
		UpdateData();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::UpdateText(const std::string &szData)
	{
		if (m_szData != szData)
		{
			m_szData = szData;

			UpdateData();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::UpdatePosition(float fX, float fY)
	{
		m_fX = fX;
		m_fY = fY;
		UpdateData();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::UpdateColour(const IOEVector &vColour)
	{
		m_vColourBegin = m_vColourEnd = vColour;
		UpdateData();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::UpdateColour(const IOEVector &vColourBegin,
							   const IOEVector &vColourEnd)
	{
		m_vColourBegin = vColourBegin;
		m_vColourEnd   = vColourEnd;
		UpdateData();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::OnRender(IOE::Renderer::IOERPI *pPtr)
	{
		pPtr->DrawBuffer(&m_tVertexBuffer, EPrimitiveTopology::TriangleList,
						 m_pFont->GetIndexBuffer(),
						 (std::int32_t)(m_szData.size()) * 6);
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEText::Map(void *&rpData)
	{
		return m_tVertexBuffer.Map(EBufferMapType::WriteDiscard, rpData);
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEText::Unmap()
	{
		return m_tVertexBuffer.Unmap();
	}

	//////////////////////////////////////////////////////////////////////////

	float IOEText::GetRenderWidth()
	{
		float fStringSize(0.0f);
		for (size_t nIdx(0); nIdx < m_szData.size(); ++nIdx)
		{
			char chCharacter = m_szData.at(nIdx);
			const IOEFontGlyph *pGlyph(m_pFont->GetGlyphFromChar(chCharacter));
			if (pGlyph != nullptr)
			{
				fStringSize += pGlyph->fSizeX;
			}
		}
		return fStringSize * m_fScaleX;
	}

	//////////////////////////////////////////////////////////////////////////

	float IOEText::GetRenderHeight()
	{
		float fStringSize(0.0f);
		for (size_t nIdx(0); nIdx < m_szData.size(); ++nIdx)
		{
			char chCharacter = m_szData.at(nIdx);
			const IOEFontGlyph *pGlyph(m_pFont->GetGlyphFromChar(chCharacter));
			if (pGlyph != nullptr)
			{
				fStringSize += pGlyph->fSizeY;
			}
		}
		return fStringSize * m_fScaleY;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEText::UpdateData()
	{
		void *pSrcData = nullptr;
		if (m_bSetup)
		{
			bool bSuccessful(Map(pSrcData));
			IOE_ASSERT(bSuccessful, "Not Successfull!");
			if (!bSuccessful)
			{
				return;
			}
		}

		float fRenderWidth(GetRenderWidth());

		float fCurrentX(m_fX);

		for (size_t nIdx(0); nIdx < m_szData.size(); ++nIdx)
		{
			char chCharacter = m_szData.at(nIdx);
			const IOEFontGlyph *pGlyph(m_pFont->GetGlyphFromChar(chCharacter));
			if (pGlyph != nullptr)
			{
				IOEVector vColourStart(
					IOEVector::Lerp(m_vColourBegin, m_vColourEnd,
									(fCurrentX - m_fX) / fRenderWidth));
				IOEVector vColourEnd(IOEVector::Lerp(
					m_vColourBegin, m_vColourEnd,
					(fCurrentX - m_fX + pGlyph->fSizeX) / fRenderWidth));

				m_tVertexBuffer.SetEmplace(
					nIdx * 4, IOEVector(fCurrentX, m_fY, 0.0f, 1.0f),
					vColourStart, pGlyph->fTexMinX, pGlyph->fTexMaxY);

				m_tVertexBuffer.SetEmplace(
					nIdx * 4 + 1,
					IOEVector(fCurrentX + pGlyph->fSizeX * m_fScaleX, m_fY,
							  0.0f, 1.0f),
					vColourEnd, pGlyph->fTexMaxX, pGlyph->fTexMaxY);

				m_tVertexBuffer.SetEmplace(
					nIdx * 4 + 2,
					IOEVector(fCurrentX, m_fY + pGlyph->fSizeY * m_fScaleY,
							  0.0f, 1.0f),
					vColourStart, pGlyph->fTexMinX, pGlyph->fTexMinY);

				m_tVertexBuffer.SetEmplace(
					nIdx * 4 + 3,
					IOEVector(fCurrentX + pGlyph->fSizeX * m_fScaleX,
							  m_fY + pGlyph->fSizeY * m_fScaleY, 0.0f, 1.0f),
					vColourEnd, pGlyph->fTexMaxX, pGlyph->fTexMinY);

				fCurrentX += pGlyph->fSizeX * m_fScaleX;
			}
		}

		if (m_bSetup)
		{
			FontVertex *pTypedData(reinterpret_cast<FontVertex *>(pSrcData));
			memcpy(pTypedData, m_tVertexBuffer.GetRawData(),
				   m_tVertexBuffer.GetNumElements() *
					   m_tVertexBuffer.GetElementSize());

			bool bSuccessful(Unmap());
			IOE_ASSERT(bSuccessful, "Not Successfull!");
			if (!bSuccessful)
			{
				return;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	IOETextManager::IOETextManager()
		: IOEManager("IOETextManager")
		, tDepthDisable(IOEDepthState::Default())
		, tAlphaBlending(IOEBlendState::DefaultAlpha())
		, m_bVisible(true)
	{
		tDepthDisable.tData.bDepthEnable = false;
	}

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IOEText> IOETextManager::CreateText(
		const std::string &rszData, float fX, float fY,
		shared_ptr<IOEFont> pFont,
		const IOEVector &vColourBegin /*=IOE::Maths::ColourList::White*/,
		const IOEVector &vColourEnd /*=IOE::Maths::ColourList::White*/)
	{
		auto pText = std::make_shared<IOEText>(rszData, fX, fY, pFont,
											   vColourBegin, vColourEnd);
		pText->Setup(IOERPI::GetSingletonPtr());
		m_arrTextInstances.push_back(pText);
		return pText;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETextManager::OnManagerPostInit()
	{
		IOEManager::OnManagerPostInit();
		m_pFontShader = std::make_shared<Renderer::IOEMaterial>(
			Core::Algorithm::JoinPath(IOE::Core::g_pEngine->GetMediaPath(),
									  L"Materials", L"RenderText.ioe.xml"));

		tDepthDisable.Setup(IOERPI::GetSingletonPtr());
		tAlphaBlending.Setup(IOERPI::GetSingletonPtr());
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETextManager::OnRender(IOE::Renderer::IOERPI *pPtr)
	{
		IOEManager::OnRender(pPtr);

		if (m_bVisible)
		{
			pPtr->SetDepthState(tDepthDisable);
			pPtr->SetBlendState(tAlphaBlending);

			pPtr->SetDepthTarget(pPtr->GetDefaultDepthStencilbuffer(), false);
			pPtr->SetRenderTarget(pPtr->GetDefaultColourBuffer());

			pPtr->ClearDepth(IOE::Maths::ColourList::White);

			IOE::Maths::IOECamera::CCameraProperties tCameraProperties;
			tCameraProperties.eProjectionMethod =
				IOE::Maths::IOECamera::EProjectionMethod::Orthographic;
			IOE::Maths::IOECamera mCameraTemp;

			mCameraTemp.SetOrthographic();
			mCameraTemp.UpdateViewMatrices();

			IOE::Maths::IOECamera tOldCamera(pPtr->GetCamera());
			pPtr->SetCamera(mCameraTemp);

			m_pFontShader->Set(pPtr, false);

			m_pFontShader->MapCBuffer("Globals");
			m_pFontShader->SetShaderVariableValue(
				"Globals", "WorldViewProj", mCameraTemp.GetProjectionMatrix());
			m_pFontShader->UnmapCBuffer("Globals");

			for (auto &rText : m_arrTextInstances)
			{
				m_pFontShader->SetTextureVariable(pPtr,
					"GlyphSampler", rText->GetFont()->GetTexture());
				rText->OnRender(pPtr);
			}

			pPtr->SetCamera(tOldCamera);

			pPtr->SetDepthState(pPtr->GetDefaultDepthState());
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE