/*
 * This macro is run in our daily CI and is intended as a minimum working
 * example showing how to unpack the raw hits into the offline tracker hit
 * format. No other reconstruction or analysis is performed
 */
#include <GlobalVariables.C>
#include <Trkr_Clustering.C>
#include <Trkr_RecoInit.C>
#include <Trkr_Reco.C>
#include <Trkr_TpcReadoutInit.C>
#include <QA.C>
#include <G4_KFParticle.C>

#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>

#include <trackingqa/TrackFittingQA.h>
#include <trackingqa/TpcSiliconQA.h>
#include <trackingqa/VertexQA.h>
#include <kfparticleqa/QAKFParticle.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wundefined-internal"

#include <kfparticle_sphenix/KFParticle_sPHENIX.h>

#pragma GCC diagnostic pop

#include <phool/recoConsts.h>

#include <stdio.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libmicromegas.so)
R__LOAD_LIBRARY(libtrack_reco.so)
R__LOAD_LIBRARY(libtrackingqa.so)
R__LOAD_LIBRARY(libkfparticle_sphenix.so)

void reconstruct_pipi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  std::string pipi_reconstruction_name = "pipi_reco";
  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(pipi_reconstruction_name);

  kfparticle->setDecayDescriptor("K_S0 -> pi^+ pi^-");
  kfparticle->saveOutput(false);

  kfparticle->usePID();
  kfparticle->setPIDacceptFraction(0.4);
  kfparticle->dontUseGlobalVertex();
  kfparticle->requireTrackVertexBunchCrossingMatch();
  kfparticle->allowZeroMassTracks();
  kfparticle->saveDST();
  kfparticle->setContainerName(pipi_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex();
  kfparticle->setMotherIPchi2(100);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(0.88);
  kfparticle->setDecayLengthRange(0.1, FLT_MAX);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.0);
  kfparticle->setMinTPChits(20);
  kfparticle->setMinMVTXhits(1);
  kfparticle->setMinINTThits(0);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(20);
  kfparticle->setMaximumDaughterDCA(0.5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(0.40);
  kfparticle->setMaximumMass(0.60);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  se->registerSubsystem(kfparticle);

  QAKFParticle *kfpqa = new QAKFParticle("QAKFParticle_K_S0","K_S0",0.4,0.6);
  kfpqa->setKFParticleNodeName(pipi_reconstruction_name);
  se->registerSubsystem(kfpqa);
}

void reconstruct_KK_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  std::string KK_reconstruction_name = "KK_reco"; 
  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(KK_reconstruction_name);

  kfparticle->setDecayDescriptor("phi -> K^+ K^-");
  kfparticle->saveOutput(false);

  kfparticle->usePID();
  kfparticle->setPIDacceptFraction(0.4);
  kfparticle->dontUseGlobalVertex();
  kfparticle->requireTrackVertexBunchCrossingMatch();
  kfparticle->allowZeroMassTracks();
  kfparticle->saveDST();
  kfparticle->setContainerName(KK_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex();
  kfparticle->setMotherIPchi2(100);
  kfparticle->setFlightDistancechi2(-1.);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.1);
  kfparticle->setMaximumTrackPT(0.7);
  kfparticle->setMaximumTrackchi2nDOF(100.);
  kfparticle->setMinTPChits(25);
  kfparticle->setMinMVTXhits(1);
  kfparticle->setMinINTThits(0);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(20);
  kfparticle->setMaximumDaughterDCA(0.05);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(0.98);
  kfparticle->setMaximumMass(1.1);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  se->registerSubsystem(kfparticle);

  QAKFParticle *kfpqa = new QAKFParticle("QAKFParticle_phi","phi",0.98,1.1);
  kfpqa->setKFParticleNodeName(KK_reconstruction_name);
  se->registerSubsystem(kfpqa);
}

void reconstruct_ppi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  std::string ppi_reconstruction_name = "ppi_reco";
  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(ppi_reconstruction_name);

  kfparticle->setDecayDescriptor("[Lambda0 -> proton^+ pi^-]cc");
  kfparticle->saveOutput(false);

  kfparticle->usePID();
  kfparticle->setPIDacceptFraction(0.4);
  kfparticle->dontUseGlobalVertex();
  kfparticle->requireTrackVertexBunchCrossingMatch();
  kfparticle->allowZeroMassTracks();
  kfparticle->saveDST();
  kfparticle->setContainerName(ppi_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(100);
  kfparticle->setFlightDistancechi2(-1.);
  kfparticle->setMinDIRA(0.88);
  kfparticle->setDecayLengthRange(0.1, FLT_MAX);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.1);
  kfparticle->setMinimumTrackIP_XY(0.05);
  kfparticle->setMinTPChits(25);
  kfparticle->setMinMVTXhits(1);
  kfparticle->setMinINTThits(0);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(20);
  kfparticle->setMaximumDaughterDCA(0.5);
  kfparticle->setMaximumDaughterDCA_XY(100);

  //Parent parameters
  kfparticle->setMotherPT(0);
  kfparticle->setMinimumMass(1.08);
  kfparticle->setMaximumMass(1.15);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  se->registerSubsystem(kfparticle);

  QAKFParticle *kfpqa = new QAKFParticle("QAKFParticle_Lambda0","Lambda0",1.08,1.15);
  kfpqa->setKFParticleNodeName(ppi_reconstruction_name);
  se->registerSubsystem(kfpqa);
}

