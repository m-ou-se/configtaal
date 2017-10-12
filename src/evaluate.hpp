#pragma once
#include "error.hpp"
#include "expression.hpp"
#include "operator.hpp"
#include "value.hpp"

#include <map>
#include <functional>
#include <optional>

namespace conftaal {

class EvaluateError : public Error {
public:
	using Error::Error;
};

class Engine;

/// Test if a value is less than another by delegating to an evaluation engine.
class ValueLess {
	Engine const * engine;
	bool operator() (Value const & a, Value const & b);
};

class Engine {
public:
	using BinaryOpKey = std::tuple<Operator, std::type_index, std::type_index>;
	using BinaryOp    = std::function<Value (Value const &, Value const &)>;
	using UnaryOpKey  = std::tuple<Operator, std::type_index>;
	using UnaryOp     = std::function<Value (Value const &)>;
	using Function    = std::function<Value(ValueList const &)>;

	using Object = std::map<std::string, Value>;
	using List   = ValueList;

	using ContextStack = std::vector<Object const *>;

	std::map<BinaryOpKey, BinaryOp> binary_ops;
	std::map< UnaryOpKey,  UnaryOp> unary_ops;
	std::map<std::string, Function> functions;

	std::vector<Object> prelude;

	template<typename R, typename T1, typename T2> void addBinaryOp(Operator op, R (*f) (T1 const &, T2 const &));
	template<typename T1, typename T2, typename F> void addBinaryOp(Operator op, F f);

	template<typename R, typename T> void addUnaryOp(Operator op, R (*f) (T const &));
	template<typename T, typename F> void addUnaryOp(Operator op, F f);

	template<typename F, typename... Args> void addFunction(std::string name, F f);
	template<typename R, typename... Args> void addFunction(std::string name, R (*) (Args...));

	std::optional<BinaryOp> findBinaryOp(Operator op, std::type_index a, std::type_index b) const;
	std::optional<BinaryOp> findBinaryOp(Operator op, Value const & a, Value const & b) const;
	std::optional<BinaryOp> findUnaryOp(Operator op, std::type_index a, std::type_index b) const;
	std::optional<BinaryOp> findUnaryOp(Operator op, Value const & a, Value const & b) const;
	std::optional<Function> findFunction(std::string_view name) const;

	Value evaluate(refcount_ptr<Expression const> expr) const;
	Value evaluate(refcount_ptr<Expression const> expr, ContextStack & context) const;

protected:
	Value evaluate(refcount_ptr<IdentifierExpression const> expr, ContextStack & context) const;
	Value evaluate(refcount_ptr<OperatorExpression const> expr, ContextStack & context) const;

	Value evaluate(refcount_ptr<LiteralExpression const> expr) const;

	List evaluate(refcount_ptr<ListExpression const> expr, ContextStack & context) const;
	Object evaluate(refcount_ptr<ObjectExpression const> expr, ContextStack & context) const;
};

void imbueDefaultPrelude(Engine & engine);

template<typename R, typename T1, typename T2>
void Engine::addBinaryOp(Operator op, R (*f) (T1 const &, T2 const &)) {
	binary_ops.insert({{op, Value::type_of<T1>(), Value::type_of<T2>()}, [f] (Value const & a, Value const & b) -> Value {
		return f(*a.as_ptr<T1>(), *b.as_ptr<T2>());
	}});
}

template<typename T1, typename T2, typename F>
void Engine::addBinaryOp(Operator op, F f) {
	static_assert(std::is_invocable_v<F, T1, T2>, "Given functor can not be invoked with specified arguments.");
	binary_ops.insert({{op, Value::type_of<T1>(), Value::type_of<T2>()}, [f = std::forward<F>(f)] (Value const & a, Value const & b) -> Value {
		return std::invoke(f, *a.as_ptr<T1>(), *b.as_ptr<T2>());
	}});
}

template<typename R, typename T>
void Engine::addUnaryOp(Operator op, R (*f) (T const &)) {
	unary_ops.insert({op, Value::type_of<T>(), [f] (Value const & value) -> Value {
		return f(*value.as_ptr<T>());
	}});
}

template<typename T, typename F>
void Engine::addUnaryOp(Operator op, F f) {
	static_assert(std::is_invocable_v<F, T>, "Given functor can not be invoked with specified arguments.");
	unary_ops.insert({op, Value::type_of<T>(), [f = std::forward<F>(f)] (Value const & value) -> Value {
		return std::invoke(f, *value.as_ptr<T>());
	}});
}

namespace detail {
	template<typename T, std::size_t I>
	T convert(Value const & value) {
		T const * p = value.as_ptr<T>();
		if (!p) throw EvaluateError("wrong type for argument " + std::to_string(I));
		return *p;
	}

	template<typename F, typename Args, std::size_t... I>
	Value invoke(F f, ValueList const & args, std::index_sequence<I...>) {
		constexpr std::size_t wanted = sizeof...(I);
		if (args.size() != wanted) throw EvaluateError("function expects exactly " + std::to_string(wanted) + " arguments, got " + std::to_string(args.size()) + " arguments");
		return std::invoke(std::forward<F>(f), convert<std::tuple_element_t<I, Args>, I>(args[I])...);
	}
}

template<typename F, typename... Args>
void Engine::addFunction(std::string name, F f) {
	static_assert(std::is_invocable_v<F, Args...>, "Given functor can not be invoked with specified arguments.");
	functions.insert({std::move(name), [f = std::forward<F>(f)] (ValueList const & args) -> Value {
		return detail::invoke<F, std::tuple<Args...>>(f, args, std::index_sequence_for<Args...>());
	}});
}

template<typename R, typename... Args>
void Engine::addFunction(std::string name, R (*f) (Args...)) {
	functions.insert({std::move(name), [f] (ValueList const & args) -> Value {
		return detail::invoke<decltype(f), std::tuple<Args...>>(f, args, std::index_sequence_for<Args...>());
	}});
}

}
