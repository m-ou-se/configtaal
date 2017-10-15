#include "evaluate.hpp"

namespace conftaal {

Engine::Engine() {
	imbueDefaultPrelude(*this);
}

Value Engine::evaluate(refcount_ptr<Expression const> expr) const {
	// Fill initial context with the prelude.
	ContextStack context{};
	context.resize(prelude.size());
	for (auto const & object : prelude) context.push_back(&object);

	return evaluate(std::move(expr), context);
}

Value Engine::evaluate(refcount_ptr<Expression const> expr, ContextStack & context) const {
	if (auto e = dynamic_pointer_cast<IdentifierExpression const>(expr)) return evaluateIdentifier(std::move(e), context);
	if (auto e = dynamic_pointer_cast<  OperatorExpression const>(expr)) return evaluateOperator(std::move(e), context);
	if (auto e = dynamic_pointer_cast<   LiteralExpression const>(expr)) return evaluateLiteral(std::move(e));
	if (auto e = dynamic_pointer_cast<      ListExpression const>(expr)) return Value{evaluateList(std::move(e), context)};
	if (auto e = dynamic_pointer_cast<    ObjectExpression const>(expr)) return Value{evaluateObject(std::move(e), context)};
	throw EvaluateError("unknown expression type");
}

Value Engine::evaluateIdentifier(refcount_ptr<IdentifierExpression const> expr, ContextStack & context) const {
	std::string key{expr->identifier};
	for (auto object = context.rbegin(); object != context.rend(); ++object) {
		auto i = (*object)->find(key);
		if (i != (*object)->end()) return i->second;
	}
	throw EvaluateError{"could not resolve identifier: " + key};
}

Value Engine::evaluateOperator(refcount_ptr<OperatorExpression const> expr, ContextStack & context) const {
	// Binary operators.
	if (expr->lhs) {
		Value lhs = evaluate(expr->lhs, context);
		Value rhs = evaluate(expr->rhs, context);
		auto i = binary_ops.find({expr->op, lhs.type(), rhs.type()});
		if (i == binary_ops.end()) throw EvaluateError("operator " + std::string{expr->op_source} + " not defined for given types");
		return i->second(lhs, rhs);

	// Unary operators.
	} else {
		Value rhs = evaluate(expr->lhs, context);
		auto i = unary_ops.find({expr->op, rhs.type()});
		if (i == unary_ops.end()) throw EvaluateError("operator " + std::string{expr->op_source} + " not defined for given types");
		return i->second(rhs);
	}
}

Value Engine::evaluateLiteral(refcount_ptr<LiteralExpression const> expr) const {
	if (auto e = dynamic_pointer_cast<IntegerLiteralExpression const>(expr)) return Value{e->value};
	if (auto e = dynamic_pointer_cast< DoubleLiteralExpression const>(expr)) return Value{e->value};
	if (auto e = dynamic_pointer_cast< StringLiteralExpression const>(expr)) return Value{e->value};
	throw EvaluateError("unknown literal type");
}

Engine::List Engine::evaluateList(refcount_ptr<ListExpression const> expr, ContextStack & context) const {
	List result;
	result.reserve(expr->elements.size());
	for (refcount_ptr<Expression const> const & elem : expr->elements) {
		result.push_back(evaluate(elem, context));
	}
	return result;
}

Engine::Object Engine::evaluateObject(refcount_ptr<ObjectExpression const> expr, ContextStack & context) const {
	// Evaluate keys first, so we don't lookup parts of keys in the object itself.
	List keys = evaluateList(expr->keys, context);
	for (std::size_t i = 0; i < expr->keys->elements.size(); ++i) {
		if (!keys[i].is<std::string>()) throw EvaluateError("key is not a string");
	}

	// Evaluate values in modified context, so they can refer to already-evaluated parts of the object itself.
	Object result;
	context.push_back(&result);

	// Evaluate values and add them to the object.
	for (std::size_t i = 0; i < expr->keys->elements.size(); ++i) {
		auto insertion = result.insert({
			*keys[i].as_ptr<std::string>(), // already checked that it is a string above
			evaluate(expr->values->elements[i], context),
		});
		if (!insertion.second) throw EvaluateError("duplicate key: " + *keys[i].as_ptr<std::string>());
	}

	// Pop the evaluated object from the context stack again.
	context.pop_back();
	return result;

}

template<Operator op, typename T1, typename T2>
auto default_binary_op(T1 const & a, T2 const & b) {
	if constexpr(op == Operator::equal)                   return a == b;
	else if constexpr(op == Operator::inequal)            return a != b;
	else if constexpr(op == Operator::greater)            return a > b;
	else if constexpr(op == Operator::less)               return a < b;
	else if constexpr(op == Operator::greater_or_equal)   return a >= b;
	else if constexpr(op == Operator::less_or_equal)      return a <= b;

	else if constexpr(op == Operator::plus)    return a + b;
	else if constexpr(op == Operator::minus)   return a - b;
	else if constexpr(op == Operator::times)   return a * b;
	else if constexpr(op == Operator::divide)  return a / b;
	else if constexpr(op == Operator::modulo)  return a % b;
	// can't do power

	else if constexpr(op == Operator::left_shift)  return a << b;
	else if constexpr(op == Operator::right_shift) return a >> b;
	else if constexpr(op == Operator::bit_and)     return a &  b;
	else if constexpr(op == Operator::bit_or)      return a |  b;
	else if constexpr(op == Operator::bit_xor)     return a ^  b;
	else if constexpr(op == Operator::logical_and) return a && b;
	else if constexpr(op == Operator::logical_or)  return a || b;

	else static_assert(op != op, "specified operator can not be delegated to C++ default");
}

template<Operator Op, typename T1, typename T2 = T1>
void delegateBinaryOp(Engine & engine) {
	engine.addBinaryOp<T1, T2>(Op, default_binary_op<Op, T1, T2>);
}

template<typename T1, typename T2, Operator... Ops>
void delegateBinaryOps(Engine & engine) {
	(delegateBinaryOp<Ops, T1, T2>(engine), ...);
}

template<typename T1, typename T2, Operator... Ops>
void delegateMixedBinaryOps(Engine & engine) {
	delegateBinaryOps<T1, T2, Ops...>(engine);
	delegateBinaryOps<T2, T1, Ops...>(engine);
}

void imbueDefaultPrelude(Engine & engine) {
	using Op = Operator;

	delegateBinaryOps<std::int64_t, std::int64_t,
		Op::equal, Op::inequal, Op::greater, Op::less, Op::greater_or_equal, Op::less_or_equal,
		Op::plus, Op::minus, Op::times, Op::divide, Op::modulo,
		Op::left_shift, Op::right_shift, Op::bit_and, Op::bit_or, Op::bit_xor,
		Op::logical_and, Op::logical_or
	>(engine);

	// TODO: Power

	delegateBinaryOps<double, double,
		Op::equal, Op::inequal, Op::greater, Op::less, Op::greater_or_equal, Op::less_or_equal,
		Op::plus, Op::minus, Op::times, Op::divide,
		Op::logical_and, Op::logical_or
	>(engine);

	// TODO: Power, Modulo

	delegateMixedBinaryOps<double, std::int64_t,
		Op::equal, Op::inequal, Op::greater, Op::less, Op::greater_or_equal, Op::less_or_equal,
		Op::plus, Op::minus, Op::times, Op::divide,
		Op::logical_and, Op::logical_or
	>(engine);

	// TODO: Power, Modulo

	delegateBinaryOps<std::string, std::string,
		Op::equal, Op::inequal, Op::greater, Op::less, Op::greater_or_equal, Op::less_or_equal,
		Op::plus
	>(engine);
}

}
