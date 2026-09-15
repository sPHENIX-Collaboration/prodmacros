/*
 * This macro shows a minimum working example of running the
 * TPC track reconstruction starting with hit unpackers
 */

// leave the GlobalVariables.C at the beginning, an empty line afterwards
// protects its position against reshuffling by clang-format
#include <GlobalVariables.C>

#include <G4_ActsGeom.C>
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <G4_Mbd.C>
#include <QA.C>
#include <Trkr_Clustering.C>
#include <Trkr_LaserClustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>

#include <trackreco/DSTClusterPruning.h>
#include <cdbobjects/CDBTTree.h>

#include <inttrawhitqa/InttQa.h>
#include <inttrawhitqa/InttRawHitQA.h>

#include <mvtxrawhitqa/MvtxRawHitQA.h>

#include <tpcqa/TpcLaserQA.h>
#include <tpcqa/TpcRawHitQA.h>

#include <trackingqa/InttClusterQA.h>
#include <trackingqa/MicromegasClusterQA.h>
#include <trackingqa/MvtxClusterQA.h>
#include <trackingqa/TpcClusterQA.h>

#include <trackingqa/SiliconSeedsQA.h>
#include <trackingqa/TpcSeedsQA.h>
#include <trackingqa/TpcSiliconQA.h>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllUtils.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>

#include <phool/recoConsts.h>

#include <tpctrackreco/TpcCrossingFinder.h>
#include <tpctrackreco/TpcPolyClusterTrkrClusterConverter.h>
#include <tpctrackreco/TpcPolyTrackSeedConverter.h>
#include <tpctrackreco/Tpc_AssembledTrackReco.h>
#include <tpctrackreco/Tpc_ModuleTrackReco.h>
#include <tpctrackreco/Tpc_PolyClusterizer.h>
#include <tpctrackreco/Tpc_PolyTrackReco.h>
#include <tpctrackreco/Tpc_PolyTrackVertexer.h>

#include <trackingdiagnostics/Tpc_AssembledTrackDisplay.h>
#include <trackingdiagnostics/Tpc_ModuleTrackDisplay.h>
#include <trackingdiagnostics/Tpc_PolyClusterDisplay.h>
#include <trackingdiagnostics/Tpc_PolyClusterResiduals.h>
#include <trackingdiagnostics/TrackResiduals.h>

#include <fun4all/Fun4AllMemoryHistograms.h>

#include <format>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libphool.so)
R__LOAD_LIBRARY(libcdbobjects.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libmicromegas.so)
R__LOAD_LIBRARY(libPHGarfield.so)
R__LOAD_LIBRARY(libtpctrackreco.so)
R__LOAD_LIBRARY(libTrackingDiagnostics.so)


// 'Golden' store of p+p
//.x Fun4All_raw_hit_TPC_reco.C(2, 79513, 0, ".", 0, "run3pp", "ana532_nocdbtag_v001","HITS_ppFieldOn")
//.x Fun4All_raw_hit_TPC_reco.C(2, 79516, 0, ".", 0, "run3pp", "ana532_nocdbtag_v001","HITS_ppFieldOn")

// 6x6       75570-75573 - 1mrad; 75574 - 0mrad
//.x Fun4All_raw_hit_TPC_reco.C(10, 75570, 0, ".", 0, "run3auau", "ana514_nocdbtag_v001","HITS_AuAu_ZeroField_1mrad")
//.x Fun4All_raw_hit_TPC_reco.C(10, 75574, 0, ".", 0, "run3auau", "ana514_nocdbtag_v001","HITS_AuAu_ZeroField_0mrad")

// 111x111 Au+Au Field On; 75391 - close to end of store, 75405 - beginning of the next store
//.x Fun4All_raw_hit_TPC_reco.C(2, 75391, 0, ".", 0, "run3auau", "ana514_nocdbtag_v001","HITS_AuAuFieldOn")
//.x Fun4All_raw_hit_TPC_reco.C(2, 75405, 0, ".", 0, "run3auau", "ana514_nocdbtag_v001","HITS_AuAuFieldOn")

// 111x111 O+O
//.x Fun4All_raw_hit_TPC_reco.C(2, 82626, 0, ".", 0, "run3oo", "ana537_nocdbtag_v001","HITS_OO")

