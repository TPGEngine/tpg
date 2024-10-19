#!/bin/bash

#defaults
numAux=4
fitMode=0
maxt=0
modes=0
phs=0
winSize=1
wd=$(echo $PWD | rev | cut -d '/' -f 1 | rev)

while getopts a:f:g:m:p:T:w: flag
do
   case "${flag}" in
      a) numAux=${OPTARG};;
      f) fitMode=${OPTARG};;
      m) modes=${OPTARG};;
      p) phs=${OPTARG};;
      T) maxt=${OPTARG};;
      w) winSize=${OPTARG};;
   esac
done

numTask=$(grep "n_task" tpg.*.std | cut -d ' ' -f 2 | head -n 1)

c=1
#if ls *p${phs}*rslt 1> /dev/null 2>&1; then rm *p${phs}.rslt; fi
# if ls *.rslt 1> /dev/null 2>&1; then rm *.rslt; fi
maxT=$maxt
if [ $maxt -eq 0 ] 
then
   maxT=$(grep "gTime t " tpg*.std | awk -F" t " '{print $2}' | awk '{print $1}' | sort -n | tail -n 1 | tr -d '\n')


elif [ $maxt -eq -1 ]
then
   files=$(ls tpg*.std | grep -v replay)
   mt=""
   for f in $files; do
      mt="$(grep "gTime t " $f | awk -F" t " '{print $2}' | awk '{print $1}' | sort -n | tail -n 1 | tr -d '\n') $mt"
   done
   maxT=$(echo $mt | tr ' ' '\n' | sort -n | head -n 1)
fi

files=$(ls tpg*.std | sort -n -t '.' -k 3 | grep -v replay)

for f in $files
do
   echo "Processing $f ..."

   #run time stats
   for aux in `seq 0 $(echo "$numAux-1" | bc)`; do
      for task in `seq 0 $(echo "$numTask-1" | bc)`; do
         echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep " fm ${fitMode} " | grep " ss $task " | grep " phs $phs " | head -n $maxT | awk -F"mnOut " '{print $2}' | \
            awk -F"p${phs}t${task}a${aux} " '{print $2}' | awk '{print $1}' | tr '\n' ' ')  >> ${wd}_aux_${aux}_ST_${task}_p${phs}.rslt;
                     if [ $fitMode -eq 0 ] && [ $numTask -gt 1 ]; then
                        echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F"mnOut " '{print $2}' | \
                           awk -F"p${phs}t${task}a${aux} " '{print $2}' | awk '{print $1}' | tr '\n' ' ')  >> ${wd}_aux_${aux}_MTA_${task}_p${phs}.rslt;
                     fi 
                  done
               done

               echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA " | grep " phs $phs " | head -n $maxT | awk -F"minThr" '{print $2}' | \
                  awk '{print $1}' | tr '\n' ' ')  >> tpg-auxDouble_MTA-minThresh.rslt

   #wall time
   echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" sec " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-sec.rslt
   echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" evl " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-eval.rslt
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" gTms " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-genTeams.rslt
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" elTms " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-setEliteTeams.rslt
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" sTms " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-selTeams.rslt
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" rprt " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-rprt.rslt
   #
   #   #cumulative state
   #   echo $(tac $f | sed '/restart/q' | tac | grep "cpAFts " | head -n $maxT | awk -F"cpAFts " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-cpAFts.rslt
   #
   #echo $(grep "tToEvl " $f | head -n $maxT | awk -F" tToEvl " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-tToEvl.rslt

  for task in `seq 0 $(echo "$numTask-1" | bc)`; do
     echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep " fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F " nP " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-pCount-st-${task}.rslt
     echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep "fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F " nT " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-tCount-st-${task}.rslt
     echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep "fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F "age" '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-age-st-${task}.rslt
     #echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep "fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F "fit" '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-fit-st-${task}.rslt
  done

   # echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " nP " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-pCount-mt.rslt
   # echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " nT " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-tCount-mt.rslt
   echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} "  | grep " phs $phs " | head -n $maxT | awk -F "age" '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-age-mt.rslt   

   #echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnProgIns " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-meanPIns.rslt
   #echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnEProgIns " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-meanEPIns.rslt

#echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " pF " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-policyFeatures.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " pF " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-policyFeatures.rslt

#echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " lTypePr " '{print $2}' | awk '{print $1}') >> tpg-propType0.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnTmSzR " '{print $2}' | awk '{print $1}') >> tpg-tmSizeRoot.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnTmSzS " '{print $2}' | awk '{print $1}') >> tpg-tmSizeSub.rslt

echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" Msz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-Msize.rslt
echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" Lsz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-Lsize.rslt
echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" mSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-MemSize.rslt
echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" rSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-Rsize.rslt
echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" eLSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-eLSz.rslt

#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" Msz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-Msize.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" Lsz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-Lsize.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" mSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-MemSize.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" rSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-Rsize.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" mrSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-mRsize.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" nOldDelPr " '{print $2}' | awk '{print $1}') >> tpg-osr.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" sRTC " '{print $2}' | awk '{print $1}') >> tpg-sRTC.rslt
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" sGsz " '{print $2}' | awk '{print $1}') >> tpg-sGsz.rslt
echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" nDel " '{print $2}' | awk '{print $1}') >> tpg-nDel.rslt

   #MODES
   if [ $modes -gt 0 ]; then
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"change" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-change.rslt
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"novelty" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-novelty.rslt
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityRTC" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityRTC.rslt
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityTeams" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityTeams.rslt
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityPrograms" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityPrograms.rslt
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityInstructions" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityInstructions.rslt
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"ecology" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-ecology.rslt
   fi
done

echo $(tac $f | sed '/restart/q' | tac | grep archive | head -n $maxT | awk -F" sz " '{print $2}' | awk '{print $1}') >> tpg-archive-size.rslt


# plot #########################################################

i=0
if ls *p${phs}.pdf 1> /dev/null 2>&1; then rm *p${phs}.pdf; fi

##combined train+test
#if [ $phs -eq 2 ]; then
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves-paired.R ${wd}_aux_0_ST_0_p0.rslt ${wd}_aux_0_ST_0_p2.rslt "Mean Fitness" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
#i=$((i+1))
#fi

# for aux in `seq 0 $(echo "$numAux-1" | bc)`; do
#    for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#       i=$((i+1))
#       Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_ST_${task}_p${phs}.rslt "Aux ${aux} (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
#    done
# done

# fitness of best individual for each task
aux=0
for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_ST_${task}_p${phs}.rslt "Fitness (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

done

# teams per prediction of best individual for each task
aux=1
for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_ST_${task}_p${phs}.rslt "Teams/Prediction (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

done

# instructions per prediction of best individual for each task
aux=2
for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_ST_${task}_p${phs}.rslt "Instructions/Prediction (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

done

# # ? per prediction of best individual for each task
# aux=3
# for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#    i=$((i+1))
#    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_ST_${task}_p${phs}.rslt "MSE (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

# done



if [ $fitMode -eq 0 ] && [ $numTask -gt 1 ]; then
   # fitness of best individual for each task
   aux=0
   for task in `seq 0 $(echo "$numTask-1" | bc)`; do
      i=$((i+1))
      Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_MTA_${task}_p${phs}.rslt "Fitness (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   done
   
   # teams per prediction of best individual for each task
   aux=1
   for task in `seq 0 $(echo "$numTask-1" | bc)`; do
      i=$((i+1))
      Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_MTA_${task}_p${phs}.rslt "Teams/Prediction (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   done

   # instructions per prediction of best individual for each task
   aux=2
   for task in `seq 0 $(echo "$numTask-1" | bc)`; do
      i=$((i+1))
      Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_MTA_${task}_p${phs}.rslt "Instructions/Prediction (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   done

   # # ? per prediction of best individual for each task
   # aux=3
   # for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   #    i=$((i+1))
   #    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_MTA_${task}_p${phs}.rslt "MSE (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   # done
fi

