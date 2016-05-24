#pragma once

#include <memory>

#include <string_view.hpp>

namespace conftaal {

class expression {
public:
	virtual ~expression() {}
};

class identifier_expression final : public expression {
public:
	explicit identifier_expression(string_view identifier) : identifier(identifier) {}

	string_view identifier;
};

class operator_expression final : public expression {
	// Represents both binary and unary operator expressions.
public:
	operator_expression(
		string_view op,
		std::unique_ptr<expression> lhs,
		std::unique_ptr<expression> rhs
	) : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

	string_view op;
	std::unique_ptr<expression> lhs;
	std::unique_ptr<expression> rhs;

	bool parenthesized = false;

	bool is_unary() const { return lhs == nullptr; }
};

class literal_expression : public expression {
public:
	literal_expression() {}
};

class string_literal_expression final : public literal_expression {
public:
	explicit string_literal_expression(string_view value)
		: value(value) {}

	string_view value;
};

class object_literal_expression final : public literal_expression {
public:
	explicit object_literal_expression(
		std::vector<std::pair<string_view, std::unique_ptr<expression>>> values
	) : values(std::move(values)) {}

	std::vector<std::pair<string_view, std::unique_ptr<expression>>> values;
};

class list_literal_expression final : public literal_expression {
public:
	explicit list_literal_expression(
		std::vector<std::unique_ptr<expression>> values
	) : values(std::move(values)) {}

	std::vector<std::unique_ptr<expression>> values;
};

}
