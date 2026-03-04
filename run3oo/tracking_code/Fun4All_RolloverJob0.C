/*
 * This macro is run in our daily CI and is intended as a minimum working
 * example showing how to unpack the raw hits into the offline tracker hit
 * format. No other reconstruction or analysis is performed
 */
#include <GlobalVariables.C>

#include <QA.C>
#include <Trkr_Clustering.C>
#include <Trkr_LaserClustering.C>
#include <Trkr_RecoInit.C>

#include <inttrawhitqa/InttQa.h>
#include <inttrawhitqa/InttRawHitQA.h>

#include <mvtxrawhitqa/MvtxRawHitQA.h>

#include <tpcqa/TpcRawHitQA.h>
#include <tpcqa/TpcLaserQA.h>

#include <trackingqa/InttClusterQA.h>
#include <trackingqa/MicromegasClusterQA.h>
#include <trackingqa/MvtxClusterQA.h>
#include <trackingqa/TpcClusterQA.h>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>

#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/recoConsts.h>

#include <format>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libmicromegas.so)
R__LOAD_LIBRARY(libinttrawhitqa.so)
R__LOAD_LIBRARY(libmvtxrawhitqa.so)
R__LOAD_LIBRARY(libtpcqa.so)
R__LOAD_LIBRARY(libtrackingqa.so)

void Fun4All_RolloverJob0(
    const int nEvents = 2,
    const int  /*runnumber*/ = 41626,
    const std::string& outdir = ".",
    const std::string& outfilename = "out.root",
    const int neventsper = 100,
    const int startseg = 0,
    const std::string& dbtag = "2024p001",
    const std::string& filelist = "filelist.list",
    const std::string& histdir = "")
{

  gSystem->Load("libg4dst.so");
  //char filename[500];
  //sprintf(filename, "%s%08d-0000.root", inputRawHitFile.c_str(), runnumber);
 
  Enable::MVTX_APPLYMISALIGNMENT = true;
  ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;
  
  TRACKING::tpc_zero_supp = true;
  G4TPC::ENABLE_CENTRAL_MEMBRANE_CLUSTERING = true;
  
  auto *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(100); // only print every 1000th event
  auto *rc = recoConsts::instance();
  
  std::ifstream ifs(filelist);
  std::string filepath; 
  
  int i = 0;
  bool process_endpoints = false;
  while(std::getline(ifs,filepath))
    {
      std::cout << "Adding DST with filepath: " << filepath << std::endl; 
     if(i==0)
	{
	   std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
	   int runNumber = runseg.first;
	   //int segment = runseg.second;
	   rc->set_IntFlag("RUNNUMBER", runNumber);
	   rc->set_uint64Flag("TIMESTAMP", runNumber);
        
	}
       if(filepath.find("ebdc") != std::string::npos)
	{
	  if(filepath.find("_0_") != std::string::npos ||
	     filepath.find("_1_") != std::string::npos)
	    {
	      process_endpoints = true;
	    }
	}
      std::string inputname = "InputManager" + std::to_string(i);
      auto *hitsin = new Fun4AllDstInputManager(inputname);
      hitsin->fileopen(filepath);
      se->registerInputManager(hitsin);
      i++;
    }

  CDBInterface::instance()->Verbosity(1);

  rc->set_StringFlag("CDB_GLOBALTAG", dbtag );

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);
  

  TrackingInit();

  for(int felix=0; felix < 6; felix++)
    {
      Mvtx_HitUnpacking(std::to_string(felix));
    }
  for(int server = 0; server < 8; server++)
    {
      Intt_HitUnpacking(std::to_string(server));
    }

  std::cout << "Process endpoints is " << process_endpoints << std::endl;
  std::string ebdcname;
  for(int ebdc = 0; ebdc < 24; ebdc++)
    {
      if(!process_endpoints)
	{
          ebdcname = std::format("{:02}",ebdc);
	  Tpc_HitUnpacking(ebdcname);
	}
      else if(process_endpoints)
	{
	  for(int endpoint = 0; endpoint <2; endpoint++)
	    {
	      ebdcname = std::format("{:02}_{}",ebdc,endpoint);
	      Tpc_HitUnpacking(ebdcname);
	    }
	}
    }

  Micromegas_HitUnpacking();

  Mvtx_Clustering();

  Intt_Clustering();

  Tpc_LaserEventIdentifying();

  TPC_LaminationClustering();

  TPC_LaserClustering();

  auto *tpcclusterizer = new TpcClusterizer;
  tpcclusterizer->Verbosity(0);
  tpcclusterizer->set_do_hit_association(G4TPC::DO_HIT_ASSOCIATION);
  tpcclusterizer->set_rawdata_reco();
  tpcclusterizer->set_reject_event(G4TPC::REJECT_LASER_EVENTS);
  se->registerSubsystem(tpcclusterizer);

  Micromegas_Clustering();

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
  std::string dstoutname = outfilename;
  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", dstoutname);
  out->AddNode("Sync");
  out->AddNode("EventHeader");
  out->AddNode("TRKR_CLUSTER");
  out->AddNode("TRKR_CLUSTERCROSSINGASSOC");
  out->AddNode("LaserEventInfo");
  out->AddNode("GL1RAWHIT");
  if(G4TPC::ENABLE_CENTRAL_MEMBRANE_CLUSTERING)
  {
    out->AddNode("LASER_CLUSTER");
    out->AddNode("LAMINATION_CLUSTER");
  }
  out->SetEventNumberRollover(neventsper); // event number for rollover
  out->StartSegment(startseg); // starting segment number
  out->UseFileRule();
  out->SetClosingScript("./stageout.sh");
  out->SetClosingScriptArgs(outdir);
  se->registerOutputManager(out);

  auto *hm = QAHistManagerDef::getHistoManager();
  hm->CopyRolloverSetting(out);
  std::string histoout = "HIST_" + outfilename;
  hm->setOutfileName(histoout);
  if ( histdir != "" )
  {
    hm->SetClosingScriptArgs(histdir);
  } else {
    hm->SetClosingScriptArgs(outdir);
  }
  
  se->run(nEvents);
  se->End();

  CDBInterface::instance()->Print();
  se->PrintTimer();

  delete se;
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);
}
