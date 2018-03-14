#ifndef _SYMTAB_
#define _SYMTAB_

#include <vector>
#include <cassert>
#include <iostream>
#include "symbols.h"
#include "comm.h"

const int ARR_SZ = 100;

// `data` holds pointer to data structure of type `entry_type`
// if `isArray` is true, `data` points to arrray of size `ARR_SZ`
class entry{
public:
	std::string name;
	type entry_type;
	bool isArray;
	data_struct *data;
	entry(std::string, type, bool = false);
	void clean();
};

class symtab_{
	std::vector<entry> entries;
public:
	~symtab_();
	bool name_exists(std::string name);
	int get_symbol(std::string name);
	void addsymbol(type, std::string, bool = false);
	entry &operator[](int i);
};
extern symtab_ symtab;

#endif
