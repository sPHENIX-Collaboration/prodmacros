#!/usr/bin/env bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"
##

echo "---------------------------------------------"
echo "Running HCalCosmics for run ${run}, seg ${seg}"
echo "---------------------------------------------"
echo "--- Environment variables"
echo dataset=$dataset
echo dsttype=$dsttype
echo intriplet=$intriplet
echo run=$run
echo seg=$seg
echo dbtag=$dbtag
echo "---------------------------------------------"

echo "--- Collecting input files"
make_filelists="./create_full_filelist_run_seg.py $dataset $intriplet $dsttype $run $seg"
echo "$make_filelists"
eval "$make_filelists"

ls -la *.list
echo end of ls -la '*.list'

# Output file names
# DST output (only produced for runs <= 66455 inside the macro)
outfile=${logbase}.root

# Histogram output file
outhist=${outfile/DST_CALOFITTING/HIST_CALOFITTINGQA}

echo "--- Output files"
echo "DST output: ${outfile}"
echo "QA hist: ${outhist}"

root_line="Fun4All_Hcal_Cosmics.C(${nevents},\"infile.list\",\"${outfile}\",\"${outhist}\",\"${dbtag}\")"
full_command="root.exe -q -b '${root_line}'"

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh

echo "--- Staging out histogram files"
shopt -s nullglob
for hfile in HIST_*.root; do
    echo ./stageout.sh ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir}
done
shopt -u nullglob

# Stage out DST if it exists
if [ -f "${outfile}" ]; then
    echo "--- Staging out DST file"
    echo ./stageout.sh ${outfile} ${outdir} ${dbid}
    . ./stageout.sh ${outfile} ${outdir} ${dbid}
else
    echo "--- No DST output"
fi

ls -la

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
