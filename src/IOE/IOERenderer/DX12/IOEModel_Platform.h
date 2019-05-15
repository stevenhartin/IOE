#pragma once

#include "../IOEModel_PlatformBase.h"

struct ID3D11Buffer;

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEModel;

	//////////////////////////////////////////////////////////////////////////

	class IOEModel_Platform : public IOEModel_PlatformBase
	{
	public:
		IOEModel_Platform(IOEModel &rModel)
			: IOEModel_PlatformBase(rModel)
		{
		}

		virtual void OnRender(class IOERPI *pRPI,
							  bool bSetShaderVariables = true) override final;
		virtual void OnRender(class IOERPI *pRPI,
							  struct IOEVisibilityLink *pHead,
							  bool bSetShaderVariables = true) override final;
		virtual void OnSetup(class IOERPI *pRPI) override final;

	private:
		TComSharedPtr<ID3D11Buffer> m_pVertexBuffer;
		TComSharedPtr<ID3D11Buffer> m_pIndexBuffer;
	};

	//////////////////////////////////////////////////////////////////////////
} // namespace Renderer
} // namespace IOE