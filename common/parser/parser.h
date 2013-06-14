#ifndef __Mt_Parser_H__
#define __Mt_Parser_H__

#include "../utils/utils.h"
#include "symbol.h"

namespace Filtering { namespace Parser {

class Parser {
   String parsed_string;
   std::list<Symbol> elements;
   std::list<Symbol> symbols;

public:
   Parser();
   Parser(const String &parsed_string, const String &separators);

public:
   Parser &addSymbol(const Symbol &symbol);
private:
   Symbol findSymbol(const String &value) const;
   bool isSymbol(const String &value) const { return findSymbol(value).type != Symbol::UNDEFINED; }
   Symbol stringToSymbol(const String &value) const;
public:
   Parser &parse(const String &parsed_string, const String &separators);

   String getParsedString() const;
   int count() const;

   std::list<Symbol> &getExpression() { return elements; }

};
Parser getDefaultParser();

} } // namespace Parser, Filtering

#endif
