#!/usr/bin/bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"

echo "---------------------------------------------"
echo "Running combined polyseeding for run ${run}, seg ${seg}"
echo "---------------------------------------------"
echo "--- Collecting input files"
echo "---------------------------------------------"
echo dataset=$dataset
echo dsttype=$dsttype
echo intriplet=$intriplet
echo run=$run
echo seg=$seg
echo "---------------------------------------------"

make_filelists="./create_full_filelist_run_seg.py $dataset $intriplet $dsttype $run $seg"
echo "$make_filelists"
eval "$make_filelists"
. ${SPHENIXPROD_SCRIPT_PATH}/stagein.sh

inputfile=$(sed -n '1p' infile.list)
if [ -z "${inputfile}" ]; then
    echo "ERROR: infile.list is empty. Cannot run combined polyseeding."
    status_f4a=20
    . ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
fi

root_line="Fun4All_PolySeeding_Combined.C(${nevents},\"${inputfile}\",\"${logbase}.root\",\"${dbtag}\")"
full_command="root.exe -q -b '${root_line}'"

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh

echo ./stageout.sh ${logbase}.root ${outdir} ${dbid} --use-cp
. ./stageout.sh ${logbase}.root ${outdir} ${dbid} --use-cp

shopt -s nullglob
for hfile in HIST_*.root; do
    echo stageout.sh ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir} --use-cp
done
shopt -u nullglob

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