# if [ $fitMode -eq 0 ] && [ $numTask -gt 1 ]; then
#    for aux in `seq 0 $(echo "$numAux-1" | bc)`; do
#       for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#          i=$((i+1))
#          Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}_aux_${aux}_MTA_${task}_p${phs}.rslt "Aux ${aux} (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
#       done
#    done
# fi

 for task in `seq 0 $(echo "$numTask-1" | bc)`; do
    i=$((i+1))
    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tCount-st-${task}.rslt "Teams per Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
 done
#  
 for task in `seq 0 $(echo "$numTask-1" | bc)`; do
    i=$((i+1))
    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-pCount-st-${task}.rslt "Programs per Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
 done
 
 for task in `seq 0 $(echo "$numTask-1" | bc)`; do
    i=$((i+1))
    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-age-st-${task}.rslt "Age of Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
 done

# for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#     i=$((i+1))
#     Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-fit-st-${task}.rslt "Fitness of Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
# done

# i=$((i+1))
# Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tCount-mt.rslt "Teams per Graph (best multi-task ${task})" "$winSize" "$(printf "%03d" $i)" 0
# i=$((i+1))
# Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-pCount-mt.rslt "Programs per Graph (best multi-task ${task})" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-age-mt.rslt "Age of Graph (best multi-task ${task})" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-auxDouble_MTA-minThresh.rslt "minThreshold" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-sRTC.rslt "Population-wide Instructions Executed" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-sGsz.rslt "Population-wide Teams Executed" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-nDel.rslt "# teams deleted" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-sec.rslt "Seconds Total" "$winSize" "$(printf "%03d" $i)" 0 
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-eval.rslt "Seconds Evaluation " "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-genTeams.rslt "Seconds Replacement" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-setEliteTeams.rslt "Seconds Set Elite Teams" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-selTeams.rslt "Seconds Selection" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-rprt.rslt "Seconds Accounting & Reporting" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-tToEvl.rslt "Teams to Evaluate" "$winSize" "$(printf "%03d" $i)" 0
#  #i=$((i+1))
#  #Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-propType0.rslt "Proportion of Action-Value Programs (best graph)" "$winSize" "$(printf "%03d" $i)" 1 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-osr.rslt "Offspring Survival Rate (numOldDeleted/numDeleted)" "$winSize" "$(printf "%03d" $i)" 0 
#  #i=$((i+1))
#  #Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-avp.rslt "Proportion Action-Value Programs in Population" "$winSize" "$(printf "%03d" $i)" 0

if [ $modes -gt 0 ]; then
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-change.rslt "MODES - Change" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-novelty.rslt "MODES - Novelty" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityRTC.rslt "MODES - Complexity RTC" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityTeams.rslt "MODES - Complexity Teams" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityPrograms.rslt "MODES - Complexity Programs" 5 "$(printf "%03d" $i)" 0
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityInstructions.rslt "MODES - Complexity Instruction" 5 "$(printf "%03d" $i)" 0
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-ecology.rslt "MODES - Ecology" 5 "$(printf "%03d" $i)" 0 
fi

#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-meanPIns.rslt "Mean Instructions per Program (best graph)" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-meanEPIns.rslt "Mean Effective Instructions per Program (best graph)" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-policyFeatures.rslt "Features (best graph)" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-Msize.rslt "Team Population Size" "$winSize" "$(printf "%03d" $i)" 0 
i=$((i+1))
Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-Lsize.rslt "Program Population Size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-MemSize.rslt "Memory Population Size" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-Rsize.rslt "Root Population Size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-eLSz.rslt "Elite Teams " "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-Msize.rslt "st Team population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-Lsize.rslt "st Program population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-Rsize.rslt "st Rsize" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-mRsize.rslt "st mRoot population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-MemSize.rslt "st Memory population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tmSizeRoot.rslt "Mean Root Team Size" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tmSizeSub.rslt "Mean Sub Team Size" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-archive-size.rslt "Archive Size " "$winSize" "$(printf "%03d" $i)" 0



pdfunite 0*.pdf ${wd}_p${phs}.pdf
rm 0*.pdf
rm *rslt

