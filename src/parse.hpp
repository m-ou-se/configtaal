#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include <mstd/optional.hpp>
#include <mstd/string_view.hpp>
#include <string_tracker.hpp>

#include "error.hpp"
#include "expression.hpp"

namespace conftaal {

class ParseError : public Error {
public:
	using Error::Error;
};

class Matcher;

extern Matcher match_end_of_file;

class Parser {

public:
	explicit Parser(string_tracker & tracker, mstd::string_view source)
		: string_tracker_(tracker), source_(source) {}

	std::unique_ptr<Expression> parse_expression(Matcher const & end = match_end_of_file);
	std::unique_ptr<ListExpression> parse_list(Matcher const & end = match_end_of_file);
	std::unique_ptr<ObjectExpression> parse_object(Matcher const & end = match_end_of_file);

	static bool is_identifier_start(char c);
	static mstd::string_view parse_identifier(mstd::string_view & source);
	static std::unique_ptr<IdentifierExpression> parse_identifier_expression(mstd::string_view & source);

private:
	std::unique_ptr<Expression> parse_expression_atom(Matcher const & end);
	bool parse_more_expression(std::unique_ptr<Expression> & expr, Matcher const & end);

	std::unique_ptr<StringLiteralExpression> parse_string_literal();
	std::unique_ptr<Expression> parse_number();

	mstd::optional<mstd::string_view> parse_end(Matcher const &, bool consume = true);

private:
	string_tracker & string_tracker_;
	mstd::string_view source_;

};

}
