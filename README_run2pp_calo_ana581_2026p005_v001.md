# run2pp calo ana581_2026p005_v001

Downstream steps only, on sphnxprod03. No event combining: DST_CALOFITTING reads the existing
`pro001_pcdb001_v001` DST_TRIGGERED_EVENT files and writes `ana581_pcdb001_v001` (QA histograms with
qadbtag `2026p005`, `ana581_2026p005_v001`). DST_JETCALO reads `ana581_pcdb001_v001` and writes
`ana581_newcdbtag_v001`.

Code is the run2auau calo code (prodmacros main 0868bb8, plus the run_jetskimmer.sh that calls
Fun4All_JetSkimmer.C), with the run2pp jet thresholds from pro001: AntiKt_Tower_r04_Sub1 > 7 GeV,
CLUSTERINFO_CEMC > 5 GeV.

Run list: `run2pp_1M_v1.list` from `run2pp_calo_pro001_pcdb001_v001/dir_run2pp_calo_pro001_pcdb001_v001`,
the list the pro001 run2pp pilot uses.
