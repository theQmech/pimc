#ifndef _COMM_
#define _COMM_

enum class type {bdd, region, cube, index, coll};

class data_struct{
public:
	virtual ~data_struct();
	virtual void operator=(data_struct &);
	virtual bool operator==(data_struct &);
	virtual bool operator>(data_struct &);
	virtual bool operator>=(data_struct &);
	virtual bool operator<(data_struct &);
	virtual bool operator<=(data_struct &);
	virtual void conjunct(data_struct &);
	virtual operator bool() const;
	virtual void operator++();
	virtual void operator--();
	virtual void set(int);
};

data_struct *make_ds(type, bool, int);

#endif