void reconstruct_Kpi_mass()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  std::string Kpi_reconstruction_name = "Kpi_reco";
  KFParticle_sPHENIX *kfparticle = new KFParticle_sPHENIX(Kpi_reconstruction_name);
  kfparticle->Verbosity(0);

  kfparticle->setDecayDescriptor("[D0 -> K^- pi^+]cc");
  kfparticle->saveOutput(false);

  kfparticle->usePID();
  kfparticle->setPIDacceptFraction(0.4);
  kfparticle->dontUseGlobalVertex();
  kfparticle->requireTrackVertexBunchCrossingMatch();
  kfparticle->allowZeroMassTracks();
  kfparticle->saveDST();
  kfparticle->setContainerName(Kpi_reconstruction_name);
  kfparticle->magFieldFile("FIELDMAP_TRACKING");

  //PV to SV cuts
  kfparticle->constrainToPrimaryVertex(true);
  kfparticle->setMotherIPchi2(100);
  kfparticle->setMotherIP(100);
  kfparticle->setMotherIP_XY(0.005);
  kfparticle->setFlightDistancechi2(5);
  kfparticle->setMinDIRA(0.9);
  kfparticle->setMinDIRA_XY(-1.1);
  kfparticle->setDecayLengthRange_XY(0.005, FLT_MAX);

  //Track parameters
  kfparticle->setMinimumTrackPT(0.2);
  kfparticle->setMaximumTrackchi2nDOF(300.);
  kfparticle->setMinTPChits(25);
  kfparticle->setMinMVTXhits(1);
  kfparticle->setMinINTThits(0);

  //Vertex parameters
  kfparticle->setMaximumVertexchi2nDOF(20);
  kfparticle->setMaximumDaughterDCA(0.1);
  kfparticle->setMaximumDaughterDCA_XY(100); //5 mm

  //Parent parameters
  kfparticle->setMotherPT(0.0);
  kfparticle->setMinimumMass(1.75);
  kfparticle->setMaximumMass(1.95);
  kfparticle->setMaximumMotherVertexVolume(0.1);

  se->registerSubsystem(kfparticle);

  QAKFParticle *kfpqa = new QAKFParticle("QAKFParticle_D0","D0",1.75,1.95);
  kfpqa->setKFParticleNodeName(Kpi_reconstruction_name);
  se->registerSubsystem(kfpqa);
}

void Fun4All_JobC(
    const int nEvents = 2,
    const std::string outfilename = "cosmictrack",
    const std::string dbtag = "2024p001",
    const std::string filelist = "filelist.list")
{
  gSystem->Load("libg4dst.so");

  ACTSGEOM::mvtx_applymisalignment = true;
  Enable::MVTX_APPLYMISALIGNMENT = true;
  
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);
  auto rc = recoConsts::instance();
  CDBInterface::instance()->Verbosity(1);

  rc->set_StringFlag("CDB_GLOBALTAG", dbtag );
  
  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);
  
  std::ifstream ifs(filelist);
  std::string filepath;
  int i = 0;
  int runnumber = 0;
  while(std::getline(ifs,filepath))
    {
      if(i==0)
	{
	   std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
	   runnumber = runseg.first;
	   int segment = runseg.second;
	   rc->set_IntFlag("RUNNUMBER", runnumber);
	   rc->set_uint64Flag("TIMESTAMP", runnumber);
	}
      std::string inputname = "InputManager" + std::to_string(i);
      auto hitsin = new Fun4AllDstInputManager(inputname);
      hitsin->fileopen(filepath);
      se->registerInputManager(hitsin);
      i++;
    }

  TpcReadoutInit( runnumber );
  std::cout<< " run: " << runnumber
	   << " samples: " << TRACKING::reco_tpc_maxtime_sample
	   << " pre: " << TRACKING::reco_tpc_time_presample
	   << " vdrift: " << G4TPC::tpc_drift_velocity_reco
	   << std::endl;
  
  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = true;

  // to turn on the default static corrections, enable the two lines below
  G4TPC::ENABLE_STATIC_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

  //to turn on the average corrections, enable the three lines below
  //note: these are designed to be used only if static corrections are also applied
  G4TPC::ENABLE_AVERAGE_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;
  G4TPC::average_correction_filename = CDBInterface::instance()->getUrl("TPC_LAMINATION_FIT_CORRECTION");
  
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  /*
   * flags for tracking
   */
  G4TPC::REJECT_LASER_EVENTS=true;
  TRACKING::pp_mode = true;
  TrackingInit();

  // reject laser events if G4TPC::REJECT_LASER_EVENTS is true 
  Reject_Laser_Events();

  
  Tracking_Reco_SiTpcTrackMatching_run2pp("TRKR_CLUSTER_SEED");
  Tracking_Reco_TrackFit_run2pp("","TRKR_CLUSTER_SEED");
  Tracking_Reco_Vertex_run2pp("TRKR_CLUSTER_SEED");
  
  auto tpcsiliconqa = new TpcSiliconQA;
  se->registerSubsystem(tpcsiliconqa);

  se->registerSubsystem(new TrackFittingQA);
  se->registerSubsystem(new VertexQA);

  reconstruct_pipi_mass();
  reconstruct_KK_mass();
  reconstruct_ppi_mass();
  reconstruct_Kpi_mass();
  
  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfilename);
  out->AddNode("Sync");
  out->AddNode("EventHeader");
  out->AddNode("GL1RAWHIT");
  out->AddNode("SvtxTrackSeedContainer");
  out->AddNode("SvtxTrackMap");
  out->AddNode("SvtxVertexMap");
  out->AddNode("TRKR_CLUSTER_SEED");
  out->AddNode("TpcTrackSeedContainer");
  out->AddNode("SiliconTrackSeedContainer");
  se->registerOutputManager(out);

  se->run(nEvents);
  se->End();

  TString qaname = "HIST_" + outfilename;
  std::string qaOutputFileName(qaname.Data());
  QAHistManagerDef::saveQARootFile(qaOutputFileName);


  CDBInterface::instance()->Print();
  se->PrintTimer();

  delete se;
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);
}
