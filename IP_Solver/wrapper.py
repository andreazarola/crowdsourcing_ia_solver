#! /usr/bin/env python3
import os

solution_directory = "Solutions"
executable = "./IA_Test.exe"
instance_directory = "../Instances/"
instance_prefix = "test-"
instance_ext = ".json"
solution_prefix = "sol_test-"
solution_ext = ".txt"
parameters = "" #timeout

instance_list = list(range(1,21))

for i in instance_list:
    instance_path = instance_directory + instance_prefix + str(i) + instance_ext;
    if os.path.isfile(instance_path):
        sol_file = solution_directory + "/" + solution_prefix + str(i) + solution_ext
        command = executable + " " + instance_path + " " + parameters + " > " + sol_file
        print(command + "\n")
        if os.path.isfile(sol_file):
            print("Solution file " + sol_file + " exists already -- Skip command")
        else:
            os.system(command)
