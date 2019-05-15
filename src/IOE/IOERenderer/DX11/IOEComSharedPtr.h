#pragma once

#include <memory>
#include <functional>
#include <atlbase.h>

namespace IOE
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////

	template <class _T>
	class TComSharedPtr : public CComPtr<_T>
	{
	public:
		_T *get() { return p; }
		_T *get() const { return p; }
		void reset() { *this = nullptr; }
		using CComPtr<_T>::CComPtr;
		using CComPtr<_T>::operator=;
	};

	template<class _Ty1,
	class _Ty2>
		TComSharedPtr<_Ty1>
		TComPtrCast(const TComSharedPtr<_Ty2>& _Other) noexcept
	{	// return shared_ptr object holding static_cast<_Ty1 *>(_Other.get())
		_Ty1 *_Ptr = static_cast<_Ty1*>(_Other.get());
		return (TComSharedPtr<_Ty1>(_Ptr));
	}	

	//////////////////////////////////////////////////////////////////////////
} // namespace Renderer
} // namespace IOE