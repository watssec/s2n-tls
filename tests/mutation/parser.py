import glob
import json
import regex as re
root_path = "./spec/*"
test_saw_match_file_path = "test_saw_match.json"
test_saw_match_json = {}
test_saw_match_file = open(test_saw_match_file_path, "w")
for saw_file_dir in glob.glob(root_path):
    for saw_file_path in glob.glob(saw_file_dir + "/*.saw"):
        print(saw_file_path)
        with open(saw_file_path,"r") as saw_file:
            file_content = saw_file.read()
            pattern1 = r"crucible_llvm_verify \w+ \"\w+\""
            pattern2 = r"crucible_llvm_unsafe_assume_spec \w+ \"\w+\""
            function_list1 = re.findall(pattern1, file_content)
            function_list2 = re.findall(pattern2, file_content)
            test_saw_match_json[saw_file_path.split("/")[-1]] = function_list1+ function_list2
           

json.dump(test_saw_match_json, test_saw_match_file,indent=4)
