#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4allraw/SingleGl1TriggeredInput.h>
#include <fun4allraw/Fun4AllTriggeredInputManager.h>
#include <fun4allraw/InputManagerType.h>
#include <fun4all/Fun4AllOutputManager.h>

#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>

#include <ffarawmodules/ClockDiffCheck.h>

#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TString.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libfun4allraw.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libffarawmodules.so)

void Fun4All_Prdf_Combiner(int nEvents = 0,
			   const std::string &daqhost = "seb15",
			   const std::string &outbase = "delme",
			   const std::string &outdir = "/sphenix/data/data02/sphnxpro/scratch/kolja/test/"
  )
{

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);
  se->VerbosityDownscale(100000);
  Fun4AllTriggeredInputManager *in = new Fun4AllTriggeredInputManager("Tin");
  SingleTriggeredInput *gl1 = new SingleGl1TriggeredInput("Gl1in");
  gl1->KeepPackets();
  gl1->AddListFile("gl1daq.list");
  //  gl1->Verbosity(10);
  in->registerGl1TriggeredInput(gl1);

  SingleTriggeredInput *input = new SingleTriggeredInput(daqhost);
  if (daqhost == "seb18") {
    input->KeepPackets();
  }

  //  input->Verbosity(10);
  //  input->FakeProblemEvent(10);
  TSystemDirectory workdir("workdir",".");
  TList *listfiles = workdir.GetListOfFiles();
  TIter listnext(listfiles);
  while ( auto listfile = (TSystemFile*) listnext() ){
      TString fname = listfile->GetName();
      if ( !fname.EndsWith(".list") ) continue;
      if ( fname == "gl1daq.list") continue;
      if ( fname.Contains( daqhost ) ){
	ifstream infile;
	infile.open(fname.Data());
	std::cout << "Adding " << fname << std::endl;
	if (infile.is_open()){
	  infile.close();
	  input->AddListFile(fname.Data());
	  in->registerTriggeredInput(input);
	  break; // don't need to break; but we should probably sort if we use multiple input lists
	}
      }
    }
  se->registerInputManager(in);
  // In principle, more than one input file list can be supported
  // std::vector<TString> filenames; // collect names first so they can be ordered
  // std::sort(filenames.begin(), filenames.end());

  
//   ifstream infile;
//   SingleTriggeredInput *input = nullptr;
//   for (int i=0; i<21; i++)
//   {
//     char daqhost[200];
//     char daqlist[200];
//     sprintf(daqhost,"seb%02d",i);
//     sprintf(daqlist,"%s.list",daqhost);
//     infile.open(daqlist);
//     if (infile.is_open())
//     {
//       infile.close();
//       input = new SingleTriggeredInput(daqhost);
// //  input->Verbosity(10);
// //      input->FakeProblemEvent(10);
//       input->AddListFile(daqlist);
//       in->registerTriggeredInput(input);
//     }
//   }
//   se->registerInputManager(in);

  SyncReco *sync = new SyncReco();
  se->registerSubsystem(sync);

  HeadReco *head = new HeadReco();
  se->registerSubsystem(head);
  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  ClockDiffCheck *clkchk = new ClockDiffCheck();
//   clkchk->Verbosity(3);
  clkchk->set_delBadPkts(true);
  se->registerSubsystem(clkchk);
  // std::string outfile = "DST_TRIGGERED_EVENT_" + daqhost + "_run2pp_new_nocdbtag_v001.root";
  std::string outfile = outbase + ".root";
  Fun4AllOutputManager *out = new Fun4AllDstOutputManager("dstout",outfile);
  out->SplitLevel(0);
  out->UseFileRule();
  out->SetNEvents(100000); 
  // out->SetClosingScript("copyscript.pl");      // script to call on file close (not quite working yet...)
  // out->SetClosingScriptArgs(" -mv -outdir " + outdir);  // additional beyond the name of the file
  // out->SetClosingScript("/bin/echo");      // script to call on file close (not quite working yet...)
  // out->SetClosingScript("/usr/bin/mv");
  // out->SetClosingScriptArgs(" -v " + outdir);  // additional beyond the name of the file
  out->SetClosingScript("./stageout.sh");
  out->SetClosingScriptArgs(outdir + " " + "0");  // additional beyond the name of the file
  se->registerOutputManager(out);
  if (nEvents >= 0)
  {
    se->run(nEvents);
  }
  se->End();
  delete se;
  std::cout << "all done" << std::endl;
  gSystem->Exit(0);
}
