#!/usr/bin/env bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"
##

echo "Running CALOFITTING for run ${run_number}, seg {segment}"
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

### Stage input to local
# for fullinfile in `cat infile_paths.list`; do
#     cp -v $fullinfile .
# done

outfile=${logbase}.root
outzdc=${logbase/DST_CALOFITTING/DST_ZDC_RAW}.root
outsepd=${logbase/DST_CALOFITTING/DST_SEPD_RAW}.root
outhist=${outfile/DST_CALOFITTING/HIST_CALOFITTINGQA}
root_line="Fun4All_Year2_Fitting.C(${nevents},\"infile.list\",\"${outfile}\",\"${outzdc}\",\"${outsepd}\",\"${outhist}\",\"${dbtag}\")"
full_command="root.exe -q -b '${root_line}'"

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh

for hfile in HIST_*.root; do
    echo stageout.sh ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir}
done

echo ./stageout.sh ${logbase}.root ${outdir} ${dbid}
. ./stageout.sh ${logbase}.root ${outdir} ${dbid}

echo ./stageout.sh ${outzdc} ${zdcoutdir:-${outdir}} ${zdcdbid:-${dbid}}
. ./stageout.sh ${outzdc} ${zdcoutdir:-${outdir}} ${zdcdbid:-${dbid}}

ls -la

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh

