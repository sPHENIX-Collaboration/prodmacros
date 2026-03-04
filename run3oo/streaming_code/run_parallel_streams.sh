#!/usr/bin/env bash

## Tedium common to all run scripts. Important, source, not execute!
echo Sourcing ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
. ${SPHENIXPROD_SCRIPT_PATH}/common_runscript_prep.sh
echo "Initialization done; back in $0"
##

echo "---------------------------------------------"
echo "Running streaming eventcombine for run ${run} on ${daqhost}"
echo "---------------------------------------------"
echo "--- Collecting input files"
make_filelists="./create_filelist_run_daqhost.py $run $daqhost $inputs"
echo "$make_filelists"
eval "$make_filelists"

# Should be exactly one gl1 file and one ebdc, mvtx, or intt file
# trying to be flexible here, but we have to assume daqhost will always be lowercase and in this family
# Bit of shell magic here, inspired by
# https://unix.stackexchange.com/questions/472668/how-to-easily-count-the-number-of-matches-of-a-glob-involving-paths-with-spaces
shopt -s nullglob
set -- *gl1*.list
if [[ $# != 1 ]] ; then
    echo "Multiple or no GL1 files found:" >&2
    ls -l `echo $@`                        >&2
    echo Stop.                             >&2
    # ./cups.py -v -r ${runnumber} -s ${segment} -d ${outbase} finished -e 111 --nevents 0 --inc 
    exit 111
fi
gl1file=$1

inttfile=""
mvtxfile=""
ebdcfile=""
tpotfile=""
ls -l | grep list
set -- `find . -maxdepth 1 -name \*.list -a -not -name $gl1file`
if [[ $# != 1 ]] ; then
    echo "Multiple or not enough .list files found:"     >&2
    ls -l `echo $@`                        >&2
    echo Stop.                             >&2
    # ./cups.py -v -r ${runnumber} -s ${segment} -d ${outbase} finished -e 111 --nevents 0 --inc 
    exit 111
fi
[[ $1 == *intt* ]] && inttfile=$1
[[ $1 == *mvtx* ]] && mvtxfile=$1
[[ $1 == *ebdc* ]] && ebdcfile=$1
[[ $1 == *ebdc39* ]] && ebdcfile="" && tpotfile=$1
shopt -u nullglob

ls -la *.list
echo end of ls -la '*.list'
for l in *list; do
    echo cat $l
    cat $l
done

echo "--- Executing macro"

root_line="Fun4All_SingleStream_Combiner.C(${nevents},${run},\"${outdir}\",\"${histdir}\",\"${outbase}\",${neventsper},\"${dbtag}\",\"${gl1file}\",\"${ebdcfile}\",\"${inttfile}\",\"${mvtxfile}\",\"${tpotfile}\");"
full_command="root.exe -q -b '${root_line}'"

echo "${full_command}"
eval "${full_command}" ;  status_f4a=$?

shopt -s nullglob
for hfile in HIST_*.root; do
    echo ./stageout ${hfile} to ${histdir}
    ./stageout.sh ${hfile} ${histdir}
done
for cfile in CALIB_*.root; do
    echo ./stageout ${cfile} to ${histdir}
    ./stageout.sh ${cfile} ${histdir}
done

shopt -u nullglob

# There should be no output files hanging around  (TODO add number of root files to exit code)
ls -la 

echo "All done"
exit ${status_f4a:-1}
