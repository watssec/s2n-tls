from operator import truediv
import subprocess
import glob
import json
import shlex
import os  
import random
import re 
from tqdm import tqdm

Initialization_pass_dir = "../../mutation/passes/Initialization/build/Initialization/libInitialization.so"
saw_file_dir = "../saw/"
bc_dir = "./bitcode/"
database_file_path = "database.json"
history_file_path = "history.json"
useless_file_path = "useless.json"
report_file_path = "report.json"

mutation_pass_dir = "../../mutation/passes/"

mutation_round = 100
# Initialize the database json with 0, initialized mark == 10
database_json = []
database_empty = {"label": [0], "grade": 10}

'''
database_file 

function_name {{point_label,}}
'''

# TODO: Create history database
# Match the mutation type then construct the argument to pass into the mutation pass



def mutation_match(selected_seed_list):

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)

    for selected_seed in selected_seed_list:    
        #Match the seed number with seed location
        selected_seed_info = random_pool_data[selected_seed-1]
        file_name = selected_seed_info["file_name"]
        function_num = selected_seed_info["function_num"]
        instruction_num = selected_seed_info["instruction_num"]
        mutation_type = selected_seed_info["mutation_type"]
        opcode = selected_seed_info["opcode"]
        mutation_selection = ""
        if mutation_type == "Binop":
            binop_rules_file = open("./mutation_rules/Binop_rules.json")
            binop_rules = json.load(binop_rules_file)
            binop_rules_group = list(binop_rules.keys()) 
            for key in binop_rules_group:
                if opcode in binop_rules[key]:
                    mutation_selection = random.choice(binop_rules[key])
                    while mutation_selection == opcode :
                        mutation_selection = random.choice(binop_rules[key])

            command = "opt -load " + mutation_pass_dir + mutation_type + "/build/Mutation/libMutation.so" + " -file_name " + file_name + \
            " -function_num " + str(function_num) + " -instruction_num " + str(instruction_num) +" -target_type " + mutation_selection + " ./bitcode/all_llvm.bc"  + " > /dev/null"
        else:
            command = "opt -load " + mutation_pass_dir + mutation_type + "/build/Mutation/libMutation.so" + " -file_name " + file_name + \
            " -function_num " + str(function_num) + " -instruction_num " + str(instruction_num) + " ./bitcode/all_llvm.bc"  + " > /dev/null"
        print(command)
        os.system(command)
        

# This function finds the mutation seed in the dataset with the largest grade
def database_seed_selection(database_json):
    # if the seed is a useless seed, then continue on this one
    max_grade = 0
    max_grade_label = []
   
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

def test_result_func(round):
    flag = False
    # Try to find match the proof fail message in the txt file.
    with open("./log/"+str(round) + ".log") as log_file:
        content = log_file.read()
        result = re.search(content, "proof failed")
        if result == None:
            flag = False
        else:
            flag = True
    print(flag)        
    return flag

def error_message_extraction(round):
    with open("./log/"+str(round)+".log") as log_file:
        content = log_file.read()

    return re.search(content,"failed: .*assertion$").group()
    
def new_error_message_check(history_json, current_error_message):
    error_message_collection = []
    for record in history_json:
        if record["error_message"] != "":
            error_message_collection.append(record["error_message"])
    if current_error_message not in error_message_collection:
        return True
    else: 
        return False

def error_elimination_check(history_json, seed, current_error_message):
    # check for the error that was eliminated by the seed
    seed_error_message = ""
    for record in history_json:
        if record["mutation_point_list"] == seed:
            seed_error_message = record["error_message"]
            break
    if current_error_message in seed_error_message:
        return True
    else:
        return False

def feedback(mutation_point_list, test_result, round):
    history_json = []
    seed = mutation_point_list[:-2]
    # process the redicted information to edit the history file test
    if os.stat(history_file_path).st_size == 0:
        history_json = []
    else:
        with open(history_file_path,"r") as history_file:
            history_json = json.load(history_file)
    
    # construct the single history_json
    temp_history_json = {}
    temp_history_json["round"] = round
    temp_history_json["mutaion_point_list"] = mutation_point_list
    current_error_message = ""
    if not test_result:
        temp_history_json["error_message"] = ""
    else:
        current_error_message = error_message_extraction(round) 
        temp_history_json["error_message"]  = current_error_message 

    add_count = 0
    if not test_result:
        json_file = open("genesis_info.json")
        random_pool_data = json.load(json_file)
        temp_report = {}
        for i in selected_seed_list:
            selected_seed_info = random_pool_data[i-1]
            temp_report[i] = selected_seed_info
        report_json.append(temp_report)
        with open(report_file_path, "w") as report_file:
            json.dump(report_json, report_file)    
    else:
        
        
        #check for new error message
        
        result1 = new_error_message_check(history_json, current_error_message)
        result2 = error_elimination_check()
        if result1 == False and result2 == False:
            add_count = 0
        elif (result1 == False and result2 ==True) or (result1 == True and result2 ==False):
            add_count = 1
        else: 
            add_count = 2
        
    history_json.append(temp_history_json)
    with open(history_file_path, "w") as history_file:
        json.dump(history_json, history_file, indent=4)    
    # if new error_message is discovered or old message is eliminated, then adjust the grade
    if add_count != 0:
        database_json = {}        
        with open(database_file_path, "r") as database_file:
            database_json = json.load(database_file)
        for record in database_json:
            if record["label"] == seed:
                record["grade"] = record["grade"] + add_count

        with open(database_file_path, "w") as database_file:
            json.dump(database_json, database_file, indent=4)

               

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
        if saw_file == "verify_HMAC.saw":
            continue

        os.system("saw "+ saw_file + "| tee -a ./log/"+str(round) +".log 1>/dev/null")
    return test_result_func(round)


# This function is for one step in a branch in the evolution mutation
# The logic: select the seed with the highest score first, then randomly
# select one from the random seed pool

def one_mutation_round(database_json):

    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)
    
    # check if the database is empty or not
    if database_json == []:
        database_json.append(database_empty)

    #****Phase1****
    # select the seed with the largest grade from the dataset
    # [] can be repeatedly selected, while others can not
    selected_seed_list = []
    selected_seed_list = selected_seed_list + database_seed_selection(database_json)

    # if the selected seed is [], then take one step further to 
    # randomly select onec

    #****Phase2****
    selected_seed = random_select_from_pool(random_pool_data, database_json)
    selected_seed_list.append(selected_seed)
    # add the selected_seed into the database
    database_json.append({"label":selected_seed_list, "grade": 10})
    # dump into database file
    with open(database_file_path,"w") as database_file:
        json.dump(database_json, database_file,indent=4)
    print(selected_seed_list)
    return selected_seed_list


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
    os.system(init_command)
    json_file = open("genesis_info.json")
    random_pool_data = json.load(json_file)

    # Normal labels start from 1, 0 represents []
    
    database_json = []
    #Initialize database, if the file is empty, then set it as 
    if not os.path.isfile(database_file_path):
        database_json = []
    else:    
        with open(database_file_path) as database_file:
            database_json = json.load(database_file)
    
    report_json = []

    for round in range(10):
        print("round number"+ str(round))
        selected_seed_list = one_mutation_round(database_json)
    
        # if this list exists as a bad seed or it has run out of combinations, skip this round

        mutation_match(selected_seed_list)
        test_result = test(round)
        feedback(selected_seed_list, test_result, round)
            
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






