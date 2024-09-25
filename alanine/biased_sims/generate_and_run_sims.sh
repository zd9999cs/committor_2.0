#!/bin/bash

# source programs
source ~/Bin/dev/gromacs-plumed2-dev/sourceme.sh
export OMP_NUM_THREADS=2

# parse iteration number
iter=$1

# define core offset and number
CORE_NUM=2
CORE_OFFSET=0
NSTEPS=$[500*1000*10] #last is ns


FOLDER_NAME="iter_${iter}" 
rm -r $FOLDER_NAME
echo folder $FOLDER_NAME

# create folder from template (need inputs, plumed.dat and model)
cp -r template $FOLDER_NAME
# move to folder
cd $FOLDER_NAME

# by default we run from A and B
mkdir A
mkdir B


# modify model name in plumed.dat
sed -i "s/model_0_z.pt/model_${iter}_z.pt/g" plumed.dat
# sed -i "s/model_0_z.pt/model_${iter}_q.pt/g" plumed.dat


# cd A + run sim A
cd A
mpiexec -n 1 gmx_mpi mdrun -s ../input.sA.tpr -nsteps $NSTEPS -cpi state.cpt -plumed ../plumed.dat -gpu_id 0 -pin on -pinoffset $CORE_OFFSET &
wait 1
cd ..
CORE_OFFSET=$(($CORE_OFFSET+$CORE_NUM+1))



# cd B + run sim B
cd B
mpiexec -n 1 gmx_mpi mdrun -s ../input.sB.tpr -nsteps $NSTEPS -cpi state.cpt -plumed ../plumed.dat -gpu_id 0 -pin on -pinoffset $CORE_OFFSET &
cd ..
CORE_OFFSET=$(($CORE_OFFSET+$CORE_NUM+1))

cd ..

