#include "lre/comm.h"
#include "utils.h"

data_struct::operator bool() const{
	std::cerr<<"Not implemented"<<std::endl;
	assert(false);
}

void data_struct::operator++(){
	std::cerr<<"Not implemented"<<std::endl;
	assert(false);
}

void data_struct::operator--(){
	std::cerr<<"Not implemented"<<std::endl;
	assert(false);
}

void data_struct::set(int){
	std::cerr<<"Not implemented"<<std::endl;
	assert(false);
}

data_struct *make_ds(type ty_){
	data_struct *ret;
	switch(ty_){
		case type::bdd:
			//TODO
			assert(false);
			break;

		case type::region:
			ret = new region();
			break;

		case type::cube:
			ret = new cube_();
			break;

		case type::index:
			ret = new index_();
			break;
	}
}
