#define CATCH_CONFIG_MAIN
#include "../../src/parse.hpp"
#include "../../src/evaluate.hpp"

#include <catch.hpp>

namespace Catch {
	template<typename T>
	struct StringMaker<std::optional<T>> {
		static std::string convert(std::optional<T> const & value) {
			if (!value) return "None";
			return "Some(" + Catch::toString(*value) + ")";
		}
	};
}

string_pool::string_tracker tracker;
conftaal::Engine engine;

std::unique_ptr<conftaal::Expression> parse(std::string_view source) {
	conftaal::Parser parser{tracker, source};
	return parser.parse_expression();
}

conftaal::Value evaluate(std::unique_ptr<conftaal::Expression> && expression) {
	return engine.evaluate(std::move(expression));
}

conftaal::Value evaluate(std::string_view source) {
	conftaal::Parser parser{tracker, source};
	return engine.evaluate(parser.parse_expression());
}

TEST_CASE("Evaluate simple integer expressions", "[evaluate]") {
	REQUIRE(evaluate("1 + 1").as<std::int64_t>() == 2);
	REQUIRE(evaluate("4 - 1").as<std::int64_t>() == 3);
	REQUIRE(evaluate("2 * 3").as<std::int64_t>() == 6);
	REQUIRE(evaluate("10 / 3").as<std::int64_t>() == 3);
	REQUIRE(evaluate("10 % 3").as<std::int64_t>() == 1);
}
