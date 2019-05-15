#pragma once

#include <memory>

namespace IOE
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////

	template <class _T>
	class TComSharedPtr : public std::shared_ptr<_T>
	{
		struct TCustomReleaser
		{
			void operator()(_T *p)
			{
				if (p != nullptr)
				{
					p->Release();
				}
			}
		};

		std::shared_ptr<_T> m_pPtr;
		TCustomReleaser m_tReleasor;

	public:
		// Inherit ctors
		using std::shared_ptr<_T>::shared_ptr;

		TComSharedPtr()
			: std::shared_ptr<_T>(nullptr, m_tReleasor)
		{
		}

		TComSharedPtr &operator=(_T *pOther)
		{
			std::shared_ptr<_T>::operator=(
				std::shared_ptr<_T>(pOther, m_tReleasor));
			return *this;
		}

		_T **operator&()
		{
			return reinterpret_cast<_T**>(this);
		}

		_T * operator->()
		{
			return m_pPtr.get();
		}

		const _T * operator->() const
		{
			return m_pPtr.get();
		}

		operator _T*()
		{
			return m_pPtr.get();
		}

		operator const _T*() const
		{
			return m_pPtr.get();
		}
	};

	template<class _Ty1,
	class _Ty2>
		TComSharedPtr<_Ty1>
		TComPtrCast(const TComSharedPtr<_Ty2>& _Other) noexcept
	{	// return shared_ptr object holding static_cast<_Ty1 *>(_Other.get())
		typedef typename TComSharedPtr<_Ty1>::element_type _Elem1;
		_Elem1 *_Ptr = static_cast<_Elem1 *>(_Other.get());
		return (TComSharedPtr<_Ty1>(std::shared_ptr<_Ty2>(_Other), _Ptr));
	}

	template <class _Ty, class... _Types>
	inline TComSharedPtr<_Ty> MakeSharedCom(_Types &&... _Args)
	{ // make a shared_ptr
		std::_Ref_count_obj<_Ty> *_Rx =
			new std::_Ref_count_obj<_Ty>(_STD std::forward<_Types>(_Args)...);

		TComSharedPtr<_Ty> _Ret;
		_Ret._Resetp0(_Rx->_Getptr(), _Rx);
		return (_Ret);
	}

	//////////////////////////////////////////////////////////////////////////
} // namespace Renderer
} // namespace IOE