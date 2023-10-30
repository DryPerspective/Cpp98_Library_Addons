#ifndef DP_CPP98_POLY_VALUE_PTR
#define DP_CPP98_POLY_VALUE_PTR

#include <algorithm>
#include "bits/smart_ptr_bases.h"
#include "cpp98/static_assert.h"
#include "bits/version_defs.h"
#include "cpp98/type_traits.h"


/*
*	A polymorphic value pointer. A smart pointer which confers value semantics for its held object, but which is aware of polymorphism and will correctly copy
*	the dynamic type of the held object rather than the static type.
*	This is a separate class as the type erasure required would add unnecessary overhead for the most common uses (val_ptr)* 
*/

namespace dp {

	namespace detail {
		template<typename T, typename U>
		struct valid_poly_ptr_type {
#ifndef DP_BORLAND
			static const bool value = dp::is_base_of<T, U>::value;
#else
			//Borland's compiler is unable to detect base_of or convertibility. The template engine simply does not have the tools available.
			//As such I can't protect Borland uses from the UB of constructing a poly_value_ptr from an unrelated type.
			static const bool value = true;
#endif
		};
	}

	//A tag type to tell the pointer what type of object it points to
	template<typename T>
	struct poly_t {};


	template<typename T>
	class poly_value_ptr {


		//For the type erasure we mimic a fairly typical std::any implementation by storing a pointer to a templated manager function
		//which both encodes the type and allows us to perform real-type-aware operations
		struct op {
			enum type {
				clone,
				destroy
			};
		};

		template<typename U>
		struct manager {
			static T* manage(typename op::type inOp, const dp::poly_value_ptr<T>* inPtr) {
				const U* dynamic_ptr = static_cast<const U*>(inPtr->get());
				switch (inOp) {
				case op::clone:
					if (dynamic_ptr) {
						U* newObj = new U(*dynamic_ptr);
						return static_cast<T*>(newObj);
					}
					return NULL;
				case op::destroy:
					dp::default_delete<U>()(const_cast<U*>(dynamic_ptr));
					return NULL;
				}
			}
		};

		typedef T*(*manager_type)(typename op::type, const dp::poly_value_ptr<T>*);
		manager_type m_manager;

		T* m_data;


	public:
		poly_value_ptr() : m_manager(NULL), m_data(NULL) {}

		//We pass a poly_t first to account for the dynamic type. Otherwise we're in slicing hell.
		template<typename Held_Type, typename Ptr_Type>
		poly_value_ptr(dp::poly_t<Held_Type>, Ptr_Type* inPtr, typename dp::enable_if<dp::detail::valid_poly_ptr_type<T, Held_Type>::value && dp::detail::valid_poly_ptr_type<T, Ptr_Type>::value, bool>::type = true) 
					: m_manager(&manager<Held_Type>::manage), m_data(static_cast<T*>(inPtr)) {}

		poly_value_ptr(const poly_value_ptr& inPtr) : m_manager(inPtr.m_manager), m_data(m_manager(op::clone, &inPtr)) {}
		poly_value_ptr& operator=(const poly_value_ptr& inPtr) {
			poly_value_ptr copy(inPtr);
			this->swap(copy);
			return *this;
		}

		template<typename U>
		poly_value_ptr(const poly_value_ptr<U>& inPtr, typename dp::enable_if<dp::detail::valid_poly_ptr_type<T, U>::value, bool>::type = true) : m_manager(&manager<U>::manage), m_data(m_manager(op::clone, &inPtr)) {}
		template<typename U>
		typename dp::enable_if<dp::detail::valid_poly_ptr_type<T, U>::value, poly_value_ptr&>::type operator=(const poly_value_ptr<U>& inPtr) {
			poly_value_ptr<T> copy(inPtr);
			this->swap(copy);
			return *this;
		}

#ifdef __cpp_rvalue_references
		poly_value_ptr(poly_value_ptr&& inPtr) : m_manager(inPtr.m_manager), m_data(inPtr.m_data) {
			inPtr.m_data = NULL;
		}
		poly_value_ptr& operator=(poly_value_ptr&& inPtr) {
			this->reset();
			m_manager = inPtr.m_manager;
			m_data = inPtr.m_data;
			inPtr.m_data = NULL;
			return *this;
		}
		template<typename U>
		poly_value_ptr(poly_value_ptr<U>&& inPtr, typename dp::enable_if<dp::detail::valid_poly_ptr_type<T, U>::value, bool>::type = true) : m_manager(inPtr.m_manager), m_data(inPtr.m_data) {
			inPtr.m_data = NULL;
		}
		template<typename U>
		typename dp::enable_if<dp::detail::valid_poly_ptr_type<T, U>::value, poly_value_ptr&>::type operator=(poly_value_ptr<U>&& inPtr) {
			this->reset();
			m_manager = inPtr.m_manager;
			m_data = inPtr.m_data;
			inPtr.m_data = NULL;
			return *this;
		}
#endif
		~poly_value_ptr() {
			this->reset();
		}

		const T* get() const {
			return m_data;
		}
		T* get() {
			return m_data;
		}

		void swap(poly_value_ptr& other) {
			using std::swap;
			swap(m_data, other.m_data);
			swap(m_manager, other.m_manager);
		}

		T* release() {
			T* temp = m_data;
			m_data = NULL;
			m_manager = NULL;
			return temp;
		}

		void reset() {
			m_manager(op::destroy, this);
			m_manager = NULL;
			m_data = NULL;
		}

		void reset(T* in) {
			if (m_data != in) {
				m_manager(op::destroy, this);
				m_data = in;
			}
		}

		template<typename U>
		void reset(U* in) {
			if (m_data != in) {
				m_manager(op::destroy, this);
				m_manager = &manager<U>::manage;
				m_data = in;
			}
		}

		operator bool() const {
			return m_data;
		}

		const T& operator*() const {
			STATIC_ASSERT(!dp::is_array<T>::value);
			return *m_data;
		}
		T& operator*() {
			STATIC_ASSERT(!dp::is_array<T>::value);
			return *m_data;
		}

		const T* operator->() const {
			STATIC_ASSERT(!dp::is_array<T>::value);
			return m_data;
		}
		T* operator->() {
			STATIC_ASSERT(!dp::is_array<T>::value);
			return m_data;
		}

		const T& operator[](std::size_t index) const {
			STATIC_ASSERT(dp::is_array<T>::value);
			return m_data[index];
		}
		T& operator[](std::size_t index) {
			STATIC_ASSERT(dp::is_array<T>::value);
			return m_data[index];
		}
	};

	//Undefined because it would open a whole can of worms
	template<>
	class poly_value_ptr<void>;

	//And of course our freestanding swap
	template<typename T>
	void swap(poly_value_ptr<T>& lhs, poly_value_ptr<T>& rhs){
		lhs.swap(rhs);
	}



	//We also provide a quick and easy static and dynamic cast overload
	template<typename U, typename T>
	U* static_ptr_cast(const dp::poly_value_ptr<T>& in) {
		STATIC_ASSERT((dp::detail::valid_poly_ptr_type<T, U>::value || dp::is_same<U, void>));
		return static_cast<U*>(const_cast<T*>(in.get()));
	}
	template<typename U, typename T>
	U* dynamic_pointer_cast(const dp::poly_value_ptr<T>& in) {
		//First let's try to filter out obvious misuse
		STATIC_ASSERT((dp::detail::valid_poly_ptr_type<T, U>::value));
		return dynamic_cast<U*>(const_cast<T*>(in.get()));
	}






}



#endif