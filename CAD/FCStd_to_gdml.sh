# Read the first line of the file into a variable
freecaddir_executable=$(head -n 1 freecadcmd_dir)

freecaddir_executable+="/freecadcmd"
freecaddir_executable+=" /home/pemb7000/eic/epic/CAD/refine_step_for_ePIC.py "
#the first argument is the
freecaddir_executable+=$1 

# Execute the command stored in the variable
eval "$freecaddir_executable"  
source remove_whitespace.sh
source convert.sh
source convert.sh