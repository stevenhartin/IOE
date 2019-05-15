#pragma once

#include <IOE/IOECore/IOEDefines.h>

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEModel;

	//////////////////////////////////////////////////////////////////////////

	abstract_class IOEModel_PlatformBase
	{
	public:
		IOEModel_PlatformBase(IOEModel & rModel)
			: m_rModel(rModel)
		{
		}

		virtual void OnRender(class IOERPI * pRPI,
							  bool bSetShaderVariables = true) = 0;
		virtual void OnRender(class IOERPI * pRPI,
							  struct IOEVisibilityLink * pHead,
							  bool bSetShaderVariables = true) = 0;
		virtual void OnSetup(class IOERPI * pRPI) = 0;

	protected:
		IOEModel &GetModel()
		{
			return m_rModel;
		}
		const IOEModel &GetModel() const
		{
			return m_rModel;
		}

	private:
		IOEModel &m_rModel;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE