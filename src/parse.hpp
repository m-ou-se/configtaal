#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include <optional.hpp>
#include <string_view.hpp>
#include <string_tracker.hpp>

#include "expression.hpp"

namespace conftaal {

class ParseError : public std::runtime_error {

private:
	string_view where_;

	// ('where', message) pairs
	std::vector<std::pair<std::string, string_view>> notes_;

public:
	ParseError(
		std::string what,
		string_view where,
		std::vector<std::pair<std::string, string_view>> notes = {}
	) : std::runtime_error(std::move(what)), where_(where), notes_(std::move(notes)) {}

	string_view where() const { return where_; }

	std::vector<std::pair<std::string, string_view>> const & notes() const { return notes_; }

};

class Matcher;

extern Matcher match_end_of_file;

class Parser {

public:
	explicit Parser(string_tracker & tracker, string_view source)
		: string_tracker_(tracker), source_(source) {}

	std::unique_ptr<Expression> parse_expression(Matcher const & end = match_end_of_file);
	std::unique_ptr<ListExpression> parse_list(Matcher const & end = match_end_of_file);
	std::unique_ptr<ObjectExpression> parse_object(Matcher const & end = match_end_of_file);

	static bool is_identifier_start(char c);
	static string_view parse_identifier(string_view & source);
	static std::unique_ptr<IdentifierExpression> parse_identifier_expression(string_view & source);

private:
	std::unique_ptr<Expression> parse_expression_atom(Matcher const & end);
	bool parse_more_expression(std::unique_ptr<Expression> & expr, Matcher const & end);

	std::unique_ptr<StringLiteralExpression> parse_string_literal();
	std::unique_ptr<Expression> parse_number();

	optional<string_view> parse_end(Matcher const &, bool consume = true);

private:
	string_tracker & string_tracker_;
	string_view source_;

};

}