void Fun4All_PolySeeding_Combined(
    const int nEvents = 10,
    const std::string &inputfile = "DST_STREAMING_EVENT_combined_run3pp_ana561_2025p013_v001-00081612-00000.root",
    const std::string &outdir = ".",
    const std::string &outfilename = "DST_TRACKSEEDS_run3pp_ana561_2025p013_v001-00081612-00000.root",
    const std::string &dbtag = "newcdbtag",
    const std::string &histdir = "")
{
  Fun4AllMemoryHistograms::instance()->Enable();
  const bool convertSeeds = false;
  auto *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(100);
  auto *rc = recoConsts::instance();

  // tracking flags
  G4TPC::ENABLE_CENTRAL_MEMBRANE_CLUSTERING = true;
  
  G4TPC::REJECT_LASER_EVENTS = true;
  // Flag for running the tpc hit unpacker with zero suppression on
  TRACKING::tpc_zero_supp = true;
  
  Enable::CDB = true;
  
  G4TPC::sampa_tzero_bias = 0;

  // First order corrections will be applied from PHGarfield
  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = false;
  G4TPC::ENABLE_STATIC_CORRECTIONS = false;
  G4TPC::ENABLE_AVERAGE_CORRECTIONS = false;

      std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(inputfile);
      int runNumber = runseg.first;
      int segment = runseg.second;
      rc->set_IntFlag("RUNNUMBER", runNumber);
      rc->set_IntFlag("RUNSEGMENT", segment);
      rc->set_uint64Flag("TIMESTAMP", runNumber);

    auto *hitsin = new Fun4AllDstInputManager("DSTin");
    hitsin->TTreeCacheSize(0);
    hitsin->fileopen(inputfile);
    se->registerInputManager(hitsin);


  CDBInterface::instance()->Verbosity(1);
  
  rc->set_StringFlag("CDB_GLOBALTAG", dbtag);
  rc->set_uint64Flag("TIMESTAMP", runNumber);

  
  std::cout << " run: " << runNumber
            << " samples: " << TRACKING::reco_tpc_maxtime_sample
            << " pre: " << TRACKING::reco_tpc_time_presample
            << " vdrift: " << G4TPC::tpc_drift_velocity_reco
            << std::endl;

  TRACKING::streaming_mode = true;

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");

  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);
  
  TpcReadoutInit(runNumber);

  TrackingInit();

  for (int felix = 0; felix < 6; felix++)
  {
    Mvtx_HitUnpacking(std::to_string(felix));
  }
  for (int server = 0; server < 8; server++)
  {
    Intt_HitUnpacking(std::to_string(server));
  }
  std::ostringstream ebdcname;
  for (int ebdc = 0; ebdc < 24; ebdc++)
  {
    for (int endpoint = 0; endpoint < 2; endpoint++)
    {
      ebdcname.str("");
      if (ebdc < 10)
      {
        ebdcname << "0";
      }
      ebdcname << ebdc << "_" << endpoint;
      Tpc_HitUnpacking(ebdcname.str());
    }
  }
  Micromegas_HitUnpacking();

  //==============================================================

  Mvtx_Clustering();
  Intt_Clustering();
  Micromegas_Clustering();

  //==============================================================


  //==============================================================

  Tracking_Reco_SiliconSeed_run2pp();

  
  auto *converter = new TrackSeedTrackMapConverter("SiliconSeedToSvtxTrackMap");
  converter->setTrackSeedName("SiliconTrackSeedContainer");
  converter->setTrackMapName("SiliconSvtxTrackMap");
  converter->setClusterMapName("TRKR_CLUSTER");
  se->registerSubsystem(converter);

  auto *finder_svx = new PHSimpleVertexFinder("SiliconVertexFinder");
  finder_svx->Verbosity(0);
  finder_svx->setDcaCut(0.1);
  finder_svx->setTrackPtCut(0.2);
  finder_svx->setBeamLineCut(1);
  finder_svx->setTrackQualityCut(500);
  finder_svx->setNmvtxRequired(3);
  finder_svx->setOutlierPairCut(0.1);
  finder_svx->setTrackMapName("SiliconSvtxTrackMap");
  finder_svx->setVertexMapName("SiliconSvtxVertexMap");
  se->registerSubsystem(finder_svx);

  //==============================================================

  se->registerSubsystem(new Tpc_ModuleTrackReco());     // makes TPC_MODULETRACKS
  se->registerSubsystem(new Tpc_AssembledTrackReco());  // makes TPC_ASSEMBLEDTRACKS

  auto *crossingFinder = new TpcCrossingFinder();
  crossingFinder->Verbosity(0);
  crossingFinder->setInputNodeName("TPC_ASSEMBLEDTRACKS");
  crossingFinder->setOutputNodeName("TPC_CROSSING_DECISIONS");
  crossingFinder->setVertexMapNodeName("SiliconSvtxVertexMap");  // optional, configurable
  se->registerSubsystem(crossingFinder);

  auto *cluster = new Tpc_PolyClusterizer();  // makes TPC_POLYCLUSTERS
  cluster->setUseSurveyGeometry(true);
  se->registerSubsystem(cluster);

  se->registerSubsystem(new Tpc_PolyTrackReco());      // makes TPC_POLYTRACKS
  se->registerSubsystem(new Tpc_PolyTrackVertexer());  // makes TPC_POLYTRACKVERTICES

  se->registerSubsystem(new TpcPolyTrackSeedConverter());           // converts TPC_POLYTRACKS to TpcTrackSeed
  se->registerSubsystem(new TpcPolyClusterTrkrClusterConverter());  // converts TPC_POLYCLUSTERS to TRKR_CLUSTER

  
  Tpc_LaserEventIdentifying();
  TPC_LaminationClustering();

  TPC_LaserClustering();
  Reject_Laser_Events();

  Tracking_Reco_TrackMatching_run2pp();
  
  auto *clusterPruner = new DSTClusterPruning("DSTClusterPruning");
  clusterPruner->pruneAllSeeds();
  se->registerSubsystem(clusterPruner);

  //! QA
  
  se->registerSubsystem(new MvtxClusterQA);
  se->registerSubsystem(new InttClusterQA);
  se->registerSubsystem(new TpcClusterQA);
  se->registerSubsystem(new MicromegasClusterQA);

  auto *mvtx = new MvtxRawHitQA;
  se->registerSubsystem(mvtx);

  se->registerSubsystem(new InttQa);

  auto *tpc = new TpcRawHitQA;
  se->registerSubsystem(tpc);

  auto *LaserQA = new TpcLaserQA;
  se->registerSubsystem(LaserQA);

  auto *siliconqa = new SiliconSeedsQA;
  siliconqa->setTrackMapName("SiliconSvtxTrackMap");
  siliconqa->setVertexMapName("SiliconSvtxVertexMap");
  se->registerSubsystem(siliconqa);

  auto *convertertpc = new TrackSeedTrackMapConverter("TpcSeedConverter");
  // Default set to full SvtxTrackSeeds. Can be set to
  // SiliconTrackSeedContainer or TpcTrackSeedContainer
  convertertpc->setTrackSeedName("TpcTrackSeedContainer");
  convertertpc->setTrackMapName("TpcSvtxTrackMap");
  convertertpc->setFieldMap(G4MAGNET::magfield_tracking);
  convertertpc->Verbosity(0);
  se->registerSubsystem(convertertpc);

  auto *findertpc = new PHSimpleVertexFinder("TpcSimpleVertexFinder");
  findertpc->Verbosity(0);
  findertpc->setDcaCut(1);
  findertpc->setTrackPtCut(0.2);
  findertpc->setBeamLineCut(1.5);
  findertpc->setTrackQualityCut(1000000000);
  // findertpc->setNmvtxRequired(3);
  findertpc->setRequireMVTX(false);
  findertpc->setOutlierPairCut(0.1);
  findertpc->setTrackMapName("TpcSvtxTrackMap");
  findertpc->setVertexMapName("TpcSvtxVertexMap");
  se->registerSubsystem(findertpc);

  auto *tpcqa = new TpcSeedsQA;
  tpcqa->setTrackMapName("TpcSvtxTrackMap");
  tpcqa->setVertexMapName("TpcSvtxVertexMap");
  tpcqa->setSegment(rc->get_IntFlag("RUNSEGMENT"));
  se->registerSubsystem(tpcqa);

  

  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfilename);

  out->AddNode("Sync");
  out->AddNode("EventHeader");
  out->AddNode("TPC_CROSSING_DECISIONS");
  out->AddNode("TPC_POLYCLUSTERS");
  out->AddNode("TPC_POLYTRACKS");
  out->AddNode("TPC_POLYTRACKVERTICES");
  out->AddNode("TRKR_CLUSTER");
  out->AddNode("TRKR_CLUSTERCROSSINGASSOC");
  out->AddNode("LaserEventInfo");
  out->AddNode("GL1RAWHIT");
  out->AddNode("TRKR_CLUSTER_SEED");
  if (G4TPC::ENABLE_CENTRAL_MEMBRANE_CLUSTERING)
  {
    out->AddNode("LASER_CLUSTER");
    out->AddNode("LAMINATION_CLUSTER");
  }
  
  out->AddNode("TpcTrackSeedContainer");
  out->AddNode("SiliconTrackSeedContainer");
  out->AddNode("LaserEventInfo");
  out->AddNode("TRKR_CLUSTERCROSSINGASSOC");
  out->AddNode("SvtxTrackSeedContainer");
  out->AddNode("GL1RAWHIT");
  
  out->StripRunNode("CYLINDERGEOM_MVTX");
  out->StripRunNode("CYLINDERGEOM_INTT");
  out->StripRunNode("TPCGEOMCONTAINER");
  out->StripRunNode("CYLINDERGEOM_MICROMEGAS_FULL");
  out->StripRunNode("GEOMETRY_IO");
  
  se->registerOutputManager(out);
  auto *hm = QAHistManagerDef::getHistoManager();
  std::string histoout = "HIST_" + outfilename;
  hm->setOutfileName(histoout);
  if (nEvents < 0)
  {
    return;
  }
  se->run(nEvents);
  se->End();
  Fun4AllMemoryHistograms::instance()->SaveHistos("testarena.root");
  se->PrintTimer();

  CDBInterface::instance()->Print();
  delete se;
  gROOT->EndOfProcessCleanups();
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);
}
