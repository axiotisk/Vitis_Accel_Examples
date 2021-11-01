#!/usr/bin/env /tools/cpkg/.packages/x86_64/RHEL7.2/python/3.7.1/bin/python3.7 
import json
import os
import subprocess
from os import listdir
from sys import argv
from collections import OrderedDict
from itertools import islice

XSA = 'xilinx_u200_qdma'
VERSION = 'Vitis 2019.2'

def overview(target,data,ref_data):
    title = data["name"]
    title = title.replace("(C)", "")
    title = title.replace("(CL)", "")
    title = title.replace("(RTL)", "")
    title = title.replace("(HLS C/C++ Kernel)", "")
    target.write(title)
    target.write("\n")
    target.write("=" * len(title))
    target.write("\n\n")
    target.write(('\n').join(data["description"]))
    target.write("\n\n")
    if 'more_info' in data:
        target.write(('\n').join(data["more_info"]))
        target.write("\n\n")
    if 'perf_fields' in data:
        target.write("PERFORMANCE\n")
        target.write("-" * len("PERFORMANCE"))
        target.write("\n\n")
        ctr = len(data["perf_fields"])
        for idx in range(0, ctr - 1):
            target.write(data["perf_fields"][idx])
            target.write("|")
        target.write(data["perf_fields"][ctr - 1])
        target.write("\n")
        for idx in range(0, ctr - 1):
            target.write("-----|")
        target.write("-----\n")
        count = len(data["performance"])	
        for result in range(0, count):
            for i in range(0, ctr - 1):
                target.write(data["performance"][result][i])
                target.write("|")
            target.write(data["performance"][result][ctr - 1])	
            target.write("\n")
    if 'key_concepts' in data:
        target.write("**KEY CONCEPTS:** ")
        elem_count = len(data["key_concepts"])
        for result in data["key_concepts"]:
            elem_count -= 1
            if "key_concepts" in ref_data:
                if result in ref_data["key_concepts"]:
                    target.write("`")
                    target.write(result)
                    target.write(" <")
                    target.write(ref_data["key_concepts"][result])
                    target.write(">`__")
                else:
                    target.write(result)
            if elem_count != 0:
                target.write(", ")
        target.write("\n\n")
    if 'keywords' in data:
        target.write("**KEYWORDS:** ")
        word_count = len(data["keywords"])
        for result in data["keywords"]:
            word_count -= 1
            if "keywords" in ref_data:
                if result in ref_data["keywords"]:
                    target.write("`")
                    target.write(result)
                    target.write(" <")
                    target.write(ref_data["keywords"][result])
                    target.write(">`__")
                else:
                    target.write(result)
            if word_count != 0:
                target.write(", ")
        target.write("\n\n")
    listfiles = os.listdir('./')
    if 'details.rst' in listfiles:
        with open('details.rst', 'r') as fin:
            for i, x in enumerate(fin):
                if 2 <= i :
                    target.write(x)
    target.write("\n")
    return

def requirements(target,data,plt_ref_data):
    if 'platform_blacklist' in data:
        target.write("**EXCLUDED PLATFORMS:** ")
        plat_count = len(data["platform_blacklist"])
        target.write("\n\n")
        for result in data['platform_blacklist']:
            plat_count -= 1
            if "platform_blacklist"  in plt_ref_data:
                target.write(" - ")
                if result in plt_ref_data["platform_blacklist"]:
                    target.write(plt_ref_data["platform_blacklist"][result])
                else:
                    target.write(result)
                target.write("\n")
        target.write("\n")
    return

def hierarchy(target):
    target.write("DESIGN FILES\n")
    target.write("-" * len("DESIGN FILES"))
    target.write("\n\n")
    target.write("Application code is located in the src directory. ")
    target.write("Accelerator binary files will be compiled to the xclbin directory. ")
    target.write("The xclbin directory is required by the Makefile and its contents will be filled during compilation. A listing of all the files ")
    target.write("in this example is shown below\n\n")
    target.write("::\n\n")
    tree_cmd = ["git ls-files | grep -e data -e src"]
    proc = subprocess.Popen(tree_cmd,stdout=subprocess.PIPE, shell=True)
    output = proc.communicate()[0]
    output = str(output).split("\'")[1]
    output = output.split("\\n")
    for lines in output:
        target.write("   ")
        target.write(lines)
        target.write("\n")
    refpath = init_cur_dir.replace(cur_dir+"/","")
    target.write("Access these files in the github repo by `clicking here <https://github.com/Xilinx/Vitis_Accel_Examples/tree/master/" + refpath + ">`__.\n\n")
    return

def commandargs(target,data):
    target.write("COMMAND LINE ARGUMENTS\n")
    target.write("-" * len("COMMAND LINE ARGUMENTS"))
    target.write("\n\n")
    target.write("Once the environment has been configured, the application can be executed by\n\n")
    target.write("::\n\n")
    target.write("   ")
    if "launch" in data:
        if not "cmd_args" in data["launch"][0]:
            target.write('./' + data["host"]["host_exe"])
        else:
            target.write('./' + data["host"]["host_exe"])
            args = data["launch"][0]["cmd_args"].split(" ")
            for arg in args[0:]:
                target.write(" ")
                arg = arg.replace('BUILD/', '<')
                arg = arg.replace('PROJECT', '.')
                arg = arg.replace('.xclbin', ' XCLBIN>') 
                target.write(arg)
    else:
        target.write('./' + data["host"]["host_exe"])
    target.write("\n\n")
    return

# Get the argument from the description
script, desc_file, name = argv

# load the description file
print ("Vitis MD2RST File Genarator")
desc = open(desc_file,'r')

# load the json data from the file
data = json.load(desc)
desc.close()

file_name = "LICENSE.txt" # file to be searched
cur_dir = os.getcwd()      # Dir from where search starts can be replaced with any path
init_cur_dir = cur_dir
ref_data = ""
platform_ref_data = ""

while True:
    file_list = os.listdir(cur_dir)
    parent_dir = os.path.dirname(cur_dir)
    if file_name in file_list:
        path = cur_dir + '/common/utility/reference.json'
        ref_file = open(path,'r')
        ref_data = json.load(ref_file)
        ref_file.close()
        platform_path = cur_dir + '/common/utility/platform_ref.json'
        platform_ref_file = open(platform_path,'r')
        platform_ref_data = json.load(platform_ref_file)
        platform_ref_file.close()
        break
    else:
        if cur_dir == parent_dir:         # if dir is root dir
            print ("LICENSE.txt file not found")
            break
        else:
            cur_dir = parent_dir

assert("OpenCL" in data['runtime'])

if "match_readme" in data and data["match_readme"] == "false":
    print ("ERROR:: README Manually Edited:: README Generator Failed\n")
else:
    print ("Generating the README for %s" % data["name"])
    target = open(name + ".rst","w")
    overview(target,data,ref_data)
    requirements(target,data)
    hierarchy(target)
    commandargs(target,data)

target.close
