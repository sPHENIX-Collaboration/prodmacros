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
root_line="Fun4All_Fitting.C(${nevents},\"infile.list\",\"${outfile}\",\"${outzdc}\")"
full_command="root.exe -q -b '${root_line}'"

# Production step. A failure here ends the job with the root.exe exit code.
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_exec.sh

# Stage out the production output before QA runs, so a QA problem can never cost it.
# --keep-local (stageout.sh in sphenixprod): copy out as usual, but keep the local
# file, because the QA step below reads it.
echo ./stageout.sh ${outfile} ${outdir} ${dbid} --keep-local
. ./stageout.sh ${outfile} ${outdir} ${dbid} --keep-local

echo ./stageout.sh ${outzdc} ${zdcoutdir:-${outdir}} ${zdcdbid:-${dbid}}
. ./stageout.sh ${outzdc} ${zdcoutdir:-${outdir}} ${zdcdbid:-${dbid}}

# Optional QA step. It does not go through common_runscript_exec.sh, so its exit code
# and resource use never replace those of the production step. A failure is logged,
# its partial histogram file is removed, and the job continues.
outhist=${outfile/DST_CALOFITTING/HIST_CALOFITTINGQA}
qa_line="Fun4All_Fitting_QA.C(${nevents},\"infile.list\",\"${outfile}\",\"${outhist}\",\"${dbtag}\")"
echo "--- Executing optional QA macro"
echo "root.exe -q -b '${qa_line}'"
root.exe -q -b "${qa_line}"
qa_status=$?
if [[ ${qa_status} -ne 0 ]]; then
    echo "WARNING: QA macro exited with code ${qa_status}. No QA histograms staged out; production output is unaffected."
    rm -f "${outhist}"
fi
echo "QA_STATUS=${qa_status}"

shopt -s nullglob
for hfile in HIST_*.root; do
    echo stageout.sh ${hfile} to ${histdir}
    . ./stageout.sh ${hfile} ${histdir}
done
shopt -u nullglob

# The production file was kept for QA; it is already staged out.
rm -fv "${outfile}"

ls -la

echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_finish.sh

