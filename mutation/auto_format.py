import glob
import os
base_dir = "./passes"
for pass_dir in glob.glob(base_dir+ "/**/**/Mutation.cpp"):
    os.system("astyle " + pass_dir)
os.system("astyle " + base_dir + "/Initialization/Initialization/Initialization.cpp" )
