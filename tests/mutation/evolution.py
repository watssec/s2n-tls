import subprocess
import glob
import json
import shlex
import os  
import random

Initialization_pass_dir = "../../mutation/passes/Initialization/build/Initialization/libInitialization.so"
saw_file_dir = "../saw/"
bc_dir = "./bitcode/"
database_file_path = "database.json"
history_file_path = "history.json"
mutation_pass_dir = "../../mutation/passes/"

mutation_round = 100
# Initialize the database json with 0, initialized mark == 10
database_json = []
database_empty = {"label": 0, "grade": 10}

'''
database_file 

function_name {{point_label,}}
'''

# TODO: Create history database
# Match the mutation type then construct the argument to pass into the mutation pass
def mutation_match(selected_seed):    
    #Match the seed number with seed location

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)

    selected_seed_info = random_pool_data[selected_seed-1]
    print(selected_seed_info)
    file_name = selected_seed_info["file_name"]
    function_num = selected_seed_info["function_num"]
    instruction_num = selected_seed_info["instruction_num"]
    mutation_type = selected_seed_info["mutation_type"]
    command = "opt -load " + mutation_pass_dir + mutation_type + " -"+ mutation_type + " -file_name " + file_name + \
    "-function_num " + str(function_num) + "-instruction_num " + str(instruction_num)   + " ./all_llvm.bc"  + " > /dev/null"
    print(command)
    os.system(command)
    

# This function finds the mutation seed in the dataset with the largest grade
def database_seed_selection(database_json):
    max_grade = 0
    max_grade_label = 0
   
    for i in database_json:
        if i["grade"] > max_grade:
            max_grade = i["grade"]
            max_grade_label = i["label"]
    return max_grade_label

# This function randomly pick one mutation point from random pool
def random_select_from_pool(random_pool_data, database_json):
    selected_list = []
    for i in database_json:
        selected_list.append(i["label"])
    selection_list = []
    cnt = 1
    for i in random_pool_data:
        cnt = cnt + 1 
        if cnt not in selected_list:
            selection_list.append(cnt)

    selected_label = random.choice(selection_list)

    return selected_label

def reward():
     # process the redicted information
     pass

def test(selected_seed, round):
    # TODO: Even if the mutation unit is chosen to be all_llvm.bc, 
    # The link from .saw to .bc can still be found to make the testing process faster

    # saw $< | tee $@
    # Currently achieved by saw every saw file in the dir, under the condition that 
    # functions that belong to one cluster might be dependent on another
    
    # redirect the information from the terminal to logfile
    
    for saw_file in glob.glob("*.saw"):
        os.system("saw "+ saw_file + "| tee ./log/"+ str(selected_seed) + "_" + str(round) + ".log")
    


def one_mutation_round(database_json):

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)

    print("in one_mutation_round")
    print(database_json)
        # check if the database is empty or not
    if database_json == []:
        print("append")
        database_json.append(database_empty)

    # select the seed with the largest grade from the dataset
    # [] can be repeatedly selected, while others can not
    
    selected_seed = database_seed_selection(database_json)
    # if the selected seed is [], then take one step further to 
    # randomly select one
    if selected_seed == 0:
        selected_seed = random_select_from_pool(random_pool_data, database_json)
        # add the selected_seed into the database
        database_json.append({"label":selected_seed, "grade": 10})
    return (database_json,  selected_seed)

# Makefile is edited to the steps before llvm link
#make_process = subprocess.Popen("make")
#stdout, stderr = make_process.communicate()
# invoke the initialization pass

'''
Copy .saw file under tests/saw and copy spec dir under tests/saw/spec
'''
if __name__ == '__main__':

    # Open history json, if that does not exist, create an empty one

    for saw_file in glob.glob(saw_file_dir+"*.saw"):
        os.system("cp " + saw_file_dir + saw_file + " .")

    os.system("cp -r "+ saw_file_dir+ "spec/ ./spec" )

    for bc_file in glob.glob(bc_dir+"*.bc"):
        
        command = "opt -load " + Initialization_pass_dir + " -Initialization < " +  bc_file + " > /dev/null"
        if bc_file == "all_llvm.bc":
            continue
        print(command)
        #os.system(command)
        
    # llvm link

    bc_file_list = []
    bc_file_command = ""
    for bc_file in glob.glob(bc_dir+"*.bc"):
        bc_file_list.append(bc_file)
        bc_file_command = bc_file_command + bc_file + " "

    os.system("llvm-link-3.9 -o ./bitcode/all_llvm.bc "+bc_file_command)

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)

    # read the genesis_info.json file, temporarily grouped by function
    # It should be changed to other rules later
    # Normal labels start from 1, 0 represents []
    

    #Initialize database, if the file is empty, then set it as 
    if not os.path.isfile(database_file_path):
        database_json = []
    else:    
        with open(database_file_path) as database_file:
            database_json = json.load(database_file)
   
    # One round stops at a bad seed
 
    for round in range(1):
        test_result = True
        
        mutation_point_list = []
        while test_result == True:
            (database_json,  selected_seed) = one_mutation_round(database_json)
            # write into file the new database_json
            with open(database_file_path, "w") as database_file:
                json.dump(database_json, database_file)

            mutation_point_list.append(selected_seed)
            print(selected_seed)
            # mutation passes
            mutation_match(selected_seed)
        
            test_result = test(selected_seed, round)
            # Match the mutation point then call the according pass with the command line argument
            

        # after selected, goto the test step
        # with the error messages collected from the test step, goto the reward step
        # if this path is not rewarded, then end it
        # else if this path is rewarded, continue it 
        

    # select out the mutation point to be mutated

    saw_file_dir = "../saw"

    #txtfiles = []
    #for file in glob.glob(saw_file_dir + "*.saw"):
    #    txtfiles.append(file)






