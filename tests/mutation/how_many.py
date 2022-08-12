import json

# Check how many points are there in the initialized 
initial_json = {}
with open('filtered_genesis_info.json','r') as initial_file:
    initial_json = json.load(initial_file)

cnt = 0
for i in initial_json:
    cnt = cnt +1

print(cnt)
