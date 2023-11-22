#ifndef DP_CPP17_DEFER
#define DP_CPP17_DEFER

#if defined(DP_CPP98_DEFER) || defined(DP_DEFER)
#error "Multiple different defer types detected. Use one or the other"
#endif

#include <functional>
#include <type_traits>

namespace dp {

	template<typename Callable, std::enable_if_t<std::is_invocable_v<Callable>, bool> = true>
	class Defer {

#ifndef DP_DEFER_ALLOW_NON_NOEXCEPT
		//For error handling purposes we hard-require that a deferred statement be noexcept
		static_assert(noexcept(Callable), "Deferred callables must be noexcept");
#endif

		//Filter out function types down to function pointers
		using defer_type = std::decay_t<Callable>;

		defer_type m_call;

	public:

		template<typename T = Callable, std::enable_if_t<std::is_convertible_v<T, Callable>, bool> = true>
		constexpr Defer(T&& in) : m_call{ std::forward<T>(in) } {}

		//By definition this is a scope-local, one-and-done tool. We don't want multiple copies of the same deferral
		Defer() = delete;
		Defer(const Defer&) = delete;
		Defer(Defer&&) = delete;

		Defer& operator=(const Defer&) = delete;
		Defer& operator=(Defer&&) = delete;


		~Defer() noexcept {
			std::invoke(m_call);
		}
	};
	template<typename T>
	Defer(T) -> Defer<T>;

}

/*
*   In the macro case, we want to be able to generate implicit Defer instances with unique names.
*   As such we use __COUNTER__ if it's a available and __LINE__ if not, and concat each one such that
*   the macro generates classes called Defer_Struct1, Defer_Struct2, Defer_Struct3. Unique but still
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
#define DEFER(...) [[maybe_unused]] auto DEFER_CONCAT_MACRO(Defer_Struct, DEFER_COUNT) = dp::Defer([&]() noexcept { __VA_ARGS__ ;});




#endif