/*
This class is meant to compute the neighbor list for two sets of atoms and dump the positions of such atoms only.
It is written starting from Coordination, CoordinationBase, DumpAtoms classes.
*/

#include "colvar/Colvar.h"
#include "tools/NeighborList.h"
#include "tools/Communicator.h"
#include "tools/OpenMP.h"
#include "colvar/ActionRegister.h"
#include "tools/File.h"

#include <memory>


namespace PLMD {

class NeighborList;

namespace colvar {

class TestFunction : public Colvar {
  OFile of;
  double lenunit;
  std::string fmt_xyz;
  bool pbc;
  bool serial;
  std::unique_ptr<NeighborList> nl;
  bool invalidateList;
  bool firsttime;
  // std::vector<AtomNumber> ga_lista,gb_lista; // TODO in case

public:
  explicit TestFunction(const ActionOptions&);
  ~TestFunction();
// active methods:
  void calculate() override;
  void prepare() override;
  //virtual double pairing(double distance,double&dfunc,unsigned i,unsigned j)const=0;
  static void registerKeywords( Keywords& keys );
};

PLUMED_REGISTER_ACTION(TestFunction,"TESTFUNCTION")


void TestFunction::registerKeywords( Keywords& keys ) {
  Colvar::registerKeywords(keys);
  keys.addFlag("SERIAL",false,"Perform the calculation in serial - for debug purpose");
  keys.addFlag("PAIR",false,"Pair only 1st element of the 1st group with 1st element in the second, etc");
  keys.addFlag("NLIST",false,"Use a neighbor list to speed up the calculation");
  keys.add("optional","NL_CUTOFF","The cutoff for the neighbor list");
  keys.add("optional","NL_STRIDE","The frequency with which we are updating the atoms in the neighbor list");
  keys.add("atoms","GROUPA","First list of atoms");
  keys.add("atoms","GROUPB","Second list of atoms (if empty, N*(N-1)/2 pairs in GROUPA are counted)");
}

TestFunction::TestFunction(const ActionOptions&ao):
  PLUMED_COLVAR_INIT(ao),
  pbc(true),
  serial(false),
  invalidateList(true),
  firsttime(true)
{

  parseFlag("SERIAL",serial);

  std::vector<AtomNumber> ga_lista,gb_lista;
  parseAtomList("GROUPA",ga_lista);
  parseAtomList("GROUPB",gb_lista);

  bool nopbc=!pbc;
  parseFlag("NOPBC",nopbc);
  pbc=!nopbc;

// pair stuff
  bool dopair=false;
  parseFlag("PAIR",dopair);

// neighbor list stuff
  bool doneigh=false;
  double nl_cut=0.0;
  int nl_st=0;
  parseFlag("NLIST",doneigh);
  if(doneigh) {
    parse("NL_CUTOFF",nl_cut);
    if(nl_cut<=0.0) error("NL_CUTOFF should be explicitly specified and positive");
    parse("NL_STRIDE",nl_st);
    if(nl_st<=0) error("NL_STRIDE should be explicitly specified and positive");
  
  // positions output file and format
  std::string file="nl_positions.xyz";
  of.link(*this);
  of.open(file);
  lenunit=1.0;
  fmt_xyz="%f";



  }

  addValueWithDerivatives(); setNotPeriodic();
  if(gb_lista.size()>0) {
    if(doneigh)  nl=Tools::make_unique<NeighborList>(ga_lista,gb_lista,serial,dopair,pbc,getPbc(),comm,nl_cut,nl_st);
    else         nl=Tools::make_unique<NeighborList>(ga_lista,gb_lista,serial,dopair,pbc,getPbc(),comm);
  } else {
    if(doneigh)  nl=Tools::make_unique<NeighborList>(ga_lista,serial,pbc,getPbc(),comm,nl_cut,nl_st);
    else         nl=Tools::make_unique<NeighborList>(ga_lista,serial,pbc,getPbc(),comm);
  }

  requestAtoms(nl->getFullAtomList());

  log.printf("  between two groups of %u and %u atoms\n",static_cast<unsigned>(ga_lista.size()),static_cast<unsigned>(gb_lista.size()));
  log.printf("  first group:\n");
  for(unsigned int i=0; i<ga_lista.size(); ++i) {
    if ( (i+1) % 25 == 0 ) log.printf("  \n");
    log.printf("  %d", ga_lista[i].serial());
  }
  log.printf("  \n  second group:\n");
  for(unsigned int i=0; i<gb_lista.size(); ++i) {
    if ( (i+1) % 25 == 0 ) log.printf("  \n");
    log.printf("  %d", gb_lista[i].serial());
  }
  log.printf("  \n");
  if(pbc) log.printf("  using periodic boundary conditions\n");
  else    log.printf("  without periodic boundary conditions\n");
  if(dopair) log.printf("  with PAIR option\n");
  if(doneigh) {
    log.printf("  using neighbor lists with\n");
    log.printf("  update every %d steps and cutoff %f\n",nl_st,nl_cut);
  }
}

TestFunction::~TestFunction() {
// destructor required to delete forward declared class
}

void TestFunction::prepare() {
  if(nl->getStride()>0) {
    if(firsttime || (getStep()%nl->getStride()==0)) {
      requestAtoms(nl->getFullAtomList());
      invalidateList=true;
      firsttime=false;
    } else {
      requestAtoms(nl->getReducedAtomList());
      invalidateList=false;
      if(getExchangeStep()) error("Neighbor lists should be updated on exchange steps - choose a NL_STRIDE which divides the exchange stride!");
    }
    if(getExchangeStep()) firsttime=true;
  }
}

// calculator
void TestFunction::calculate()
{

  int atoms_in_nl=0;
  std::vector< int > atoms_list;

    ////// TODO
    // add group a atoms by default
      // for(unsigned int i=0; i<ga_lista.size(); i+=1) {
      //   atoms_list.push_back(ga_lista[i].index());
      // }
    ////// TODO


  if(nl->getStride()>0 && invalidateList) {
    nl->update(getPositions());
  }

 unsigned stride;
  unsigned rank;
  if(serial) {
    stride=1;
    rank=0;
  } else {
    stride=comm.Get_size();
    rank=comm.Get_rank();
  }

  unsigned nt=OpenMP::getNumThreads();
  const unsigned nn=nl->size();
  atoms_in_nl = nn;

  if(nt*stride*10>nn) nt=1;

  // this goes in parallel
  #pragma omp parallel num_threads(nt)
  {
    std::vector< int > omp_atoms_list;
    
    ////// TODO
    // add group a atoms by default
      // for(unsigned int i=0; i<ga_lista.size(); i+=1) {
      //   omp_atoms_list.push_back(ga_lista[i].index());
      // }
    ////// TODO

    #pragma omp for nowait
    for(unsigned int i=rank; i<nn; i+=stride) {

      // Vector distance;
      unsigned i0=nl->getClosePair(i).first;
      unsigned i1=nl->getClosePair(i).second;

      if (!( std::find(atoms_list.begin(), atoms_list.end(), i0) != atoms_list.end() ))
        omp_atoms_list.push_back(i0);

      if (!( std::find(atoms_list.begin(), atoms_list.end(), i1) != atoms_list.end() ))
        omp_atoms_list.push_back(i1);

  // this is forced to go on a single thread
  #pragma omp critical
      if(nt>1) {
        for(unsigned i=0; i<omp_atoms_list.size(); i++){
          auto aux = omp_atoms_list[i]; 
          if (!( std::find(atoms_list.begin(), atoms_list.end(), aux) != atoms_list.end() ))
            atoms_list.push_back(aux);
      }
      }
      else {
        atoms_list = omp_atoms_list;
      }
    }
  }

  // return length of neighbor list
  // setValue(atoms_in_nl);
  
  // return the number of atoms in the neighborhood
  setValue(atoms_list.size());
  
    // // debug 
  // for (int i: atoms_list)
  //   std::cout << i << ' '<< std::endl;
  // std::cout << ' '<< std::endl;

  // print positions file
  // of.printf("%d\n",getNumberOfAtoms()); // debug --> check number of atoms
  
  of.printf("Number of neighbors: %d\n",atoms_list.size());

  // cell informations
    const Tensor & t(getPbc().getBox());
    if(getPbc().isOrthorombic()) {
      of.printf((" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+"\n").c_str(),lenunit*t(0,0),lenunit*t(1,1),lenunit*t(2,2));
    } else {
      of.printf((" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+"\n").c_str(),
                lenunit*t(0,0),lenunit*t(0,1),lenunit*t(0,2),
                lenunit*t(1,0),lenunit*t(1,1),lenunit*t(1,2),
                lenunit*t(2,0),lenunit*t(2,1),lenunit*t(2,2)
               );
    }
    for(unsigned j=0; j<atoms_list.size(); ++j) {
      int i =  atoms_list[j];
      const char* defname="X";
      const char* name=defname;
      int item = getAbsoluteIndex(i).index();
      of.printf(("%s "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+"%d"+"\n").c_str(),name,lenunit*getPosition(i)(0),lenunit*getPosition(i)(1),lenunit*getPosition(i)(2),item);
    }
    // for(unsigned j=0; j<100 - atoms_list.size(); ++j) {
    //   const char* defname="X";
    //   const char* name=defname;
    //   of.printf(("%s "+fmt_xyz+" "+fmt_xyz+" "+fmt_xyz+" "+"%d"+"\n").c_str(),name,0.00000,0.00000,0.00000,0);
    // }
    

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // If i can pass this to a dump atoms we are good to go :D 


}
}
}
