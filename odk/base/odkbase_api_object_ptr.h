// Copyright DEWETRON GmbH 2017
#pragma once

#include "odkbase_if_api_object.h"
#include "odkuni_defines.h"
#include <boost/intrusive_ptr.hpp>

namespace odk
{

namespace detail
{
    template<class T>
    class ApiObjectPtr : public boost::intrusive_ptr<T>
    {
    public:
        ApiObjectPtr() noexcept = default;

        explicit ApiObjectPtr(T* p)
            : boost::intrusive_ptr<T>::intrusive_ptr(p, false)
        {
        }

        ApiObjectPtr(T* p, bool addRef)
            : boost::intrusive_ptr<T>::intrusive_ptr(p, addRef)
        {
        }
    };
} // namespace detail

    /**
     * Returns a smart pointer to any object derived from IfApiObject.
     * The pointer adopts object p and handles reference counting automatically.
     * It therefore integrates well with common plugin get*-functions which return objects with pre-increased ref count.
     */
    template <class T>
    ODK_NODISCARD detail::ApiObjectPtr<T> ptr(T* p)
    {
        return detail::ApiObjectPtr<T>(p);
    }

    template <class T>
    ODK_NODISCARD detail::ApiObjectPtr<T> ptr_add_ref(T* p)
    {
        return detail::ApiObjectPtr<T>(p, true);
    }

    template <class T, class O>
    ODK_NODISCARD detail::ApiObjectPtr<T> api_ptr_cast(const detail::ApiObjectPtr<O>& ptr)
    {
        detail::ApiObjectPtr<T> ret;
        ret.reset(static_cast<T*>(ptr.get()));
        return ret;
    }

    template <class T, class O>
    ODK_NODISCARD detail::ApiObjectPtr<T> value_ptr_cast(const detail::ApiObjectPtr<O>& ptr)
    {
        detail::ApiObjectPtr<T> ret;
        if (ptr && ptr->getType() == T::type_index)
        {
            ret.reset(static_cast<T*>(ptr.get()));
        }
        return ret;
    }

    template <class T, class O>
    ODK_NODISCARD detail::ApiObjectPtr<const T> value_ptr_cast(const detail::ApiObjectPtr<const O>& ptr)
    {
        detail::ApiObjectPtr<const T> ret;
        if (ptr && ptr->getType() == T::type_index)
        {
            ret.reset(static_cast<const T*>(ptr.get()));
        }
        return ret;
    }
}
