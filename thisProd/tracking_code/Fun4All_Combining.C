/*
 * This macro shows a minimum working example of running the
 * TPC track reconstruction starting with hit unpackers
 */

// leave the GlobalVariables.C at the beginning, an empty line afterwards
// protects its position against reshuffling by clang-format
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

#include <format>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libcdbobjects.so)


void Fun4All_Combining(
    const int nEvents = 10,
    const std::string &outdir = ".",
    const std::string &outfilename = "DST_COMBINED_run3pp_ana561_2025p013_v001.root",
    const int neventsper = 1000,
    const int startseg = 0,
    const std::string &dbtag = "newcdbtag",
    const std::string &filelist = "filelistold.list",
    const std::string &histdir = "")
{
  gSystem->Load("libg4dst.so");
  auto *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(100);
  auto *rc = recoConsts::instance();
  rc->set_StringFlag("CDB_GLOBALTAG", dbtag);
  std::ifstream ifs(filelist);
  std::string filepath;

  int i = 0;
  int runNumber = 0;
  while (std::getline(ifs, filepath))
  {
    std::cout << "Adding DST with filepath: " << filepath << std::endl;
    if (i == 0)
    {
      std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
      runNumber = runseg.first;
      rc->set_IntFlag("RUNNUMBER", runNumber);
      rc->set_uint64Flag("TIMESTAMP", runNumber);
    }
    
    std::string inputname = "InputManager" + std::to_string(i);
    auto *hitsin = new Fun4AllDstInputManager(inputname);
    hitsin->TTreeCacheSize(0);
    hitsin->fileopen(filepath);
    se->registerInputManager(hitsin);
    i++;
  }


  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");

  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  
  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfilename);
  
  out->StripRunNode("CYLINDERGEOM_MVTX");
  out->StripRunNode("CYLINDERGEOM_INTT");
  out->StripRunNode("TPCGEOMCONTAINER");
  out->StripRunNode("CYLINDERGEOM_MICROMEGAS_FULL");
  out->StripRunNode("GEOMETRY_IO");
  out->SetEventNumberRollover(neventsper);  // event number for rollover
  out->StartSegment(startseg);              // starting segment number

  out->UseFileRule();
  out->SetClosingScript("./stageout.sh");
  out->SetClosingScriptArgs(outdir);
 
  se->registerOutputManager(out);
  if (nEvents < 0)
  {
    return;
  }
  se->run(nEvents);
  se->End();
  se->PrintTimer();

  CDBInterface::instance()->Print();
  delete se;
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);
}
