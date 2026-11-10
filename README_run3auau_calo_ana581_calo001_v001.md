# run3auau calo ana581_calo001_v001

Full calo chain with build ana.581, set up like the run2auau ana581 production:

| Step | Output triplet | Conditions tag |
|---|---|---|
| DST_TRIGGERED_EVENT (event combining) | `ana581_nocdbtag_v001` | none |
| DST_CALOFITTING (waveform fitting) | `ana581_pcdb001_v001` | `pcdb001` |
| QA histograms of the fitting step | `ana581_calo001_v001` | `calo001` |
| DST_JETCALO (jet skim) | `ana581_calo001_v001` | `calo001` |

Run list: `thisProd/runlist_run3auau_test_list.txt`, a copy of `/sphenix/u/sphnxpro/run3auau_test_list.txt`.

Jet skim thresholds (AntiKt_Tower_r03_Sub1 > 10 GeV, CLUSTERINFO_CEMC > 8 GeV) are still to be decided.
