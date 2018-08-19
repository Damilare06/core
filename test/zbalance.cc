#include <apf.h>
#include <apfMDS.h>
#include <apfMesh2.h>
#include <apfZoltan.h>
#include <gmi_null.h>
#include <gmi_mesh.h>
#include <parma.h>
#include <PCU.h>
#include <pcu_util.h>
#include <cstdlib> // exit and EXIT_FAILURE
#ifdef HAVE_SIMMETRIX
#include <gmi_sim.h>
#include <SimUtil.h>
#include <MeshSim.h>
#include <SimModel.h>
#endif


int main(int argc, char** argv)
{
  MPI_Init(&argc,&argv);
  PCU_Comm_Init();
  if ( argc != 4 ) {
    if ( !PCU_Comm_Self() )
      printf("Usage: %s <model> <mesh> <output mesh prefix>\n", argv[0]);
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }
#ifdef HAVE_SIMMETRIX
  MS_init();
  SimModel_start();
  Sim_readLicenseFile(NULL);
  gmi_sim_start();
  gmi_register_sim();
#endif
  gmi_register_null();
  gmi_register_mesh();
  //load model and mesh
  apf::Mesh2* m = apf::loadMdsMesh(argv[1],argv[2]);
  apf::MeshTag* weights = Parma_WeighByMemory(m);
  apf::Balancer* balancer = makeZoltanBalancer(m, apf::GRAPH, apf::REPARTITION);
  balancer->balance(weights, 1.10);
  delete balancer;
  apf::removeTagFromDimension(m, weights, m->getDimension());
  Parma_PrintPtnStats(m, "");
  m->destroyTag(weights);
  m->writeNative(argv[3]);
  // destroy mds
  m->destroyNative();
  apf::destroyMesh(m);
#ifdef HAVE_SIMMETRIX
  gmi_sim_stop();
  Sim_unregisterAllKeys();
  SimModel_stop();
  MS_exit();
#endif
  PCU_Comm_Free();
  MPI_Finalize();
}
