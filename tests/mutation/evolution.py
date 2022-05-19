import subprocess
import glob
import json
import shlex
import os  
import random

Initialization_pass_dir = "../../mutation/passes/Initialization/build/Initialization/libInitialization.so"
bc_dir = "./bitcode/"
database_file = "database.json"
history_file = "history.json"
json_file = open("genesis_info.json")
random_pool_data = json.load(json_file)
mutation_round = 100
'''
database_file 

function_name {{point_label,}}
'''

# This function finds the mutation seed in the dataset with the largest grade
def database_seed_selection(database_json):
    max_grade = 0
    max_grade_label = 0
    for i in database_json:
        if i["grade"] > max_grade:
            max_grade = i["grade"]
            max_grade_label = i["label"]
# This function randomly pick one mutation point from random pool
def random_select_from_pool(random_pool_data, database_json):
    selected_list = []
    for i in database_json:
        selected_list.append(i["label"])
    selection_list = []
    cnt = 1
    for i in random_pool_data:
        if cnt not in selected_list:
            selection_list.append(cnt)
    random.choice(selection_list)

def reward():
    pass
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

# read the genesis_info.json file, temporarily grouped by function
# It should be changed to other rules later
# Normal labels start from 1, 0 represents []
 
# Initialize the database json with 0, initialized mark == 10
database_json = []
database_empty = {"label": 0, "grade": 10}


for round in range(mutation_round):
    # check if the database is empty or not
    if database_json == []:
        database_json.append(database_empty)

    # select the seed with the largest grade from the dataset
    # [] can be repeatedly selected, while others can not
    
    selected_seed = database_seed_selection(database_json)
    # if the selected seed is [], then randomly select
    if selected_seed == 0:
        random_select_from_pool()
    # after selected, goto the test step
    # with the error messages collected from the test step, goto the reward step
    # if this path is not rewarded, then end it
    # else if this path is rewarded, continue it 
     

for i in random_pool_data:
    print(i)


# select out the mutation point to be mutated

saw_file_dir = "../saw"

#txtfiles = []
#for file in glob.glob(saw_file_dir + "*.saw"):
#    txtfiles.append(file)






