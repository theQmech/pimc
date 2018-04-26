#include "api.h"

void handle_exit(){
    if (global_ret_value == _result::SAT)
        std::cout<<"Result: SAT"<<std::endl;
    else if (global_ret_value == _result::UNSAT)
        std::cout<<"Result: UNS"<<std::endl;
    else if (global_ret_value == _result::UNDEF)
        std::cout<<"Result: UNDEF"<<std::endl;

    Abc_Stop();

    exit(0);
}

void assignWithInit(region &U){
    region tmp(man_t.cInit, man_t.Network_Cnf()->nVars);
    U = tmp;
}

//! For each primed(nonprimed) literal, flip to its nonprimed(primed) version.
//! @param[in]      U       cube_ whose variable are to be flipped. Each literal
///                         must have a valid counterpart.
void toPrime(cube_ &U){
    U.toPrime(man_t.toPrime);
}

//! @return A single literal cube_. To set the safety property(P) to true,
//! set this literal to true. To set P' to false, set negation of this literal
//! to true.
cube_ getProperty(){
    vector<lit> tmp(1, toLitCond(man_t.getP(), 0));
    cube_ ret(tmp);
    return ret;
}

void InitProperty(cube_ &U){
    U.nLits = 1;
    U.vLits.clear();
    U.vLits.push_back(toLitCond(man_t.getP(), 1));
}

//! @return Region which represents the transition function in CNF form.
region getT(){
    return region(man_t.Network_Cnf());
    assert(false);
}

void initTransition(region &U){
    U = man_t.Network_Cnf();
}

//! @return Initial states in the form of a region(CNF).
region getInit(){
    return region(man_t.cInit, man_t.Network_Cnf()->nVars);
}

void initInitStates(region &U){
    U.initialize(man_t.cInit, man_t.Network_Cnf()->nVars);
}

/*! Empty constructor.
 */
InfoMan::InfoMan(){
}

/*! Empty constructor.
 */
InfoMan::~InfoMan(){
    if(pCnf)
        Cnf_DataFree(pCnf);
}

/*!
 *  @return Pointer to AIG network.
 */
Aig_Man_t *InfoMan::AigNtk(){
    return pAig;
}

/*!
 *  @return Pointer to CNF holding the network.
 */
Cnf_Dat_t *InfoMan::Network_Cnf(){
    return pCnf;
}

/*! Extracts information from the network and stores in member variables. Call
 *  this function to register the problem instance.
 *  @param[in]  pAbc        Pointer to ABC Frame holding the network
 */
void InfoMan::load_network(Abc_Frame_t *pAbc){
    cout<<"Loading network"<<endl;

    pNtk = Abc_FrameReadNtk(pAbc);
    pAig = Abc_NtkToDar(pNtk, 0, 1);
    pCnf = aig2cnf(pAig);

    assert(Abc_NtkLatchNum(pNtk) == Aig_ManRegNum(pAig));

    cInit.resize(Aig_ManRegNum(pAig), vector<int>(1, -1));
    int i=0, j=0;
    Abc_Obj_t *pLatch;
    cout<<"Num regs:\t"<<pAig->nRegs<<endl;
    Abc_NtkForEachLatch(pNtk, pLatch, i){
        cInit[j][0] = toLitCond(
                        pCnf->pVarNums[Aig_ObjId(Saig_ManLo(pAig, j))],
                        !Abc_LatchIsInit1(pLatch));
        ++j;
    }

    // Init the map toPrime
    Aig_Obj_t *pIn, *pOut;
    int k;
    Aig_ManForEachLiLoSeq(pAig, pOut, pIn, k){
        lit in = pCnf->pVarNums[Aig_ObjId(pIn)], out = pCnf->pVarNums[Aig_ObjId(pOut)];
        toPrime[in] = out;
        toPrime[out] = in;
    }
}

/*! Get variable number of the safety property of the network.
 */
lit InfoMan::getP(){
    return pCnf->pVarNums[Aig_ObjId(Aig_ManCo(pAig, 0))];
}
