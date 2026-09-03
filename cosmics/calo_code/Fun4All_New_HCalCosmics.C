#ifndef FUN4ALL_NEW_HCALCOSMICS_C
#define FUN4ALL_NEW_HCALCOSMICS_C

#include <QA.C>

#include <caloreco/CaloTowerBuilder.h>
#include <caloreco/CaloTowerDefs.h>
#include <caloreco/CaloWaveformProcessing.h>

#include <calotrigger/TriggerRunInfoReco.h>

#include <calovalid/CaloFittingQA.h>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>

#include <fun4allraw/Fun4AllPrdfInputManager.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/SubsysReco.h>

#include <phool/recoConsts.h>

#include <TSystem.h>

#include <fstream>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libfun4allraw.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libffamodules.so)

void Fun4All_New_HCalCosmics(int nEvents = 0,
                             const std::string &inlist = "files.list",
                             const std::string &outfile = "DST_CALOFITTING_run3cosmics_pro001_pcdb001_v002-00067486-00000.root",
                           const std::string &outfile_hist = "HIST_CALOFITTINGQA_run3cosmics_pro001_pcdb001_v001-00067486-00000.root",
                             const std::string &dbtag = "pcdb001")
{
  // v1 uncomment:
  // CaloTowerDefs::BuilderType buildertype = CaloTowerDefs:::kPRDFTowerv1;
  // v2 uncomment:
  CaloTowerDefs::BuilderType buildertype = CaloTowerDefs::kPRDFTowerv4;
  // v3 uncomment:

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(10000);

  recoConsts *rc = recoConsts::instance();

  // conditions DB global tag
  rc->set_StringFlag("CDB_GLOBALTAG", dbtag);
  CDBInterface::instance()->Verbosity(1);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  // Get info from DB and store in DSTs
  TriggerRunInfoReco *triggerinfo = new TriggerRunInfoReco();
  se->registerSubsystem(triggerinfo);

  /////////////////
  // build towers
  CaloTowerBuilder *ctbIHCal = new CaloTowerBuilder("HCALINBUILDER");
  ctbIHCal->set_detector_type(CaloTowerDefs::HCALIN);
  ctbIHCal->set_processing_type(CaloWaveformProcessing::TEMPLATE);
  ctbIHCal->set_builder_type(buildertype);
  ctbIHCal->set_nsamples(12);
  ctbIHCal->set_offlineflag();
  ctbIHCal->set_softwarezerosuppression(true, 200);
  se->registerSubsystem(ctbIHCal);

  CaloTowerBuilder *ctbOHCal = new CaloTowerBuilder("HCALOUTBUILDER");
  ctbOHCal->set_detector_type(CaloTowerDefs::HCALOUT);
  ctbOHCal->set_processing_type(CaloWaveformProcessing::TEMPLATE);
  ctbOHCal->set_builder_type(buildertype);
  ctbOHCal->set_nsamples(12);
  ctbOHCal->set_offlineflag();
  ctbOHCal->set_softwarezerosuppression(true, 200);
  se->registerSubsystem(ctbOHCal);

  CaloFittingQA *ca = new CaloFittingQA("CaloFittingQA");
  se->registerSubsystem(ca);

  // loop over all files in file list and create an input manager for each one
  Fun4AllInputManager *In = nullptr;
  std::ifstream infile;
  infile.open(inlist);
  int iman = 0;
  std::string line;
  bool first{true};
  int runnumber = 0;
  if (infile.is_open())
  {
    while (std::getline(infile, line))
    {
      if (line[0] == '#')
      {
        std::cout << "found commented out line " << line << std::endl;
        continue;
      }
      // extract run number from first not commented out file in list
      if (first)
      {
	std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(line);
        runnumber = runseg.first;
        rc->set_uint64Flag("TIMESTAMP", runnumber);
        first = false;
      }
      std::string magname = "DSTin_" + std::to_string(iman);
      In = new Fun4AllDstInputManager(magname);
      In->Verbosity(1);
      In->AddFile(line);
      se->registerInputManager(In);
      iman++;
    }
    infile.close();
  }

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfile);
  // this strips all nodes under the Packets PHCompositeNode
  // (means removes all offline packets)
  out->StripCompositeNode("Packets");
  se->registerOutputManager(out);
  
  se->run(nEvents);
  se->End();

  QAHistManagerDef::saveQARootFile(outfile_hist);

  CDBInterface::instance()->Print();  // print used DB files
  se->PrintTimer();
  delete se;

  std::cout << "All done!" << std::endl;
  gSystem->Exit(0);
}

#endif
