/*
 * This macro is run in our daily CI and is intended as a minimum working
 * example showing how to unpack the raw hits into the offline tracker hit
 * format. No other reconstruction or analysis is performed
 */
// GlobalVariables.C has to be first, leave empty line after it so clang-format
// does not reshuffle it
#include <GlobalVariables.C>

#include <QA.C>

#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>

#include <trackreco/DSTClusterPruning.h>

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

#include <phool/recoConsts.h>

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
    const std::string &outfilename = "cosmicsseed",
    const std::string &dbtag = "2024p007",
    const std::string &filelist = "filelist.list")
{
  gSystem->Load("libg4dst.so");

  auto *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(10);  // only print every 1000th event
  auto *rc = recoConsts::instance();
  CDBInterface::instance()->Verbosity(1);

  rc->set_StringFlag("CDB_GLOBALTAG", dbtag);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  std::ifstream ifs(filelist);
  std::string filepath;
  int i = 0;
  int runNumber = 0;
  while (std::getline(ifs, filepath))
  {
    if (i == 0)
    {
      std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
      runNumber = runseg.first;
      int segment = runseg.second;
      rc->set_IntFlag("RUNNUMBER", runNumber);
      rc->set_IntFlag("RUNSEGMENT", segment);
      rc->set_uint64Flag("TIMESTAMP", runNumber);
    }
    std::string inputname = "InputManager" + std::to_string(i);
    auto *hitsin = new Fun4AllDstInputManager(inputname);
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
  TRACKING::streaming_mode = false;

  Enable::MVTX_APPLYMISALIGNMENT = true;
  ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;

  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = true;
  G4TRACKING::SC_CALIBMODE = false;
  G4TPC::REJECT_LASER_EVENTS = true;

  // to turn on the default static corrections, enable the two lines below
  G4TPC::ENABLE_STATIC_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

  // to turn on the average corrections derived from simulation, enable the three lines below
  // note: these are designed to be used only if static corrections are also applied
  G4TPC::ENABLE_AVERAGE_CORRECTIONS = false;
  G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;
  G4TPC::average_correction_filename = CDBInterface::instance()->getUrl("TPC_LAMINATION_FIT_CORRECTION");

  TrackingInit();

  // reject laser events if G4TPC::REJECT_LASER_EVENTS is true
  Reject_Laser_Events();

  /*
   * Silicon Seeding
   */
  Tracking_Reco_SiliconSeed_run2pp();

  /*
   * Tpc Seeding
   */
  auto *seeder = new PHCASeeding("PHCASeeding");
  double fieldstrength = std::numeric_limits<double>::quiet_NaN();  // set by isConstantField if constant
  bool ConstField = isConstantField(G4MAGNET::magfield_tracking, fieldstrength);
  if (ConstField)
  {
    seeder->useConstBField(true);
    seeder->constBField(fieldstrength);
  }
  else
  {
    seeder->set_field_dir(-1 * G4MAGNET::magfield_rescale);
    seeder->useConstBField(false);
    seeder->magFieldFile(G4MAGNET::magfield_tracking);  // to get charge sign right
  }
  seeder->Verbosity(0);
  seeder->SetLayerRange(7, 55);
  seeder->reject_zsize1_clusters(true);
  seeder->SetSearchWindow(2.0, 0.05);  // (z width, phi width)
  seeder->SetMinHitsPerCluster(0);
  seeder->SetClusAdd_delta_window(3.0, 0.06);
  seeder->SetMinClustersPerTrack(3);
  seeder->useFixedClusterError(true);
  // improves seeding performance by effectively seeding in drift time rather than z
  seeder->set_pp_mode(true);
  se->registerSubsystem(seeder);

  // expand stubs in the TPC using simple kalman filter
  auto *cprop = new PHSimpleKFProp("PHSimpleKFProp");
  cprop->set_field_dir(G4MAGNET::magfield_rescale);
  if (ConstField)
  {
    cprop->useConstBField(true);
    cprop->setConstBField(fieldstrength);
  }
  else
  {
    cprop->magFieldFile(G4MAGNET::magfield_tracking);
    cprop->set_field_dir(-1 * G4MAGNET::magfield_rescale);
  }
  cprop->useFixedClusterError(true);
  cprop->set_max_window(5.);
  cprop->set_max_seeds(50000);
  cprop->Verbosity(0);
  cprop->set_pp_mode(true);
  se->registerSubsystem(cprop);

  // apply preliminary distortion corrections to TPC clusters before crossing is known
  // and refit the trackseeds. Replace KFProp fits with the new fit parameters in the TPC seeds.
  auto *prelim_distcorr = new PrelimDistortionCorrection;
  prelim_distcorr->set_pp_mode(true);
  prelim_distcorr->Verbosity(0);
  se->registerSubsystem(prelim_distcorr);

  /*
   * Track Matching between silicon and TPC
   */
  // The normal silicon association methods
  // Match the TPC track stubs from the CA seeder to silicon track stubs from PHSiliconTruthTrackSeeding
  auto *silicon_match = new PHSiliconTpcTrackMatching;
  silicon_match->Verbosity(0);
  silicon_match->Verbosity(0);
  silicon_match->set_pp_mode(TRACKING::streaming_mode);
  if (G4TPC::ENABLE_AVERAGE_CORRECTIONS)
  {
    // for general tracking
    // Eta/Phi window is determined by 3 sigma window
    // X/Y/Z window is determined by 4 sigma window
    silicon_match->window_deta.set_posQoverpT_maxabs({-0.014, 0.0331, 0.48});
    silicon_match->window_deta.set_negQoverpT_maxabs({-0.006, 0.0235, 0.52});
    silicon_match->set_deltaeta_min(0.03);
    silicon_match->window_dphi.set_QoverpT_range({-0.15, 0, 0}, {0.15, 0, 0});
    silicon_match->window_dx.set_QoverpT_maxabs({3.0, 0, 0});
    silicon_match->window_dy.set_QoverpT_maxabs({3.0, 0, 0});
    silicon_match->window_dz.set_posQoverpT_maxabs({1.138, 0.3919, 0.84});
    silicon_match->window_dz.set_negQoverpT_maxabs({0.719, 0.6485, 0.65});
    silicon_match->set_crossing_deltaz_max(30);
    silicon_match->set_crossing_deltaz_min(2);

    // for distortion correction using SI-TPOT fit and track pT>0.5
    if (G4TRACKING::SC_CALIBMODE)
    {
      silicon_match->window_deta.set_posQoverpT_maxabs({0.016, 0.0060, 1.13});
      silicon_match->window_deta.set_negQoverpT_maxabs({0.022, 0.0022, 1.44});
      silicon_match->set_deltaeta_min(0.03);
      silicon_match->window_dphi.set_QoverpT_range({-0.15, 0, 0}, {0.09, 0, 0});
      silicon_match->window_dx.set_QoverpT_maxabs({2.0, 0, 0});
      silicon_match->window_dy.set_QoverpT_maxabs({1.5, 0, 0});
      silicon_match->window_dz.set_posQoverpT_maxabs({1.213, 0.0211, 2.09});
      silicon_match->window_dz.set_negQoverpT_maxabs({1.307, 0.0001, 4.52});
      silicon_match->set_crossing_deltaz_min(1.2);
    }
  }
  se->registerSubsystem(silicon_match);

  // Match TPC track stubs from CA seeder to clusters in the micromegas layers
  auto *mm_match = new PHMicromegasTpcTrackMatching;
  mm_match->Verbosity(0);
  mm_match->set_rphi_search_window_lyr1(3);
  mm_match->set_rphi_search_window_lyr2(15.0);
  mm_match->set_z_search_window_lyr1(30.0);
  mm_match->set_z_search_window_lyr2(3.);

  mm_match->set_min_tpc_layer(38);             // layer in TPC to start projection fit
  mm_match->set_test_windows_printout(false);  // used for tuning search windows only
  se->registerSubsystem(mm_match);


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
  out->StripRunNode("CYLINDERGEOM_MVTX");
  out->StripRunNode("CYLINDERGEOM_INTT");
  out->StripRunNode("TPCGEOMCONTAINER");
  out->StripRunNode("CYLINDERGEOM_MICROMEGAS_FULL");
  out->StripRunNode("GEOMETRY_IO");
  se->registerOutputManager(out);

  auto *converter = new TrackSeedTrackMapConverter("SiliconSeedConverter");
  // Default set to full SvtxTrackSeeds. Can be set to
  // SiliconTrackSeedContainer or TpcTrackSeedContainer
  converter->setTrackSeedName("SiliconTrackSeedContainer");
  converter->setTrackMapName("SiliconSvtxTrackMap");
  converter->setFieldMap(G4MAGNET::magfield_tracking);
  converter->Verbosity(0);
  se->registerSubsystem(converter);

  auto *finder = new PHSimpleVertexFinder("SiliconVertexFinder");
  finder->Verbosity(0);
  finder->setDcaCut(0.1);
  finder->setTrackPtCut(0.1);
  finder->setBeamLineCut(1);
  finder->setTrackQualityCut(1000000000);
  finder->setNmvtxRequired(3);
  finder->setOutlierPairCut(0.1);
  finder->setTrackMapName("SiliconSvtxTrackMap");
  finder->setVertexMapName("SiliconSvtxVertexMap");
  se->registerSubsystem(finder);

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

  auto *tpcsiliconqa = new TpcSiliconQA;
  se->registerSubsystem(tpcsiliconqa);

  auto *clusterPruner = new DSTClusterPruning("DSTClusterPruning");
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
