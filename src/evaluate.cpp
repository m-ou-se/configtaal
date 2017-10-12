#include "evaluate.hpp"

namespace conftaal {

bool ValueLess::operator() (Value const & a, Value const & b) {
	if (a.type() < b.type()) return true;
	if (a.type() > b.type()) return false;
	auto i = engine->binary_ops.find({Operator::less, a.type(), b.type()});
	if (i == engine->binary_ops.end()) throw EvaluateError("can not compare values");

	Value result = i->second(a, b);
	if (auto r = result.as<        bool>()) return *r;
	if (auto r = result.as<std::int64_t>()) return *r;
	throw EvaluateError("comparing values did not result in a boolean value");
}

Value Engine::evaluate(refcount_ptr<Expression const> expr) const {
	// Fill initial context with the prelude.
	ContextStack context{};
	context.resize(prelude.size());
	for (auto const & object : prelude) context.push_back(&object);

	return evaluate(std::move(expr), context);
}

Value Engine::evaluate(refcount_ptr<Expression const> expr, ContextStack & context) const {
	if (auto e = dynamic_pointer_cast<IdentifierExpression const>(expr)) return evaluate(std::move(e), context);
	if (auto e = dynamic_pointer_cast<  OperatorExpression const>(expr)) return evaluate(std::move(e), context);
	if (auto e = dynamic_pointer_cast<   LiteralExpression const>(expr)) return evaluate(std::move(e), context);
	if (auto e = dynamic_pointer_cast<      ListExpression const>(expr)) return Value{evaluate(std::move(e), context)};
	if (auto e = dynamic_pointer_cast<    ObjectExpression const>(expr)) return Value{evaluate(std::move(e), context)};
	throw EvaluateError("unknown expression type");
}

Value Engine::evaluate(refcount_ptr<IdentifierExpression const> expr, ContextStack & context) const {
	std::string key{expr->identifier};
	for (auto object = context.rbegin(); object != context.rend(); ++object) {
		auto i = (*object)->find(key);
		if (i != (*object)->end()) return i->second;
	}
	throw EvaluateError{"could not resolve identifier: " + key};
}

Value Engine::evaluate(refcount_ptr<OperatorExpression const> expr, ContextStack & context) const {
	// Binary operators.
	if (expr->lhs) {
		Value lhs = evaluate(expr->lhs, context);
		Value rhs = evaluate(expr->lhs, context);
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

Value Engine::evaluate(refcount_ptr<LiteralExpression const> expr) const {
	if (auto e = dynamic_pointer_cast<IntegerLiteralExpression const>(expr)) return Value{e->value};
	if (auto e = dynamic_pointer_cast< DoubleLiteralExpression const>(expr)) return Value{e->value};
	if (auto e = dynamic_pointer_cast< StringLiteralExpression const>(expr)) return Value{e->value};
	throw EvaluateError("unknown literal type");
}

Engine::List Engine::evaluate(refcount_ptr<ListExpression const> expr, ContextStack & context) const {
	List result;
	result.reserve(expr->elements.size());
	for (refcount_ptr<Expression const> const & elem : expr->elements) {
		result.push_back(evaluate(elem, context));
	}
	return result;
}

Engine::Object Engine::evaluate(refcount_ptr<ObjectExpression const> expr, ContextStack & context) const {
	// Evaluate keys first, so we don't lookup parts of keys in the object itself.
	List keys = evaluate(expr->keys, context);
	for (std::size_t i = 0; i < expr->keys->elements.size(); ++i) {
		if (!keys[i].is<std::string>()) throw EvaluateError("key is not a string");
	}

	Object result;
	context.push_back(&result);
	for (std::size_t i = 0; i < expr->keys->elements.size(); ++i) {
		result.insert({
			*keys[i].as_ptr<std::string>(), // already checked that it is a string above
			evaluate(expr->values->elements[i], context),
		});
	}
	context.pop_back();
	return result;

}

void imbueDefaultPrelude(Engine & engine);

}
