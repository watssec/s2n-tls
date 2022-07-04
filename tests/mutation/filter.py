import json

original_json = "genesis_info.json"
filtered_json = "filtered_genesis_info.json"
function_call_path = "callgraphnode_callgraph.json"
original_function_list = ["s2n_blob_zero","s2n_increment_drbg_counter","s2n_drbg_bytes_used","s2n_drbg_block_encrypt","s2n_drbg_bits","s2n_drbg_update","s2n_drbg_seed","s2n_drbg_mix","s2n_drbg_instantiate","s2n_drbg_generate","s2n_connection_get_client_auth_type","s2n_advance_message","s2n_conn_set_handshake_type"]
derived_function_list = []
function_call_map = {}
with open(function_call_path, "r") as call_graph_data:
    call_graph = json.load(call_graph_data)
    for i in call_graph:
        if list(i.keys())[0] in original_function_list:
            derived_function_list += i[list(i.keys())[0]]

filtered_json_data = []
original_json_file = open(original_json, "r")
original_json_data = json.load(original_json_file)
function_list = original_function_list + derived_function_list

for i in original_json_data:
    if i["function_name"] in function_list:
        filtered_json_data.append(i)
    
with open(filtered_json, "w") as filtered_json_file:
    json.dump(filtered_json_data, filtered_json_file, indent =4)  
