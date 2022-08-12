import os 
import glob
bc_dir = "./bitcode/"
bc_file_list = []
bc_file_command = ""
for bc_file in glob.glob(bc_dir+ "*.bc"):
    if bc_file != "./bitcode/all_llvm.bc":
        bc_file_list.append(bc_file)
        bc_file_command= bc_file_command+ bc_file+ " "

os.system("llvm-link-3.9 -o ./bitcode/all_llvm.bc "+bc_file_command)
