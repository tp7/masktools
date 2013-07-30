#ifndef __Mt_Parser_H__
#define __Mt_Parser_H__

#include "../utils/utils.h"
#include "symbol.h"
#include <deque>

namespace Filtering { namespace Parser {

class Parser {
   String parsed_string;
   std::deque<Symbol> elements;
   std::deque<Symbol> symbols;

public:
   Parser();
   Parser(const String &parsed_string, const String &separators);

public:
   Parser &addSymbol(const Symbol &symbol);
private:
   const Symbol *findSymbol(const String &value) const;
   Symbol stringToSymbol(const String &value) const;
public:
   Parser &parse(const String &parsed_string, const String &separators);

   String getParsedString() const;
   int count() const;

   std::deque<Symbol> &getExpression() { return elements; }

};
Parser getDefaultParser();

} } // namespace Parser, Filtering

#endif
