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

	Engine();

	template<typename R, typename T1, typename T2> void addBinaryOp(Operator op, R (*f) (T1, T2));
	template<typename T1, typename T2, typename F> void addBinaryOp(Operator op, F f);

	template<typename R, typename T> void addUnaryOp(Operator op, R (*f) (T));
	template<typename T, typename F> void addUnaryOp(Operator op, F f);

	template<typename F, typename... Args> void addFunction(std::string name, F f);
	template<typename R, typename... Args> void addFunction(std::string name, R (*) (Args...));

	Value evaluate(refcount_ptr<Expression const> expr) const;
	Value evaluate(refcount_ptr<Expression const> expr, ContextStack & context) const;

protected:
	Value evaluateIdentifier(refcount_ptr<IdentifierExpression const> expr, ContextStack & context) const;
	Value evaluateOperator(refcount_ptr<OperatorExpression const> expr, ContextStack & context) const;

	Value evaluateLiteral(refcount_ptr<LiteralExpression const> expr) const;

	List evaluateList(refcount_ptr<ListExpression const> expr, ContextStack & context) const;
	Object evaluateObject(refcount_ptr<ObjectExpression const> expr, ContextStack & context) const;
};

void imbueDefaultPrelude(Engine & engine);

template<typename R, typename T1, typename T2>
void Engine::addBinaryOp(Operator op, R (*f) (T1, T2)) {
	binary_ops.insert({{op, TypeInfo::of<std::decay_t<T1>>(), TypeInfo::of<std::decay_t<T2>>()}, [f] (Value const & a, Value const & b) {
		return Value{f(a.unchecked<T1>(), b.unchecked<T2>())};
	}});
}

template<typename T1, typename T2, typename F>
void Engine::addBinaryOp(Operator op, F f) {
	static_assert(std::is_invocable_v<F, T1, T2>, "Given functor can not be invoked with specified arguments.");
	binary_ops.insert({{op, TypeInfo::of<T1>(), TypeInfo::of<T2>()}, [f = std::forward<F>(f)] (Value const & a, Value const & b) {
		return Value{std::invoke(f, a.unchecked<T1>(), b.unchecked<T2>())};
	}});
}

template<typename R, typename T>
void Engine::addUnaryOp(Operator op, R (*f) (T)) {
	unary_ops.insert({op, TypeInfo::of<std::decay_t<T>>(), [f] (Value const & value) {
		return Value{f(value.unchecked<T>())};
	}});
}

template<typename T, typename F>
void Engine::addUnaryOp(Operator op, F f) {
	static_assert(std::is_invocable_v<F, T>, "Given functor can not be invoked with specified arguments.");
	unary_ops.insert({op, TypeInfo::of<T>(), [f = std::forward<F>(f)] (Value const & value) {
		return Value{std::invoke(f, value.unchecked<T>())};
	}});
}

namespace detail {
	template<typename T, std::size_t I>
	T convert(Value const & value) {
		T const * p = value.as_ptr<T>();
		if (!p) throw EvaluateError("type mismatch for argument " + std::to_string(I));
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
