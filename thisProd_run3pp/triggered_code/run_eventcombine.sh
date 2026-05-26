#!/usr/bin/env bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"
##

echo "---------------------------------------------"
echo "Running eventcombine for run ${run} on ${daqhost}"
echo "---------------------------------------------"
echo "--- Collecting input files"
make_filelists="./create_filelist_run_daqhost.py $run $daqhost $inputs"
echo "$make_filelists"
eval "$make_filelists"
. ${SPHENIXPROD_SCRIPT_PATH}/stagein.sh --checkonly

echo "--- Executing macro"

root_line="Fun4All_Prdf_Combiner.C(${nevents},\"${daqhost}\",\"${outbase}\",\"${outdir}\")"
full_command="root.exe -q -b '${root_line}'"

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh

shopt -s nullglob
for hfile in HIST_*.root; do
    echo ./stageout ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir}
done

for hfile in CALIB_*.root; do
    echo ./stageout ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir}
done
shopt -u nullglob

# There should be no output files hanging around  (TODO add number of root files to exit code)
ls -la

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh

