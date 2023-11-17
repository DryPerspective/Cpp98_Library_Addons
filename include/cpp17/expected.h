#ifndef DP_CPP17_EXPECTED
#define DP_CPP17_EXPECTED

#ifdef DP_CPP98_EXPECTED
#error "Both C++98 and C++17 dp::expected detected. Only use one or the other"
#endif

#include <stdexcept>
#include <variant>
#include <utility>
#include <type_traits>

/*
*	An analogue of std::expected, written in C++17
*	Intended as an update to my C++98 expected but is entirely standalone and C++17-compliant
*
*/
namespace dp {

	template<typename T>
	class bad_expected_access;

	template<>
	class bad_expected_access<void> : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Bad expected access";
		}
	};

	template<typename Err>
	class bad_expected_access {
		Err m_error;

	public:
		explicit bad_expected_access(Err e) : m_error(std::move(e)) {}

		Err& error() & noexcept {
			return m_error;
		}
		const Err& error() const& noexcept {
			return m_error;
		}
		Err&& error() && noexcept {
			return std::move(m_error);
		}
		const Err&& error() const&& noexcept {
			return std::move(m_error);
		}
	};


	template<typename Err>
	class unexpected {
		Err m_error;

	public:
		constexpr unexpected(const unexpected&) = default;
		constexpr unexpected(unexpected&&) = default;

		template<typename E = Err>
		constexpr explicit unexpected(E&& in) : m_error{ std::forward<E>(in) } {}

		template<typename... Args, std::enable_if_t<std::is_constructible_v<Err, Args...>, bool> = true>
		constexpr explicit unexpected(std::in_place_t, Args&&... args) : m_error{ std::forward<Args...>(args...) } {}

		template<typename U, typename... Args, std::enable_if_t<std::is_constructible_v<Err, std::initializer_list<U>, Args...>, bool> = true>
		constexpr explicit unexpected(std::in_place_t, std::initializer_list<U>& inList, Args&&... args) : m_error{ inList, std::forward<Args...>(args...) } {}

		constexpr Err& error() & noexcept {
			return m_error;
		}
		constexpr const Err& error() const& noexcept {
			return m_error;
		}
		constexpr Err&& error() && noexcept {
			return std::move(m_error);
		}
		constexpr const Err&& error() const&& noexcept {
			return std::move(m_error);
		}

		constexpr void swap(dp::unexpected<Err>& other) noexcept(std::is_nothrow_swappable_v<Err>) {
			using std::swap;
			swap(m_error, other.m_error);
		}

	};
	template<typename Err>
	unexpected(Err) -> unexpected<Err>;


	template<typename Err, std::enable_if_t<std::is_swappable_v<Err>, bool> = true>
	constexpr void swap(dp::unexpected<Err>& lhs, dp::unexpected<Err>& rhs) noexcept(std::is_nothrow_swappable_v<Err>) {
		lhs.swap(rhs);
	}

	template<typename Err, typename Other>
	constexpr bool operator==(const dp::unexpected<Err>& lhs, const dp::unexpected<Other>& rhs) {
		return lhs.error() == rhs.error();
	}


	//Layover from the days of C++98 where we didn't have CTAD or mandatory prvalue elision
	template<typename Err>
	[[deprecated("dp::unex is deprecated. Use dp::unexpected(error) instead")]]
	constexpr dp::unexpected<Err> unex(Err&& in) {
		return dp::unexpected(std::forward<Err>(in));
	}


	struct unexpect_t {
		explicit unexpect_t() = default;
	};

	constexpr inline unexpect_t unexpect{};

	namespace detail {
		template<typename T>
		static constexpr inline bool is_special_of_unexpected = false;

		template<typename T>
		static constexpr inline bool is_special_of_unexpected<dp::unexpected<T>> = true;

		template<typename T>
		using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
	}





	template<typename T, typename E>
	class expected {
		//Clear out the ill-formed versions
		static_assert(!std::is_reference_v<T> && !std::is_reference_v<E>, "Cannot create an expected of reference type");
		static_assert(!std::is_function_v<T> && !std::is_function_v<E>, "Cannot create an expected of function type");
		static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t> && !std::is_same_v <std::remove_cv_t<T>, unexpect_t>, "Cannot create an expected of in_place_t or unexpect_t");
		static_assert(!detail::is_special_of_unexpected<T> && !detail::is_special_of_unexpected<E>, "Cannot use dp::unexpected for the type of dp::expected");


		//Because it's possible for T to be void, and because we must always hold a value, we need to account for this here
		static constexpr bool T_is_void{ std::is_same_v<std::remove_cv_t<T>, void> };
		template<typename valT>
		using data_type = std::conditional_t <T_is_void, std::variant<std::monostate, E>, std::variant<valT, E>>;

		data_type<T> m_data;

	public:

		using value_type = T;
		using error_type = E;
		using unexpected_type = dp::unexpected<E>;

		template<typename U>
		using rebind = dp::expected<U, error_type>;

		constexpr expected() noexcept {
			static_assert(std::is_default_constructible_v<T> || T_is_void);
		}

		constexpr expected(const expected&) = default;
		constexpr expected(expected&&) noexcept = default;

		template<typename U = T, std::enable_if_t<!T_is_void && std::is_convertible_v<U,T>, bool> = true>
		constexpr expected(U&& in) : m_data{ std::in_place_index<0>, std::forward<U>(in) } {}

		template<typename G, std::enable_if_t<std::is_constructible_v<const G&, E>, bool> = true>
		constexpr expected(const dp::unexpected<G>& unex) : m_data{ std::in_place_index<1>, unex.error() } {}

		template<typename G, std::enable_if_t<std::is_constructible_v<G&&, E>, bool> = true>
		constexpr expected(dp::unexpected<G>&& unex) : m_data{ std::in_place_index<1>, std::move(unex.error()) } {}

		template<typename... Args, std::enable_if_t<!T_is_void, bool> = true>
		constexpr expected(std::in_place_t, Args&&... args) : m_data{ std::in_place_index<0>, std::forward<Args...>(args...) } {}

		template<typename U, typename... Args, std::enable_if_t<!T_is_void && std::is_constructible_v<T, std::initializer_list<U>&, Args...>, bool> = true>
		constexpr expected(std::in_place_t, std::initializer_list<U> inList, Args&&... args) : m_data{ std::in_place_index<0>, inList, std::forward<Args...>(args...) } {}

		template<typename... Args, std::enable_if_t<!T_is_void, bool> = true>
		constexpr expected(std::in_place_t) {}

		template<typename... Args, std::enable_if_t<std::is_constructible_v<E, Args...>, bool> = true>
		constexpr expected(dp::unexpect_t, Args&&... args) : m_data{ std::in_place_index<1>, std::forward<Args...>(args...) } {}

		template<typename U, typename... Args, std::enable_if_t<std::is_constructible_v<E, std::initializer_list<U>&, Args...>, bool> = true>
		constexpr expected(std::in_place_t, std::initializer_list<U> inList, Args&&... args) : m_data{ std::in_place_index<1>, inList, std::forward<Args...>(args...) } {}

		//No constexpr destructors until C++20. Big sad.
		~expected() = default;


		constexpr expected& operator=(const expected&) = default;
		constexpr expected& operator=(expected&&) noexcept = default;

		/*
		*  Variant assignment comes with a lot of exception guarantees so we can be less stringent about exception safety in this user class
		*/
		//What I wouldn't do for concept syntax...
		template<typename U = T, std::enable_if_t<
			!std::is_same_v<expected, detail::remove_cvref_t<U>> &&
			!detail::is_special_of_unexpected<detail::remove_cvref_t<U>> &&
			std::is_constructible_v<T,U> &&
			std::is_assignable_v<T&, U> &&
			(std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>), bool> = true>
		constexpr expected& operator=(U&& inVal) {
			if constexpr (std::is_nothrow_assignable_v<T, decltype(std::forward<U>(inVal))>) {
				if (this->has_value()) **this = std::forward<U>(inVal);
				else m_data = data_type{ std::in_place_index<0>, std::forward<U>(inVal) };
			}
			else {
				m_data = data_type{ std::in_place_index<0>, std::forward<U>(inVal) };
			}
			return *this;
		}

		template<typename G, std::enable_if_t<
			std::is_constructible_v<E, const G&> &&
			std::is_assignable_v<E&, const G&> &&
			(T_is_void || std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>), bool> = true>
		constexpr expected& operator=(const dp::unexpected<G>& inUnex) {
			if constexpr (std::is_nothrow_assignable_v<E, decltype(inUnex.error())>) {
				this->error() = inUnex.error();
			}
			else {
				m_data = data_type{ std::in_place_index<1>, inUnex.error() };
			}
			return this;
		}

		template<typename G, std::enable_if_t<
			std::is_constructible_v<E, G&&> &&
			std::is_assignable_v<E&, G&&> &&
			(T_is_void || std::is_nothrow_constructible_v<E, G&&> || std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>), bool> = true>
		constexpr expected& operator=(dp::unexpected<G>&& inUnex) {
			if constexpr (std::is_nothrow_assignable_v<E, G&&>) {
				this->error() = inUnex.error();
			}
			else {
				m_data = data_type{ std::in_place_index<1>, std::move(inUnex.error()) };
			}
			return *this;
		}


		/*
		*  Access is safer than specified by the standard. This may be reassessed in future.
		*/
		constexpr std::conditional_t<T_is_void, void, const T&> operator*() const & {
			if constexpr (T_is_void) return;
			else {
				return std::get<0>(m_data);
			}
		}
		constexpr T& operator*() & {
			return std::get<0>(m_data);
		}
		constexpr std::conditional_t<T_is_void, void, const T&&> operator*() const && {
			if constexpr (T_is_void) return;
			else {
				return std::move(std::get<0>(m_data));
			}
		}
		constexpr T&& operator*() && {
			return std::move(std::get<0>(m_data));
		}

		constexpr const T* operator->() const {
			return &std::get<0>(m_data);
		}
		constexpr T* operator->() {
			return &std::get<0>(m_data);
		}

		constexpr T& value() & {
			if (this->has_value()) return **this;
			throw dp::bad_expected_access(std::as_const(error()));
		}

		constexpr std::conditional_t <T_is_void, void, const T&> value() const & {
			if constexpr (T_is_void) return;
			else {
				if (this->has_value()) return **this;
				throw dp::bad_expected_access(std::as_const(error()));
			}
		}

		constexpr std::conditional_t <T_is_void, void, T&&> value()&& {
			if constexpr (T_is_void) return;
			else {
				if (this->has_value()) return std::move(**this);
				throw dp::bad_expected_access(std::move(error()));
			}
		}
		constexpr const T&& value() const&& {
			if (this->has_value()) return std::move(**this);
			throw dp::bad_expected_access(std::move(error()));
		}

		constexpr const E& error() const& {
			return std::get<1>(m_data);
		}
		constexpr E& error() & {
			return std::get<1>(m_data);
		}
		constexpr const E&& error() const&& {
			return std::move(std::get<1>(m_data));
		}
		constexpr E&& error()&& {
			return std::move(std::get<1>(m_data));
		}


		constexpr bool has_value() const noexcept {
			return m_data.index() == 0;
		}
		constexpr explicit operator bool() const noexcept {
			return has_value();
		}

		template<typename U>
		constexpr T value_or(U&& in) const & {
			return static_cast<bool>(*this) ? **this : static_cast<T>(std::forward<U>(in))
		}
		template<typename U>
		constexpr T value_or(U&& in)&& {
			return static_cast<bool>(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(in));
		}

		template<typename... Args>
		constexpr T& emplace(Args&&... args) noexcept {
			return m_data.emplace(std::forward<Args...>(args...));
		}
		template<typename U, typename... Args>
		constexpr T& emplace(std::initializer_list<U> inList, Args&&... args) noexcept {
			return m_data.emplace(inList, std::forward<Args...>(args...));
		}

		constexpr void swap(expected& other) {
			m_data.swap(other.m_data);
		}


	};

	template<typename T1, typename E1, typename T2, typename E2>
	constexpr bool operator==(const dp::expected<T1,E1>& lhs, const dp::expected<T2,E2>& rhs) noexcept {
		return (lhs.has_value() && rhs.has_value() && *lhs == *rhs);
	}


	template<typename T, typename E>
	void swap(dp::expected<T, E>& lhs, dp::expected<T, E>& rhs) noexcept {
		lhs.swap(rhs);
	}





}







#endif