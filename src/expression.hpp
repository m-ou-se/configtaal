#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include <mstd/string_view.hpp>
#include <mstd/refcount.hpp>

#include "operator.hpp"

namespace conftaal {

using mstd::string_view;
using mstd::refcounted;
using mstd::refcount_ptr;
using mstd::take_or_copy;
using mstd::static_pointer_cast;
using mstd::dynamic_pointer_cast;

class Expression : public refcounted {
public:
	virtual ~Expression() {}
};

class IdentifierExpression final : public Expression {
public:
	explicit IdentifierExpression(string_view identifier)
		: identifier(identifier) {}

	string_view identifier;
};

class OperatorExpression final : public Expression {
	// Represents both binary and unary operator expressions.
public:
	OperatorExpression(
		Operator op,
		string_view op_source,
		refcount_ptr<Expression const> lhs,
		refcount_ptr<Expression const> rhs
	) : op(op), op_source(op_source), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

	Operator op;
	string_view op_source;
	refcount_ptr<Expression const> lhs;
	refcount_ptr<Expression const> rhs;

	bool parenthesized = false;

	bool is_unary() const { return lhs == nullptr; }
};

class LiteralExpression : public Expression {
public:
	LiteralExpression() {}
};

class IntegerLiteralExpression final : public LiteralExpression {
public:
	explicit IntegerLiteralExpression(std::int64_t value) : value(value) {}

	std::int64_t value;
};

class DoubleLiteralExpression final : public LiteralExpression {
public:
	explicit DoubleLiteralExpression(double value) : value(value) {}

	double value;
};

class StringLiteralExpression final : public LiteralExpression {
public:
	explicit StringLiteralExpression(string_view value)
		: value(value) {}

	string_view value;
};

class ListExpression final : public Expression {
public:
	explicit ListExpression(
		std::vector<refcount_ptr<Expression const>> elements
	) : elements(std::move(elements)) {}

	std::vector<refcount_ptr<Expression const>> elements;
};

class ObjectExpression final : public Expression {
public:
	ObjectExpression(
		refcount_ptr<ListExpression const> keys,
		refcount_ptr<ListExpression const> values
	) : keys(std::move(keys)), values(std::move(values)) {
		assert(this->keys->elements.size() == this->values->elements.size());
	}

	refcount_ptr<ListExpression const> keys;
	refcount_ptr<ListExpression const> values;
};

}
