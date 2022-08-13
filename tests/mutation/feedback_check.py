import json
file_dir = "history.json"

history_content = {}
mutation_point_match = {}
keys = []

with open(file_dir, 'r') as history_file:
    history_content = json.load(history_file)

for i in history_content:

    if (i["mutaion_point_list"])[:-1] in keys:
        for j in history_content:
            if j["mutaion_point_list"] == (i["mutaion_point_list"])[:-1]:
                
                for errmsg in j["error_message"]:
                
                    if errmsg not in i["error_message"]:
                        print("hi")
                        print(i)
    
    keys.append(i["mutaion_point_list"])
