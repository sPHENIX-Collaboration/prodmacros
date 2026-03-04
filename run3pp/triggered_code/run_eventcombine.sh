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

ls -la *.list
echo end of ls -la '*.list'
for l in *list; do
    echo cat $l
    cat $l
done

echo "--- Executing macro"

root_line="Fun4All_SingleJob0.C(${nevents},${run},\"${logbase}.root\",\"${dbtag}\",\"infile.list\")"
root_line="Fun4All_Prdf_Combiner.C(${nevents},\"${daqhost}\",\"${outbase}\",\"${outdir}\")"
full_command="root.exe -q -b '${root_line}'"

echo "${full_command}"
eval "${full_command}" ;  status_f4a=$?

shopt -s nullglob
for hfile in HIST_*.root; do
    echo ./stageout ${hfile} to ${histdir}
    ./stageout.sh ${hfile} ${histdir}
done

for hfile in CALIB_*.root; do
    echo ./stageout ${hfile} to ${histdir}
    ./stageout.sh ${hfile} ${histdir}
done
shopt -u nullglob

# # Signal that the job is done
# destname=${outdir}/${logbase}.finished
# # change the destination filename the same way root files are treated for easy parsing
# destname="${destname}:nevents:0"
# destname="${destname}:first:-1"
# destname="${destname}:last:-1"
# destname="${destname}:md5:none"
# destname="${destname}:dbid:${dbid}"
# echo touch $destname
# touch $destname

# There should be no output files hanging around  (TODO add number of root files to exit code)
ls -la 

echo "All done"
exit ${status_f4a:-1}
