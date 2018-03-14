#include "api.h"
#include "lre/driver.hh"

InfoMan man_t;
symtab_ symtab;
lre_driver driver;
_result global_ret_value = _result::UNDEF;

void fillsymbols(){
	symtab.addsymbol(type::cube, "P");
	symtab.addsymbol(type::region, "T");
	symtab.addsymbol(type::region, "I");

	symtab.addsymbol(type::cube, "CLS_ARR", true);
	symtab.addsymbol(type::region, "CNF_ARR", true);
}

void initsymbols(){
	// TODO: Copy vals
	*(symtab[symtab.get_symbol("P")].data) = getProperty();
	*(symtab[symtab.get_symbol("T")].data) = getT();
	*(symtab[symtab.get_symbol("I")].data) = getInit();
}

int main(int argc, char *argv[]){
	if (argc != 3){
		cout<<"Usage: "<<argv[0]<<" <lre_file> <aig_file>"<<endl;
		return 1;
	}

	string lre_file = argv[1];
	string aig_file = argv[2];

	Abc_Start();

	Abc_Frame_t *pAbc = Abc_FrameGetGlobalFrame();
	if (readAig(pAbc, aig_file) == 0){
		cerr<<"Reading AIG complete"<<endl;
	}
	else{
		cerr<<"Reading AIG fail"<<endl;
		return 0;
	}

	man_t.load_network(pAbc);
	cout<<"Processed AIG file"<<endl;

	fillsymbols();
	initsymbols();

	if (driver.parse(argv[1]) == 0)
		std::cout<<"Parsing complete"<<std::endl;
	else{
		std::cerr<<"Parsing fail"<<std::endl;
		return 1;
	}

	driver.root->compute();

	handle_exit();

	return 0;
}