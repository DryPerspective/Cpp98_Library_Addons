#ifndef DP_CPP98_VALUE_PTR
#define DP_CPP98_VALUE_PTR

#include <algorithm>
#include "bits/smart_ptr_bases.h"
#include "cpp98/static_assert.h"
#include "bits/type_traits_ns.h"


/*
*	A "value pointer". A pointer to a free-store allocated resource which gives that resource value semantics.
*   This allows the implciitly generated copy operations for a class to make the correct copies automatically
*	While this is a smart pointer, and will automatically delete its pointed-to resource; the primary purpose of this class
*	is to allow for value semantics rather than more directly as a type of ownership. As such it does not see as much
*	interoperability with the other smart pointer types.
*/

namespace dp {


	template<typename T, typename dp::enable_if<dp::is_value_type<T>::value, bool>::type = true>
	class value_ptr {

		T* m_data;

	public:
		
		typedef T		element_type;
		typedef T*		pointer;

		explicit value_ptr() : m_data(NULL) {}
		explicit value_ptr(T* in) : m_data(in) {}

		value_ptr(const value_ptr<T>& in) : m_data(in.m_data ? new T(*in.m_data) : NULL) {}
		value_ptr& operator=(const value_ptr<T>& in) {
			value_ptr copy(in);
			this->swap(copy);
			return *this;
		}

		//Because move semantics will make a significant difference, and because we're not bound to match the standard library 
		//as much in this addon lib
#ifdef __cpp_rvalue_references
		value_ptr(value_ptr<T>&& in) : m_data(in.m_data) {
			in.m_data = NULL;
		}
		value_ptr& operator=(value_ptr<T>&& in) {
			this->reset();
			m_data = in.m_data;
			in.m_data = NULL;
		}
#endif
		~value_ptr() {
			this->reset();
		}

		const T* get() const {
			return m_data;
		}
		T* get() {
			return m_data;
		}

		void swap(value_ptr& other) {
			using std::swap;
			swap(m_data, other.m_data);
		}

		T* release() {
			T* temp = m_data;
			m_data = NULL;
			return temp;
		}

		void reset(T* in = NULL) {
			if (m_data != in) {
				//Use a default delete to ensure we perform the correct deletion
				dp::default_delete<T>()(m_data);
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
			return *m_data;
		}
		T* operator->() {
			STATIC_ASSERT(!dp::is_array<T>::value);
			return *m_data;
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



	//And our usual swap function
	template<typename T>
	void swap(dp::value_ptr<T>& lhs, dp::value_ptr<T>& rhs) {
		lhs.swap(rhs);
	}

	template<typename T>
	bool operator==(const value_ptr<T>& lhs, const value_ptr<T>& rhs) {
		return lhs.get() == rhs.get();
	}
	template<typename T>
	bool operator!=(const value_ptr<T>& lhs, const value_ptr<T>& rhs) {
		return !(lhs == rhs);
	}
	template<typename T>
	bool operator<(const value_ptr<T>& lhs, const value_ptr<T>& rhs) {
		return lhs.get() < rhs.get();
	}
	template<typename T>
	bool operator<=(const value_ptr<T>& lhs, const value_ptr<T>& rhs) {
		return (lhs < rhs) || (lhs == rhs);
	}
	template<typename T>
	bool operator>(const value_ptr<T>& lhs, const value_ptr<T>& rhs) {
		return !(lhs <= rhs);
	}
	template<typename T>
	bool operator>=(const value_ptr<T>& lhs, const value_ptr<T>& rhs) {
		return !(lhs < rhs);
	}

	template<typename T>
	bool operator==(const value_ptr<T>& lhs, dp::null_ptr_t rhs) {
		return lhs.get() == NULL;
	}
	template<typename T>
	bool operator==(dp::null_ptr_t lhs, const value_ptr<T>& rhs) {
		return rhs.get() == NULL;
	}
	template<typename T>
	bool operator!=(const value_ptr<T>& lhs, dp::null_ptr_t rhs) {
		return lhs.get();
	}
	template<typename T>
	bool operator!=(dp::null_ptr_t lhs, const dp::value_ptr<T>& rhs) {
		return rhs.get();
	}





}


#endif