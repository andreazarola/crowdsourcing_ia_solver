#!/usr/bin/env python3
# coding: utf-8

import os

expected_num_items = [100, 200, 500, 1000, 2000, 5000]
num_item_categories = [1, 2, 3]

modes = {}
modes[1] = [0]
modes[2] = [0, 1, 2] # three modes for 2 categories
modes[3] = [0, 1] # two modes for 3 categories

num_worker_properties = [1, 2, 4, 6]

expertise_level = 40

executable = "./instance_generator_param.py"

for i in expected_num_items:
    for c in num_item_categories:
        for p in num_worker_properties:
            for m in modes[c]:
                command = executable + " -e " + str(i) + " -c " + str(c)  + " -p " + str(p) + " -m " + str(m) + " -ex " + str(expertise_level)
                print(command)
                os.system(command)
                
