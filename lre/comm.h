#ifndef _COMM_
#define _COMM_

enum class type {bdd, region, cube, index};

class data_struct{
public:
	virtual operator bool() const;
	virtual void operator++();
	virtual void operator--();
	virtual void set(int);
};

data_struct *make_ds(type);

#endif
