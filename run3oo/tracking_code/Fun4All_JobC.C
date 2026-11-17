/*
 * This macro is run in our daily CI and is intended as a minimum working
 * example showing how to unpack the raw hits into the offline tracker hit
 * format. No other reconstruction or analysis is performed
 */
// GlobalVariables.C has to be first, leave empty line after it so clang-format
// does not reshuffle it
#include <GlobalVariables.C>

#include <KFParticle_selections.C>
#include <QA.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>

#include <trackingqa/TpcSiliconQA.h>
#include <trackingqa/TrackFittingQA.h>
#include <trackingqa/VertexQA.h>

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

void Fun4All_JobC(
    const int nEvents = 2,
    const std::string &outfilename = "cosmictrack",
    const std::string &dbtag = "2024p001",
    const std::string &filelist = "filelist.list")
{
  gSystem->Load("libg4dst.so");

  ACTSGEOM::mvtx_applymisalignment = true;
  Enable::MVTX_APPLYMISALIGNMENT = true;

  auto *se = Fun4AllServer::instance();
  se->Verbosity(1);
  auto *rc = recoConsts::instance();
  CDBInterface::instance()->Verbosity(1);

  rc->set_StringFlag("CDB_GLOBALTAG", dbtag);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  std::ifstream ifs(filelist);
  std::string filepath;
  int i = 0;
  int runnumber = 0;
  while (std::getline(ifs, filepath))
  {
    if (i == 0)
    {
      std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
      runnumber = runseg.first;
      // int segment = runseg.second;
      rc->set_IntFlag("RUNNUMBER", runnumber);
      rc->set_uint64Flag("TIMESTAMP", runnumber);
    }
    std::string inputname = "InputManager" + std::to_string(i);
    auto *hitsin = new Fun4AllDstInputManager(inputname);
    hitsin->fileopen(filepath);
    se->registerInputManager(hitsin);
    i++;
  }

  TpcReadoutInit(runnumber);
  std::cout << " run: " << runnumber
            << " samples: " << TRACKING::reco_tpc_maxtime_sample
            << " pre: " << TRACKING::reco_tpc_time_presample
            << " vdrift: " << G4TPC::tpc_drift_velocity_reco
            << std::endl;
  TRACKING::use_slim_tracks = true;
  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = false;

  // to turn on the default static corrections, enable the two lines below
  G4TPC::ENABLE_STATIC_CORRECTIONS = false;
  G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

  // to turn on the average corrections, enable the three lines below
  // note: these are designed to be used only if static corrections are also applied
  G4TPC::ENABLE_AVERAGE_CORRECTIONS = false;
  G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;
  G4TPC::average_correction_filename = CDBInterface::instance()->getUrl("TPC_LAMINATION_FIT_CORRECTION");

  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  /*
   * flags for tracking
   */
  G4TPC::REJECT_LASER_EVENTS = true;
  TRACKING::streaming_mode = true;
  TrackingInit();

  // reject laser events if G4TPC::REJECT_LASER_EVENTS is true
  Reject_Laser_Events();

  Tracking_Reco_SiTpcTrackMatching_run2pp("TRKR_CLUSTER_SEED");
  Tracking_Reco_TrackFit_run2pp("", "TRKR_CLUSTER_SEED");
  Tracking_Reco_Vertex_run2pp("TRKR_CLUSTER_SEED");

  auto *tpcsiliconqa = new TpcSiliconQA;
  se->registerSubsystem(tpcsiliconqa);

  se->registerSubsystem(new TrackFittingQA);
  se->registerSubsystem(new VertexQA);

  //Track resonance QA setup
  set_OO_cuts();
  KFPARTICLE::save_kfpntuple = false;
  KFPARTICLE::save_kfpntuple_to_qa = true;
  KFPARTICLE::getQA = true;

  reconstruct_pipi_mass();
  reconstruct_KK_mass();
  reconstruct_ppi_mass();
  reconstruct_Kpi_mass();

  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfilename);
  out->AddNode("Sync");
  out->AddNode("EventHeader");
  out->AddNode("GL1RAWHIT");
  out->AddNode("SvtxTrackMap");
  out->AddNode("SvtxVertexMap");
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
