#!/usr/bin/bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"

echo "---------------------------------------------"
echo "Running Fitting (jobC) for run ${run}, seg {segment}"
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

ls -la *.list
echo end of ls -la '*.list'
### Stage input to local
# for infile in `cat infile_paths.list`; do
#     cp -v $infile .
# done

root_line="Fun4All_JobC.C(${nevents},\"${logbase}.root\",\"${dbtag}\",\"infile.list\")"
full_command="root.exe -q -b '${root_line}'"

echo "--- Executing macro"
echo "${full_command}"
eval "${full_command}" ;  status_f4a=$?

echo ./stageout.sh ${logbase}.root ${outdir} ${dbid}
./stageout.sh ${logbase}.root ${outdir} ${dbid}

for hfile in HIST_*.root; do
    echo stageout.sh ${hfile} to ${histdir}
    ./stageout.sh ${hfile} ${histdir}
done

ls -la

echo done
exit ${status_f4a:-1}
