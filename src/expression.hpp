#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <string_view.hpp>

#include "operator.hpp"

namespace conftaal {

class Expression {
public:
	virtual ~Expression() {}
};

class IdentifierExpression final : public Expression {
public:
	explicit IdentifierExpression(string_view identifier) : identifier(identifier) {}

	string_view identifier;
};

class OperatorExpression final : public Expression {
	// Represents both binary and unary operator expressions.
public:
	OperatorExpression(
		Operator op,
		string_view op_source,
		std::unique_ptr<Expression> lhs,
		std::unique_ptr<Expression> rhs
	) : op(op), op_source(op_source), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

	Operator op;
	string_view op_source;
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;

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

class ObjectExpression final : public Expression {
public:
	explicit ObjectExpression(
		std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> elements
	) : elements(std::move(elements)) {}

	std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> elements;
};

class ListExpression final : public Expression {
public:
	explicit ListExpression(
		std::vector<std::unique_ptr<Expression>> elements
	) : elements(std::move(elements)) {}

	std::vector<std::unique_ptr<Expression>> elements;
};

}
