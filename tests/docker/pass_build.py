import os
import glob

pass_root_dir = "/project/mutation/passes"

#go into each pass and cmake .. make

for pass_dir in glob.glob(pass_root_dir+"/**"):
    pass_name = pass_dir.split("/")[-1]
    build_dir = pass_root_dir+ "/"+pass_name+ "/build"
    os.system("rm -rf "+ build_dir)
    print("Building "+ pass_name +"...")
    if not os.path.exists(build_dir):
        os.system("mkdir "+ build_dir)
    os.chdir(build_dir)
    os.system("cmake ..")
    os.system("make")



