#!/usr/bin/bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"

echo "---------------------------------------------"
echo "Running seeding (jobA) for run ${run}, seg {segment}"
echo "---------------------------------------------"
echo "--- Collecting input files"
echo "---------------------------------------------"
echo "--- Collecting input files"
echo dataset=$dataset
echo dsttype=$dsttype
echo intriplet=$intriplet
echo run=$run
echo seg=$seg
echo "---------------------------------------------"

make_filelists="./create_full_filelist_run_seg.py $dataset $intriplet $dsttype $run $seg"
echo "$make_filelists"
eval "$make_filelists"
. ${SPHENIXPROD_SCRIPT_PATH}/stagein.sh --checkonly

root_line="Fun4All_JobA.C(${nevents},\"${logbase}.root\",\"${dbtag}\",\"infile.list\")"
full_command="root.exe -q -b '${root_line}'"

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh

echo ./stageout.sh ${logbase}.root ${outdir} ${dbid}
. ./stageout.sh ${logbase}.root ${outdir} ${dbid}

for hfile in HIST_*.root; do
    echo stageout.sh ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir}
done

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh

