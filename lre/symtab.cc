#include "symtab.h"

entry::entry(std::string name_, type ty_):name(name_),entry_type(ty_){
	data = make_ds(ty_);
}

void entry::clean(){
	if (data)
		delete(data);
}

entry &symtab_::operator[](int i){
	return entries.at(i);
}

void symtab_::clean(){
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

void symtab_::addsymbol(type ty_, std::string name){
	if (name_exists(name)){
		std::cerr<<name<<" symbol already exists"<<std::endl;
		assert(false);
	}
	entries.push_back(entry(name, ty_));
}


void initsymbols(){
	symtab.addsymbol(type::cube, "P");
	symtab.addsymbol(type::region, "T");
	symtab.addsymbol(type::region, "I");

	// TODO: Copy vals
}
