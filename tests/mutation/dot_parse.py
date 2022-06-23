import pydot
import pydotplus
import json 

graphs = pydotplus.graph_from_dot_file("callgraph.dot")
edge_list = graphs.get_edges()
function_list = ["s2n_blob_zero","s2n_increment_drbg_counter","s2n_drbg_bytes_used","s2n_drbg_block_encrypt","s2n_drbg_bits","s2n_drbg_update","s2n_drbg_seed","s2n_drbg_mix","s2n_drbg_instantiate","s2n_drbg_generate","s2n_connection_get_client_auth_type","s2n_advance_message","s2n_conn_set_handshake_type"]
function_call_map_path = "function_call_map.json"
function_call_map = {}

for i in edge_list:
    #print(i.get_destination())

    source_node = graphs.get_node(i.get_source())
    if source_node == []:
        continue
    else:
        source_function_name = source_node[0].get_attributes()["label"].strip("\"").strip("{").strip("}")


        destination_node = graphs.get_node(i.get_destination())
        if destination_node == []:
            continue
        else:
            destination_function_name = destination_node[0].get_attributes()["label"].strip("\"").strip("{").strip("}")
            dict_keys = list(function_call_map.keys())
            if (source_function_name not in dict_keys) and (source_function_name in function_list):
                function_call_map[source_function_name] = []
            if(source_function_name in function_list) and destination_function_name not in function_call_map[source_function_name]:
                function_call_map[source_function_name].append(destination_function_name)

print(function_call_map)
with open(function_call_map_path, "w") as function_call_file:
    json.dump(function_call_map, function_call_file, indent =4)