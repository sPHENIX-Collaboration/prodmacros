#include <QA.C>

#include <inttcalib/InttCalib.h>

#include <ffamodules/HeadReco.h>
#include <ffamodules/FlagHandler.h>
#include <ffamodules/SyncReco.h>
#include <ffamodules/CDBInterface.h>

#include <ffarawmodules/InttCheck.h>
#include <ffarawmodules/StreamingCheck.h>
#include <ffarawmodules/TpcCheck.h>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllUtils.h>

#include <fun4allraw/Fun4AllStreamingInputManager.h>
#include <fun4allraw/InputManagerType.h>
#include <fun4allraw/SingleGl1PoolInput.h>
#include <fun4allraw/SingleInttPoolInput.h>
#include <fun4allraw/SingleMicromegasPoolInput.h>
#include <fun4allraw/SingleMvtxPoolInput.h>
#include <fun4allraw/SingleTpcPoolInput.h>
#include <fun4allraw/SingleTpcTimeFrameInput.h>

#include <phool/recoConsts.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libfun4allraw.so)
R__LOAD_LIBRARY(libffarawmodules.so)
R__LOAD_LIBRARY(libintt.so)

bool isGood(const string &infile);
int getrunnumber(const string &listfile);

void Fun4All_SingleStream_Combiner(int nEvents = 0,
				   const int runnumber1 = 30117,
				   const string &outdir = "/sphenix/lustre01/sphnxpro/commissioning/slurp/tpccosmics/",
				   const string& histdir = "/sphenix/data/data02/sphnxpro/single_streamhist/",
				   const string &type = "beam",
				   const int neventsper = 100,
				   const string &dbtag = "newcdbtag",
				   const string &input_gl1file = "gl1daq.list",
				   const string &input_tpcfile00 = "tpc00.list",
				   const string &input_inttfile00 = "intt0.list",
				   const string &input_mvtxfile00 = "mvtx0.list",
				   const string &input_tpotfile = "tpot.list")
{
  int registered_subsystems = 0;
// GL1 which provides the beam clock reference (if we ran with GL1)
  vector<string> gl1_infile;
  gl1_infile.push_back(input_gl1file);


// MVTX
  vector<string> mvtx_infile;
  mvtx_infile.push_back(input_mvtxfile00);

// INTT
  vector<string> intt_infile;
  intt_infile.push_back(input_inttfile00);

  vector<string> tpc_infile;
  tpc_infile.push_back(input_tpcfile00);

// TPOT
  vector<string> tpot_infile;
  tpot_infile.push_back(input_tpotfile);

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(10000); // only print every 10000th event

// Get the runnumber from the filename
    int runnumber = -99999;
  if (!gl1_infile.empty())
  {
    runnumber = getrunnumber(gl1_infile[0]);
  }
  else if (!mvtx_infile.empty())
  {
    runnumber = getrunnumber(mvtx_infile[0]);
  }
  else if (!intt_infile.empty())
  {
    runnumber = getrunnumber(intt_infile[0]);
  }
  else if (!tpc_infile.empty())
  {
    runnumber = getrunnumber(tpc_infile[0]);
  }
  else if (!tpot_infile.empty())
  {
    runnumber = getrunnumber(tpot_infile[0]);
  }

  recoConsts *rc = recoConsts::instance();
  CDBInterface::instance()->Verbosity(1);
  rc->set_StringFlag("CDB_GLOBALTAG", dbtag );

  rc->set_IntFlag("RUNNUMBER", runnumber);
  Fun4AllStreamingInputManager *in = new Fun4AllStreamingInputManager("Comb");
//  in->Verbosity(3);

// create and register input managers
  int i = 0;

  for (auto iter : gl1_infile)
  {
    if (isGood(iter))
    {
      SingleGl1PoolInput *gl1_sngl = new SingleGl1PoolInput("GL1_" + to_string(i));
      //    gl1_sngl->Verbosity(3);
      gl1_sngl->AddListFile(iter);
      in->registerStreamingInput(gl1_sngl, InputManagerType::GL1);
      i++;
    }
  }
  i = 0;


  bool isInttStreaming = true;
  for (auto iter : intt_infile)
  {
    if (isGood(iter))
    {
      SingleInttPoolInput *intt_sngl = new SingleInttPoolInput("INTT_" + to_string(i));
      //intt_sngl->Verbosity(3);

      /// find the ebdc number from the filename
      std::string filepath, felix;
      std::ifstream ifs(iter);
      while(std::getline(ifs, filepath))
      {
	auto pos = filepath.find("intt");
	felix = filepath.substr(pos+4, 1);
	break;
      }
      intt_sngl->setHitContainerName("INTTRAWHIT_" + felix);
      intt_sngl->AddListFile(iter);
      in->registerStreamingInput(intt_sngl, InputManagerType::INTT);
      i++;
      registered_subsystems++;
    }
  }
  i = 0;
  for (auto iter : mvtx_infile)
  {
    if (isGood(iter))
    {

      /// find the ebdc number from the filename
      std::string filepath, felix;
      std::ifstream ifs(iter);
      while(std::getline(ifs, filepath))
      {
	auto pos = filepath.find("mvtx");
	felix = filepath.substr(pos+4, 1);
	break;
      }

      SingleMvtxPoolInput *mvtx_sngl = new SingleMvtxPoolInput("MVTX_" + to_string(i));
//    mvtx_sngl->Verbosity(5);
      mvtx_sngl->setHitContainerName("MVTXRAWHIT_" + felix);
      mvtx_sngl->setRawEventHeaderName("MVTXRAWEVTHEADER_" + felix);
      mvtx_sngl->AddListFile(iter);
      in->registerStreamingInput(mvtx_sngl, InputManagerType::MVTX);
      i++;
      registered_subsystems++;
    }
  }
  i = 0;
  for (auto iter : tpc_infile)
  {
    if (isGood(iter))
    {

      /// find the ebdc number from the filename
      std::string filepath, ebdc;
      std::ifstream ifs(iter);
      while(std::getline(ifs, filepath))
      {
	auto pos = filepath.find("ebdc");
	ebdc = filepath.substr(pos+4, 4);
	break;
      }

      SingleTpcTimeFrameInput *tpc_sngl = new SingleTpcTimeFrameInput("TPC_" + to_string(i));
//    tpc_sngl->Verbosity(2);
      //   tpc_sngl->DryRun();
      tpc_sngl->setHitContainerName("TPCRAWHIT_" + ebdc);
      tpc_sngl->AddListFile(iter);
      in->registerStreamingInput(tpc_sngl, InputManagerType::TPC);
      i++;
      registered_subsystems++;
    }
  }
  i = 0;

  for (auto iter : tpot_infile)
  {
    if (isGood(iter))
    {
      SingleMicromegasPoolInput *mm_sngl = new SingleMicromegasPoolInput("MICROMEGAS_" + to_string(i));
      //   sngl->Verbosity(3);
      mm_sngl->SetBcoRange(10);
      mm_sngl->SetNegativeBco(2);
      mm_sngl->SetBcoPoolSize(150);
      mm_sngl->AddListFile(iter);
      in->registerStreamingInput(mm_sngl, InputManagerType::MICROMEGAS);
      i++;
      registered_subsystems++;
    }
  }

  se->registerInputManager(in);
  if (registered_subsystems == 0)
  {
    std::cout << "No streaming readoung input managers resgistered, quitting" << std::endl;
    gSystem->Exit(1);
  }
  // StreamingCheck *scheck = new StreamingCheck();
  // scheck->SetTpcBcoRange(130);
  // se->registerSubsystem(scheck);
  // TpcCheck *tpccheck = new TpcCheck();
  // tpccheck->Verbosity(3);
  // tpccheck->SetBcoRange(130);
  // se->registerSubsystem(tpccheck);

  for (auto iter : intt_infile)
  {
    if (isGood(iter))
    {
      std::string filepath, felix;
      std::ifstream ifs(iter);
      while(std::getline(ifs, filepath))
      {
	auto pos = filepath.find("intt");
	felix = filepath.substr(pos+4, 1);
	break;
      }
      auto inttcalib = new InttCalib("INTTCalib_" + felix);
      char hotmapfilename[500];
      sprintf(hotmapfilename,"./CALIB_HOTMAP_%s-%08i-%05i.root",type.c_str(), runnumber, 0);

      char bcomapfilename[500];
      sprintf(bcomapfilename,"./CALIB_BCOMAP_%s-%08i-%05i.root", type.c_str(), runnumber, 0);

      char pngfilename[500];
      sprintf(pngfilename, "./CALIB_PNG_%s-%08i-%05i.root", type.c_str(), runnumber, 0);

      inttcalib->SetRawHitContainerName("INTTRAWHIT_" + felix);
      inttcalib->SetHotMapCdbFile(hotmapfilename);
      inttcalib->SetBcoMapCdbFile(bcomapfilename);
      inttcalib->SetOneFelixServer(std::stoi(felix));
      inttcalib->SetDoFeebyFee(false);
      inttcalib->SetStreamingMode(isInttStreaming);
      se->registerSubsystem(inttcalib);
    }
  }

  SyncReco *sync = new SyncReco();
  se->registerSubsystem(sync);

  HeadReco *head = new HeadReco();
  se->registerSubsystem(head);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);


  char outfile[500];
  sprintf(outfile,"./%s.root",type.c_str());

  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("out",outfile);
  out->UseFileRule();
  out->SetEventNumberRollover(neventsper); // event number for rollover
  // out->SetNEvents(neventsper);                       // number of events per output file
  out->SetClosingScript("./stageout.sh");      // script to call on file close (not quite working yet...)
  // out->SetClosingScriptArgs(outdir + " " + "0");  // with dbid
  out->SetClosingScriptArgs(outdir);  // additional beyond the name of the file
  se->registerOutputManager(out);

  if (nEvents < 0)
  {
    return;
  }
  se->run(nEvents);

  se->End();

  char histoutfile[500];
  sprintf(histoutfile,"./HIST_%s-%08i-%05i.root",type.c_str(),runnumber,0);
  QAHistManagerDef::saveQARootFile(histoutfile);

  delete se;
  cout << "all done" << endl;
  gSystem->Exit(0);
}

bool isGood(const string &infile)
{
  ifstream intest;
  intest.open(infile);
  bool goodfile = false;
  if (intest.is_open())
  {
    if (intest.peek() != std::ifstream::traits_type::eof()) // is it non zero?
    {
      goodfile = true;
    }
    intest.close();
  }
  return goodfile;
}

int getrunnumber(const std::string &listfile)
{
  if (! isGood(listfile))
  {
    std::cout << "listfile " << listfile << " is bad" << std::endl;
    gSystem->Exit(1);
  }
  std::ifstream ifs(listfile);
  std::string filepath;
  std::getline(ifs, filepath);

  pair<int, int> runseg = Fun4AllUtils::GetRunSegment(filepath);
  int runnumber = runseg.first;
//  int segment = abs(runseg.second);
  return runnumber;
}
