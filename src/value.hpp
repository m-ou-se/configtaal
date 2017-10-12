#include <map>
#include <memory>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace conftaal {

namespace detail {
	template<typename T> struct ValueImpl;

	struct ValueBase {
		virtual std::unique_ptr<ValueBase> clone() = 0;
		~ValueBase() {}

		template<typename T>
		ValueImpl<T> * cast() {
			return dynamic_cast<ValueImpl<T> *>(this);
		}

		template<typename T>
		ValueImpl<T> const * cast() const {
			return dynamic_cast<ValueImpl<T> const *>(this);
		}
	};

	template<typename T>
	struct ValueImpl : ValueBase {
		T value;

		ValueImpl(T const & value) : value{value} {}
		ValueImpl(T && value) : value{std::move(value)} {}

		template<typename... Args>
		explicit ValueImpl(std::in_place_t, Args && ...args) : value(std::forward<Args>(args)...) {}

		std::unique_ptr<ValueBase> clone() override {
			return std::make_unique<ValueImpl>(value);
		}
	};

	template<typename T>
	std::optional<T> as_option(T const * p) {
		return p ? std::optional<T>{*p} : std::nullopt;
	}

	template<typename T>
	std::optional<T> into_option(T * p) {
		return p ? std::optional<T>{std::move(*p)} : std::nullopt;
	}

	template<typename T>
	std::unique_ptr<ValueBase> wrap_value(T && val) {
		return std::make_unique<ValueImpl<std::decay_t<T>>>(std::forward<T>(val));
	}

	template<typename T, typename... Args>
	std::unique_ptr<ValueBase> make_value(Args && ...args) {
		return std::make_unique<ValueImpl<T>>(std::in_place, std::forward<Args>(args)...);
	}
}

/// Class capable of holding almost any type of object.
/**
 * Held objects must be CopyConstructible.
 */
class Value {
	std::unique_ptr<detail::ValueBase> value_;

public:
	/// Copy-construct a value.
	Value(Value const & other) : value_{other.value_->clone()} {}

	/// Move-construct a value.
	Value(Value && other) = default;

	/// Copy-assign a value.
	Value & operator= (Value const & other) {
		value_ = other.value_->clone();
		return *this;
	}

	/// Move-assign a value.
	Value & operator= (Value &&) = default;

	/// Construct a value from any compatible object.
	template<typename T>
	explicit Value(T && val) : value_{detail::wrap_value(std::forward<T>(val))} {}

	/// Get the type index used for the specified type.
	template<typename T>
	static std::type_index type_of() noexcept { return {typeid(T *)}; }

	/// Swap the contents of two values.
	void swap(Value & other) noexcept { value_.swap(other.value_); }

	/// Get the held object as a pointer.
	/**
	 * \return a pointer to the held object, or null if the type doesn't match.
	 */
	template<typename T> T * as_ptr() noexcept {
		if (auto p = value_->cast<T>()) return &p->value;
		return nullptr;
	}

	template<typename T> T const * as_ptr() const noexcept {
		if (auto p = value_->cast<T>()) return &p->value;
		return nullptr;
	}

	/// Get the held object.
	/**
	 * \return a copy of the held object, or an empty optional if the type doesn't match.
	 */
	template<typename T> std::optional<T> as() const & { return detail::as_option(as_ptr<T>()); }

	/// Move the held object out of the value.
	/**
	 * \return the held object, or an empty optional if the type doesn't match.
	 */
	template<typename T> std::optional<T> as() && { return detail::into_option(as_ptr<T>()); }

	/// Check if the value holds an object of the specified type.
	template<typename T> bool is() const noexcept { return as_ptr<T>() != nullptr; }

	/// Get the type index associated with the currently held object.
	std::type_index type() const noexcept { return std::type_index(typeid(value_.get())); }
};

/// Make a value using in-place construction of the underlying object.
template<typename T, typename... Args>
Value make_value(Args && ...args) {
	return Value{detail::make_value<T>(std::forward<Args>(args)...)};
}

using ValueList = std::vector<Value>;

template<typename Comperator>
using ValueMap  = std::map<Value, Value, Comperator>;

}
