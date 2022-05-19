
import subprocess
import glob
import json
import shlex
import os  


Initialization_pass_dir = "../../mutation/passes/Initialization/build/Initialization/libInitialization.so"
bc_dir = "./bitcode/"

# Makefile is edited to the steps before llvm link
#make_process = subprocess.Popen("make")
#stdout, stderr = make_process.communicate()
# invoke the initialization pass

for bc_file in glob.glob(bc_dir+"*.bc"):
    
    command = "opt -load " + Initialization_pass_dir + " -Initialization < " +  bc_file + " > /dev/null"
    print(command)
    os.system(command)
    #stdout, stderr = sub_initialize_process.communicate()



# llvm link

# read the genesis_info.json file, group by function cluster



#subprocess.popen("")


# evolution logic here ...

# two important files: database & history
#
database = "database.json"
history = "history.json"

saw_file_dir = "../saw"

#txtfiles = []
#for file in glob.glob(saw_file_dir + "*.saw"):
#    txtfiles.append(file)






