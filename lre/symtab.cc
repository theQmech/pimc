#include "symtab.h"

entry::entry(std::string name_, type ty_, bool isArray_)
:name(name_),entry_type(ty_), isArray(isArray_){
	data = make_ds(ty_, isArray_, ARR_SZ);
}

void entry::clean(){
	if (data)
		if (!isArray)
			delete data;
}

entry &symtab_::operator[](int i){
	// .at(int) catches out of bounds array access
	return entries.at(i);
}

symtab_::~symtab_(){
	// std::cout<<"Clearing symbol table:\t["<<this<<"]"<<std::endl;
	for(auto en : entries)
		en.clean();
}

bool symtab_::name_exists(std::string name){
	return (get_symbol(name)>=0);
}

int symtab_::get_symbol(std::string name){
	for(int i=0; i<entries.size(); ++i)
		if (entries[i].name == name)
			return i;
	return -1;
}

void symtab_::addsymbol(type ty_, std::string name, bool isArray){
	if (name_exists(name)){
		std::cerr<<name<<" symbol already exists"<<std::endl;
		assert(false);
	}
	entries.push_back(entry(name, ty_, isArray));
}
