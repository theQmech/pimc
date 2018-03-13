#ifndef _SYMTAB_
#define _SYMTAB_

#include <vector>
#include <cassert>
#include <iostream>
#include "symbols.h"
#include "comm.h"

class entry{
public:
	std::string name;
	type entry_type;
	data_struct *data;
	entry(std::string, type);
	void clean();
};

class symtab_{
	std::vector<entry> entries;
	void clean();
public:
	bool name_exists(std::string name);
	int get_symbol(std::string name);
	void addsymbol(type, std::string);
	entry &operator[](int i);
};

extern symtab_ symtab;
void initsymbols();

#endif
