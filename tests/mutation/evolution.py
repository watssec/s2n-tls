import subprocess
import glob
import json
import shlex
import os  
import random
import re 

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


def target_type_selection(selected_seed):
    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)
    selected_seed_info = random_pool_data[selected_seed-1]
    mutation_type = selected_seed_info["mutation_type"]
    binop_rules_file = open("./mutation_rules/Binop_rules.json")
    binop_rules = json.load(binop_rules_file)
    binop_rules_group = list(binop_rules.keys()) 
    opcode = selected_seed_info["opcode"]
    if mutation_type == "Binop":
        for key in binop_rules_group:
            if opcode in binop_rules[key]:
                mutation_selection = random.choice(binop_rules[key])
                while mutation_selection == opcode :
                    mutation_selection = random.choice(binop_rules[key])
    return mutation_selection

    
def mutation_match(selected_seed):    
    #Match the seed number with seed location
    selected_seed_info = random_pool_data[selected_seed-1]
    file_name = selected_seed_info["file_name"]
    function_num = selected_seed_info["function_num"]
    instruction_num = selected_seed_info["instruction_num"]
    mutation_type = selected_seed_info["mutation_type"]
    command = "opt -load " + mutation_pass_dir + mutation_type + " -"+ mutation_type + " -file_name " + file_name + \
    "-function_num " + str(function_num) + "-instruction_num " + str(instruction_num)   + " ./all_llvm.bc"  + " > /dev/null"
    os.system(command)
    

# This function finds the mutation seed in the dataset with the largest grade
def database_seed_selection(database_json, mutation_point_list):
    max_grade = 0
    max_grade_label = 0
   
    for i in database_json:
        if i["label"] in mutation_point_list:
            continue
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
            print(cnt)
            selection_list.append(cnt)
    
    selected_label = random.choice(selection_list)

    return selected_label

def test_result(round):
    flag = False
    # Try to find match the proof fail message in the txt file.
    with open("./log/"+str(round)+ ".log") as log_file:
        content = log_file.read()
        result = re.search(content, "proof failed")
        if result == None:
            flag = False
        else:
            flag = True
    return flag

def error_message_extraction():
    return ""

def reward(mutation_point_list, test_result):

    # process the redicted information to edit the history file 
    with open(history_file_path) as history_file:
        history_json = json.load(history_file)
        # construct the single history_json
        temp_history_json = {}
        temp_history_json["mutaion_point_list"] = mutation_point_list
        if test_result:
            temp_history_json["error_message"] = ""
        else:
            temp_history_json["error_message"]  = error_message_extraction() 

def cluster_match():
    pass
def test(round):
    # TODO: Even if the mutation unit is chosen to be all_llvm.bc, 
    # The link from .saw to .bc can still be found to make the testing process faster

    # saw $< | tee $@
    # Currently achieved by saw every saw file in the dir, under the condition that 
    # functions that belong to one cluster might be dependent on another
    
    # redirect the information from the terminal to logfile
    
    for saw_file in glob.glob("*.saw"):
        os.system("saw "+ saw_file + "| tee -a ./log/"+ str(round) + ".log")
    return test_result(round)


# This function is for one step in a branch in the evolution mutation
# The logic is: if not empty, select the seed with the largest grade value
# if empty, select the seed form random seed pool

def one_mutation_round(database_json, mutation_point_list):

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)
    
    # check if the database is empty or not
    if database_json == []:
        database_json.append(database_empty)

    # select the seed with the largest grade from the dataset
    # [] can be repeatedly selected, while others can not
    selected_seed = database_seed_selection(database_json, mutation_point_list)

    # if the selected seed is [], then take one step further to 
    # randomly select one
    if selected_seed == 0:
        selected_seed = random_select_from_pool(random_pool_data, database_json)

        # add the selected_seed into the database
        database_json.append({"label":selected_seed, "grade": 10})
    return (database_json,  selected_seed)



def llvm_link():
    bc_file_list = []
    bc_file_command = ""
    for bc_file in glob.glob(bc_dir+"*.bc"):
        bc_file_list.append(bc_file)
        bc_file_command = bc_file_command + bc_file + " "
    os.system("llvm-link-3.9 -o ./bitcode/all_llvm.bc "+bc_file_command)

'''
Copy .saw file under tests/saw and copy spec dir under tests/saw/spec
'''
if __name__ == '__main__':

    # Open history json, if that does not exist, create an empty one

    for saw_file in glob.glob(saw_file_dir+"*.saw"):
        os.system("cp " + saw_file_dir + saw_file + " .")

    os.system("cp -r "+ saw_file_dir+ "bike_r1/ ./bike_r1" )
    os.system("cp -r "+ saw_file_dir+ "bike_r2/ ./bike_r2" )
    os.system("cp -r "+ saw_file_dir+ "failure_tests/ ./failure_tests" )
    #os.system("cp -r "+ saw_file_dir+ "HMAC/ ./HMAC" )
    os.system("cp -r "+ saw_file_dir+ "sike_r1/ ./sike_r1" )

    os.system("cp -r "+ saw_file_dir+ "spec/ ./spec" )


    # Makefile is edited to the steps before llvm link
    make_process = subprocess.Popen("make") 
    stdout, stderr = make_process.communicate()
    llvm_link()
    # invoke the initialization pass
    init_command = "opt -load " + Initialization_pass_dir + " -Initialization < " +  "./bitcode/all_llvm.bc" + " > /dev/null"

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)

    # Normal labels start from 1, 0 represents []
    

    #Initialize database, if the file is empty, then set it as 
    if not os.path.isfile(database_file_path):
        database_json = []
    else:    
        with open(database_file_path) as database_file:
            database_json = json.load(database_file)
   
    # One round stops at a bad seed
 
    for round in range(10):
        test_result = True
        
        mutation_point_list = []
        while test_result == True:
            (database_json,  selected_seed) = one_mutation_round(database_json, mutation_point_list)
            # write into file the new database_json
            with open(database_file_path, "w") as database_file:
                json.dump(database_json, database_file)

            mutation_point_list.append(selected_seed)

            test_result = test(round)
            reward(mutation_point_list,test_result)

        # link the file again to get the renewed all_llvm.bc

        llvm_link()
        # after selected, goto the test step
        # with the error messages collected from the test step, goto the reward step
        # if this path is not rewarded, then end it
        # else if this path is rewarded, continue it 
        

    # select out the mutation point to be mutated

    saw_file_dir = "../saw"

    #txtfiles = []
    #for file in glob.glob(saw_file_dir + "*.saw"):
    #    txtfiles.append(file)






