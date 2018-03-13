#include "api.h"
#include "lre/driver.hh"

InfoMan man_t;
symtab_ symtab;
lre_driver driver;
_result global_ret_value = _result::UNDEF;

int main(int argc, char *argv[]){
	if (argc != 3){
		cout<<"Usage: "<<argv[0]<<" <lre_file> <aig_file>"<<endl;
		return 1;
	}

	string lre_file = argv[1];
	string aig_file = argv[2];

	if (driver.parse(argv[1]) == 0)
		std::cout<<"Parsing complete"<<std::endl;
	else{
		std::cerr<<"Parsing fail"<<std::endl;
		return 1;
	}

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

	driver.root->compute();

	handle_exit();

	return 0;
}
