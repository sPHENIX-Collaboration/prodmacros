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

ls -la *.list
echo end of ls -la '*.list'

### Stage input to local
# for fullinfile in `cat infile_paths.list`; do
#     cp -v $fullinfile .
# done

outfile=${logbase}.root
outhist=${outfile/DST_CALOFITTING/HIST_CALOFITTINGQA}
root_line="Fun4All_Year2_Fitting.C(${nevents},\"infile.list\",\"${outfile}\",\"${outhist}\",\"${dbtag}\")"
full_command="root.exe -q -b '${root_line}'"
eval "${full_command}"

for hfile in HIST_*.root; do
    echo stageout.sh ${hfile} to ${histdir}
    ./stageout.sh ${hfile} ${histdir}
done

echo ./stageout.sh ${logbase}.root ${outdir} ${dbid}
./stageout.sh ${logbase}.root ${outdir} ${dbid}



ls -la

echo done
exit ${status_f4a:-1}

# status_f4a=0

# for infile_ in ${inputs[@]}; do
#     outfile=${logbase}.root
#     outhist=${outfile/DST_CALOFITTING/HIST_CALOFITTINGQA}
# ./cups.py -r ${runnumber} -s ${segment} -d ${outbase} running
    
#     root.exe -q -b Fun4All_Year2_Fitting.C\(${nevents},\"${infile}\",\"${outfile}\",\"${outhist}\",\"${dbtag}\"\);  status_f4a=$?

#     nevents=${nevents_:--1}
#     echo Stageout ${outfile} to ${outdir}
#         ./stageout.sh ${outfile} ${outdir}
 
#     for hfile in `ls HIST_*.root`; do
# 	echo Stageout ${hfile} to ${histdir}
#         ./stageout.sh ${hfile} ${histdir}
#     done

# done
    

# exit ${status_f4a}











dstname=${logbase%%-*}

out0=${logbase}.root
out1=HIST_${logbase#DST_}.root

nevents=-1
status_f4a=0

for infile_ in ${inputs[@]}; do

#   infile=$( basename ${infile_} )
#   cp -v ${infile_} .
    infile=$infile_
    
    outfile=${logbase}.root
    outhist=${outfile/DST_CALOFITTING/HIST_CALOFITTINGQA}
./cups.py -r ${runnumber} -s ${segment} -d ${outbase} running
    
    root.exe -q -b Fun4All_Year2_Fitting.C\(${nevents},\"${infile}\",\"${outfile}\",\"${outhist}\",\"${dbtag}\"\);  status_f4a=$?

    nevents=${nevents_:--1}
    echo Stageout ${outfile} to ${outdir}
        ./stageout.sh ${outfile} ${outdir}
 
    for hfile in `ls HIST_*.root`; do
	echo Stageout ${hfile} to ${histdir}
        ./stageout.sh ${hfile} ${histdir}
    done

done

ls -lah

#______________________________________________________________________________________ finished __
echo ./cups.py -v -r ${runnumber} -s ${segment} -d ${outbase} finished -e ${status_f4a} --nevents ${nevents} --inc 
     ./cups.py -v -r ${runnumber} -s ${segment} -d ${outbase} finished -e ${status_f4a} --nevents ${nevents} --inc 
#_________________________________________________________________________________________________



echo "bdee bdee bdee, That's All Folks!"

} > ${logdir#file:/}/${logbase}.out 2> ${logdir#file:/}/${logbase}.err

if [ -e cups.stat ]; then
    cp cups.stat ${logdir#file:/}/${logbase}.dbstat
fi


exit ${status_f4a}
