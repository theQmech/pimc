#include "lre/comm.h"
#include "utils.h"

data_struct::~data_struct(){
}

void data_struct::operator=(data_struct &){
	logAndStop("Not implemented");
}

bool data_struct::operator==(data_struct &){
	logAndStop("Not implemented");
}

bool data_struct::operator>(data_struct &){
	logAndStop("Not implemented");
}

bool data_struct::operator>=(data_struct &){
	logAndStop("Not implemented");
}

bool data_struct::operator<(data_struct &){
	logAndStop("Not implemented");
}

bool data_struct::operator<=(data_struct &){
	logAndStop("Not implemented");
}

void data_struct::conjunct(data_struct &){
	logAndStop("Not implemented");
}

data_struct::operator bool() const{
	logAndStop("Not implemented");
}

void data_struct::operator++(){
	logAndStop("Not implemented");
}

void data_struct::operator--(){
	logAndStop("Not implemented");
}

void data_struct::set(int){
	logAndStop("Not implemented");
}

data_struct *make_ds(type ty_, bool isArray, int sz){
	data_struct *ret;

	switch(ty_){
		case type::bdd:
			//TODO
			logAndStop("Not implemented");
			break;

		case type::region:
			if (isArray)
				ret = new region[sz]();
			else
				ret = new region();
			break;

		case type::cube:
			if (isArray)
				ret = new cube_[sz]();
			else
				ret = new cube_();
			break;

		case type::index:
			if (isArray)
				logAndStop("Not implemented");
			else
				ret = new index_();
			break;

		case type::coll:
			if (isArray)
				logAndStop("Not implemented");
			else
				ret = new coll_();
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}

	return ret;
}
