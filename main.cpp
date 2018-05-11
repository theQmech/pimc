#include <sys/resource.h>
#include <unistd.h>
#include <csignal>

#include "api.h"
#include "lre/driver.hh"

InfoMan man_t;
symtab_ symtab;
lre_driver driver;
_result global_ret_value = _result::UNDEF;
bool VERBOSE = false;
bool DEBUG = false;

void fillsymbols(){
	symtab.addsymbol(type::cube, "P");
	symtab.addsymbol(type::cube, "notP");
	symtab.addsymbol(type::region, "T");
	symtab.addsymbol(type::region, "I");

	symtab.addsymbol(type::cube, "CLS_ARR", true);
	symtab.addsymbol(type::region, "CNF_ARR", true);
}

void initsymbols(){
	cube_ P;
	InitProperty(P);
	*(symtab[symtab.get_symbol("P")].data) = P;
	*(symtab[symtab.get_symbol("notP")].data) = P;
	((cube_ *)symtab[symtab.get_symbol("notP")].data)->complement();

	region T, I;
	initTransition(T); initInitStates(I);
	*(symtab[symtab.get_symbol("T")].data) = T;
	*(symtab[symtab.get_symbol("I")].data) = I;
}

void set_limits(){
	struct rlimit old_l, new_l;
	struct rlimit *newp;
	pid_t pid = getpid();

	// 5 seconds
	new_l.rlim_cur = 5;
	new_l.rlim_max = 6;
	newp = &new_l;
	if (prlimit(pid, RLIMIT_CPU, newp, &old_l) == -1){
		perror("prlimit-1");
		exit(EXIT_FAILURE);
	}

	// 1 GB = 1<<30
	new_l.rlim_cur = 1<<30;
	new_l.rlim_max = 1<<31;
	newp = &new_l;

	if (prlimit(pid, RLIMIT_AS, newp, &old_l) == -1){
		perror("prlimit-2");
		exit(EXIT_FAILURE);
	}
}

void segv_handler(int signum){
	global_ret_value = _result::EMEM;
	// cout << "Interrupt signal (" << signum << ") received.\n";
	// exit(signum);
	handle_exit();
}

void xcpu_handler(int signum){
	global_ret_value = _result::TIMEOUT;
	// cout << "Interrupt signal (" << signum << ") received.\n";
	// exit(signum);
	handle_exit();
}

void register_handlers(){
	signal(SIGSEGV, segv_handler);
	signal(SIGXCPU, xcpu_handler);
}

int main(int argc, char *argv[]){
	set_limits();
	register_handlers();

	if (argc != 3){
		cout<<"Usage: "<<argv[0]<<" <lre_file> <aig_file>"<<endl;
		return 1;
	}

	string lre_file = argv[1];
	string aig_file = argv[2];

	Abc_Start();

	Abc_Frame_t *pAbc = Abc_FrameGetGlobalFrame();
	if (readAig(pAbc, aig_file) == 0){
		if (VERBOSE)
			cout<<"Reading AIG complete"<<endl;
	}
	else{
		if (VERBOSE)
			cout<<"Reading AIG fail"<<endl;
		return 0;
	}

	man_t.load_network(pAbc);
	if (VERBOSE)
		cout<<"Processed AIG file"<<endl;

	fillsymbols();
	initsymbols();
	if (VERBOSE)
		cout<<"Filled Symbol Table"<<endl;

	driver.verbose = VERBOSE;
	if (driver.parse(argv[1]) == 0){
		if (VERBOSE)
			std::cout<<"Parsing complete"<<std::endl;
	}
	else{
		if (VERBOSE)
			std::cerr<<"Parsing fail"<<std::endl;
		return 1;
	}

	if (VERBOSE)
		driver.root->print(0);

	if (VERBOSE)
		std::cout<<"--------------------------------------------"<<endl<<endl;

	driver.root->compute();

	handle_exit();

	return 0;
}
