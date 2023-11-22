#ifndef DP_CPP17_DEFER
#define DP_CPP17_DEFER

#if defined(DP_CPP98_DEFER) || defined(DP_DEFER)
#error "Multiple different defer types detected. Use one or the other"
#endif

#include <functional>
#include <type_traits>

namespace dp {

	template<typename Callable, std::enable_if_t<std::is_invocable_v<Callable>, bool> = true>
	class defer {


		//Filter out function types down to function pointers
		using defer_type = std::decay_t<Callable>;

		defer_type m_call;


	public:

		template<typename T = Callable, std::enable_if_t<std::is_convertible_v<T, Callable>, bool> = true>
		constexpr defer(T&& in) : m_call{ std::forward<T>(in) } {}

		//By definition this is a scope-local, one-and-done tool. We don't want multiple copies of the same deferral
		defer() = delete;
		defer(const defer&) = delete;
		defer(defer&&) = delete;

		defer& operator=(const defer&) = delete;
		defer& operator=(defer&&) = delete;


		~defer() noexcept {
			std::invoke(m_call);
		}
	};
	template<typename T>
	defer(T) -> defer<T>;

}

/*
*   In the macro case, we want to be able to generate implicit defer instances with unique names.
*   As such we use __COUNTER__ if it's a available and __LINE__ if not, and concat each one such that
*   the macro generates classes called defer_Struct1, defer_Struct2, defer_Struct3. Unique but still
*   understandable and diagnosable if needed.
*/
#define DEFER_CONCAT_IMPL(x,y) x##y
#define DEFER_CONCAT_MACRO( x, y ) DEFER_CONCAT_IMPL( x, y )

#ifdef __COUNTER__
#define DEFER_COUNT __COUNTER__
#else
#define DEFER_COUNT __LINE__
#endif

//Note that a [&] isn't the sledgehammer it may appear to be.
//A lambda is not required to actually capture variables it does not use, and since this is a scope-local construct we are unlikely to run into scope issues
//Varidadic macro to prevent commas being problematic
#define DEFER(...) [[maybe_unused]] auto DEFER_CONCAT_MACRO(defer_Struct, DEFER_COUNT) = dp::defer([&]() noexcept { __VA_ARGS__ ;});




#endif