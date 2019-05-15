#pragma once

//////////////////////////////////////////////////////////////////////////

#include <IOE/IOECore/IOEDefines.h>
#include RPI_INCLUDE(IOEViewport_Platform.h)

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	struct IOEViewportDefinition
	{
	public:
		IOEViewportDefinition()
			: fTop(0.0f)
			, fLeft(0.0f)
			, fWidth(0.0f)
			, fHeight(0.0f)
			, fMinDepth(0.0f)
			, fMaxDepth(1.0f)
		{
		}
		IOEViewportDefinition(float fTop_, float fLeft_, float fWidth_,
							  float fHeight_, float fMinDepth_ = 0.0f,
							  float fMaxDepth_ = 1.0f)
			: fTop(fTop_)
			, fLeft(fLeft_)
			, fWidth(fWidth_)
			, fHeight(fHeight_)
			, fMinDepth(fMinDepth_)
			, fMaxDepth(fMaxDepth_)
		{
		}

	public:
		IOEViewportDefinition(const IOEViewportDefinition &rhs) = default;
		IOEViewportDefinition(IOEViewportDefinition &&rhs) = default;

		IOEViewportDefinition &
		operator=(const IOEViewportDefinition &rhs) = default;
		IOEViewportDefinition &
		operator=(IOEViewportDefinition &&rhs) = default;

	public:
		float fTop;
		float fLeft;
		float fWidth;
		float fHeight;
		float fMinDepth;
		float fMaxDepth;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEViewport
	{
	public:
		IOEViewport(float fWidth, float fHeight)
		{
			Resize(fWidth, fHeight);
		}
		IOEViewport(float fTop, float fLeft, float fWidth, float fHeight,
					float fMinDepth = 0.0f, float fMaxDepth = 1.0f)
			: m_tDefinition(fTop, fLeft, fWidth, fHeight, fMinDepth, fMaxDepth)
		{
		}

	public:
		FORCEINLINE IOEViewport_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		FORCEINLINE const IOEViewport_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

	public:
		FORCEINLINE float GetWidth() const
		{
			return m_tDefinition.fWidth;
		}
		FORCEINLINE float GetHeight() const
		{
			return m_tDefinition.fHeight;
		}
		FORCEINLINE float GetTop() const
		{
			return m_tDefinition.fTop;
		}
		FORCEINLINE float GetLeft() const
		{
			return m_tDefinition.fLeft;
		}
		FORCEINLINE float GetAspect() const
		{
			return m_fAspect;
		}

	public:
		FORCEINLINE_DEBUGGABLE void Resize(float fWidth, float fHeight)
		{
			m_tDefinition.fHeight = fHeight;
			m_tDefinition.fWidth  = fWidth;
			m_fAspect			  = std::numeric_limits<float>::infinity();
			if (fWidth > 0.0f && fHeight > 0.0f)
			{
				m_fAspect = fWidth / fHeight;
			}
		}

	private:
		IOEViewportDefinition m_tDefinition;
		IOEViewport_Platform m_tPlatform;
		float m_fAspect;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE