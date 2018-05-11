#ifndef API_H
#define API_H

#include "utils.h"

#define STO_SZ 400

enum class _result {SAT, UNSAT, TIMEOUT, EMEM, UNDEF};
extern _result global_ret_value;

void handle_exit();

/*! Store for all information about the circuit/network, specific to the
 *  problem instance.
 */
class InfoMan{
    Abc_Ntk_t *pNtk;
    Aig_Man_t *pAig;
    Cnf_Dat_t *pCnf;

public:
    Aig_Man_t *AigNtk();
    Cnf_Dat_t *Network_Cnf();
    /// For each latch, maps input variable to output variable and vice versa.
    unordered_map<int, int> toPrime;
    /// Stores initial value of each register. For now, assume single state.
    vector<vector<lit>> cInit;

    InfoMan();
    ~InfoMan();
    void load_network(Abc_Frame_t *pAbc);
    int getP();
};

extern InfoMan man_t;

void toPrime(cube_ &U);
void toPrime(region &U);
void InitProperty(cube_ &U);
void initTransition(region &U);
void initInitStates(region &u);

vector<lit> implicate(cube_ &, region &);
vector<lit> rec_implicate(vector<lit>, vector<lit>, sat_solver *);
vector<lit> min_indc(cube_ &, region &, region &);
vector<lit> lic(region &, region &, region &, vector<lit>);

vector<lit> mic(region &, region &, region &, vector<lit>);
bool is_inductive(region &, region &, region &, vector<lit>);

#endif
