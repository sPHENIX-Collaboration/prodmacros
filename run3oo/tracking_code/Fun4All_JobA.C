/*
 * This macro is run in our daily CI and is intended as a minimum working
 * example showing how to unpack the raw hits into the offline tracker hit
 * format. No other reconstruction or analysis is performed
 */
#include <QA.C>

#include <GlobalVariables.C>
#include <Trkr_Clustering.C>
#include <Trkr_RecoInit.C>
#include <Trkr_Reco.C>
#include <Trkr_TpcReadoutInit.C>

#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>

#include <phool/recoConsts.h>

#include <trackreco/DSTClusterPruning.h>

#include <trackingqa/SiliconSeedsQA.h>
#include <trackingqa/TpcSeedsQA.h>
#include <trackingqa/TpcSiliconQA.h>

#include <stdio.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libmicromegas.so)
R__LOAD_LIBRARY(libtrack_reco.so)
R__LOAD_LIBRARY(libtrackingqa.so)
void Fun4All_JobA(
    const int nEvents = 2,
    const std::string outfilename = "cosmicsseed",
    const std::string dbtag = "2024p007",
    const std::string filelist = "filelist.list")
{

  gSystem->Load("libg4dst.so");

  auto se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(10); // only print every 1000th event
  auto rc = recoConsts::instance();
  CDBInterface::instance()->Verbosity(1);

  rc->set_StringFlag("CDB_GLOBALTAG", dbtag ); 

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  std::ifstream ifs(filelist);
  std::string filepath;
  int i = 0;
  int runNumber = 0;
  while(std::getline(ifs,filepath))
    {
      if(i==0)
	{
	   std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
	   runNumber = runseg.first;
	   int segment = runseg.second;
	   rc->set_IntFlag("RUNNUMBER", runNumber);
	   rc->set_IntFlag("RUNSEGMENT", segment);
	   rc->set_uint64Flag("TIMESTAMP", runNumber);
	}
      std::string inputname = "InputManager" + std::to_string(i);
      auto hitsin = new Fun4AllDstInputManager(inputname);
      hitsin->fileopen(filepath);
      se->registerInputManager(hitsin);
      i++;
    }
  
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  
  /*
   * Flags for seeding macro
   */
  TpcReadoutInit(runNumber);
  TRACKING::pp_mode = true;
  
  Enable::MVTX_APPLYMISALIGNMENT = true;
  ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;
  
  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = true;
  G4TRACKING::SC_CALIBMODE = false;
  G4TPC::REJECT_LASER_EVENTS = true;
  
    //to turn on the default static corrections, enable the two lines below
  G4TPC::ENABLE_STATIC_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

  //to turn on the average corrections derived from simulation, enable the three lines below
  //note: these are designed to be used only if static corrections are also applied
  G4TPC::ENABLE_AVERAGE_CORRECTIONS = false;
  G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;
  G4TPC::average_correction_filename = CDBInterface::instance()->getUrl("TPC_LAMINATION_FIT_CORRECTION");

  
  TrackingInit();

  // reject laser events if G4TPC::REJECT_LASER_EVENTS is true
  Reject_Laser_Events();
  
  Tracking_Reco_TrackSeed_run2pp();

  Tracking_Reco_TrackMatching_run2pp();
  
  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfilename);
  out->AddNode("Sync");
  out->AddNode("EventHeader");
  out->AddNode("TpcTrackSeedContainer");
  out->AddNode("SiliconTrackSeedContainer");
  out->AddNode("LaserEventInfo");
  out->AddNode("TRKR_CLUSTER_SEED");
  out->AddNode("TRKR_CLUSTERCROSSINGASSOC");
  out->AddNode("SvtxTrackSeedContainer");
  out->AddNode("GL1RAWHIT");

  se->registerOutputManager(out);

  auto converter = new TrackSeedTrackMapConverter("SiliconSeedConverter");
  // Default set to full SvtxTrackSeeds. Can be set to
  // SiliconTrackSeedContainer or TpcTrackSeedContainer
  converter->setTrackSeedName("SiliconTrackSeedContainer");
  converter->setTrackMapName("SiliconSvtxTrackMap");
  converter->setFieldMap(G4MAGNET::magfield_tracking);
  converter->Verbosity(0);
  se->registerSubsystem(converter);

  auto finder = new PHSimpleVertexFinder("SiliconVertexFinder");
  finder->Verbosity(0);
  finder->setDcaCut(0.1);
  finder->setTrackPtCut(0.1);
  finder->setBeamLineCut(1);
  finder->setTrackQualityCut(1000000000);
  finder->setNmvtxRequired(3);
  finder->setOutlierPairCut(0.1);
  finder->set_pp_mode(TRACKING::pp_mode);
  finder->setTrackMapName("SiliconSvtxTrackMap");
  finder->setVertexMapName("SiliconSvtxVertexMap");
  se->registerSubsystem(finder);

  auto siliconqa = new SiliconSeedsQA;
  siliconqa->setTrackMapName("SiliconSvtxTrackMap");
  siliconqa->setVertexMapName("SiliconSvtxVertexMap");
  se->registerSubsystem(siliconqa);

  auto convertertpc = new TrackSeedTrackMapConverter("TpcSeedConverter");
  // Default set to full SvtxTrackSeeds. Can be set to
  // SiliconTrackSeedContainer or TpcTrackSeedContainer
  convertertpc->setTrackSeedName("TpcTrackSeedContainer");
  convertertpc->setTrackMapName("TpcSvtxTrackMap");
  convertertpc->setFieldMap(G4MAGNET::magfield_tracking);
  convertertpc->Verbosity(0);
  se->registerSubsystem(convertertpc);

  auto findertpc = new PHSimpleVertexFinder("TpcSimpleVertexFinder");
  findertpc->Verbosity(0);
  findertpc->setDcaCut(1);
  findertpc->setTrackPtCut(0.2);
  findertpc->setBeamLineCut(1.5);
  findertpc->setTrackQualityCut(1000000000);
  //findertpc->setNmvtxRequired(3);
  findertpc->setRequireMVTX(false);
  findertpc->setOutlierPairCut(0.1);
  findertpc->setTrackMapName("TpcSvtxTrackMap");
  findertpc->setVertexMapName("TpcSvtxVertexMap");
  se->registerSubsystem(findertpc);

  auto tpcqa = new TpcSeedsQA;
  tpcqa->setTrackMapName("TpcSvtxTrackMap");
  tpcqa->setVertexMapName("TpcSvtxVertexMap");
  tpcqa->setSegment(rc->get_IntFlag("RUNSEGMENT"));
  se->registerSubsystem(tpcqa);

  auto tpcsiliconqa = new TpcSiliconQA;
  se->registerSubsystem(tpcsiliconqa);


  
  auto clusterPruner = new DSTClusterPruning("DSTClusterPruning");
  clusterPruner->pruneAllSeeds();
  se->registerSubsystem(clusterPruner);
  
  
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
