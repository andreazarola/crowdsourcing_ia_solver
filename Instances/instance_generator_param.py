#!/usr/bin/env python3
# coding: utf-8

import numpy as np
import scipy as sp
import pandas as pd
import pickle
#from tqdm import tqdm
import os
import random
import glob
import warnings
import argparse
import json
import pprint
import math


class Feature:
    def __init__(self, id, levels, assignments):
        self.id = id
        self.levels = levels
        self.assignments = assignments
    def __str__(self):
        return str(self.__class__) + ": " + str(self.__dict__)
        
class Object:
    def __init__(self, id, features):
        self.id = id
        self.features = features
    def __str__(self):
        return str(self.__class__) + ": " + str(self.__dict__)


parser = argparse.ArgumentParser(description='Run Item Assignment genererator')
parser.add_argument('-c', dest='categories', default=1,
                    help='Number of item categories <1, 2, 3>, default n=1')
parser.add_argument('-m', dest='mode', default=1,
                    help='Two modes for instances with 1 categories <0, 1>, default n=1')
parser.add_argument('-p', dest='properties', default=1, help='Number of worker properties <1, 2,4 ,6>, default n=1')
parser.add_argument('-e', dest='expected_num_items', default=10, help='Expected number of items, default n=1')
parser.add_argument('-s', dest='inst_name', default="test", help='Instance file, default test')
parser.add_argument('-ex', dest='expertise_level', default=40, help='Minimum expertise level, default n=40')

args = parser.parse_args()


mode = int(args.mode) #if 0 (A), 2 categories, levels 4/2
            #if 1 (B), 2 categories, levels 3/2
            #else (C), 2 categories, levels 6/2

num_categories = int(args.categories)
num_properties = int(args.properties)
expected_num_items = int(args.expected_num_items)
expertise_level = int(args.expertise_level)

num_item_levels = []
categories_vect = []

if (num_categories == 1):
    num_item_levels.append(6)
    total_worker_assignments = 6
    prefix = ""
elif(num_categories == 2):
    if mode == 0:
        num_item_levels.append(4)
        num_item_levels.append(2)
        total_worker_assignments = 4 #least common multiple
        prefix = "-A"
    elif mode == 1:
        num_item_levels.append(3)
        num_item_levels.append(2)
        total_worker_assignments = 6
        prefix = "-B"
    elif mode == 2:
        num_item_levels.append(6)
        num_item_levels.append(2)
        total_worker_assignments = 6
        prefix = "-C"    
    elif mode == 3:
        num_item_levels.append(6)
        num_item_levels.append(2)
        total_worker_assignments = 6
        prefix = "-case"
elif(num_categories == 3):
    if mode == 0:
        num_item_levels.append(4)
        num_item_levels.append(3)
        num_item_levels.append(2)
        total_worker_assignments = 12
        prefix = "-A"
    elif mode == 1:
        num_item_levels.append(6)
        num_item_levels.append(4)
        num_item_levels.append(2)
        total_worker_assignments = 12
        prefix = "-B"    
else:
    print("Wrong number of categories, admitted values <1, 2, 3>")
    assert(false)
    
num_worker_levels = []
properties_vect = []
    
if (num_properties == 1):
    num_worker_levels.append(1)
    item_assignments = 1
    min_repetitions = 5
elif(num_properties == 2):
    if mode == 0 or mode == 1 or mode == 2:
        num_worker_levels.append(3)
        num_worker_levels.append(4)
        item_assignments = 2
        min_repetitions = 8 # max num_worker_levels * item_assignments
    elif mode == 3:
        num_worker_levels.append(2)
        num_worker_levels.append(2)
        item_assignments = 5
        min_repetitions = 10
elif(num_properties == 4):
    num_worker_levels.append(2)
    num_worker_levels.append(3)
    num_worker_levels.append(4)
    num_worker_levels.append(5)
    item_assignments = 2
    min_repetitions = 10
elif(num_properties == 6):
    num_worker_levels.append(2)
    num_worker_levels.append(2)
    num_worker_levels.append(3)
    num_worker_levels.append(3)
    num_worker_levels.append(4)
    num_worker_levels.append(5)
    item_assignments = 1
    min_repetitions = 5
else:
    print("Wrong number of properties, admitted values <1, 2, 4, 6>")
    assert(false)
    
prod = 1
for k in num_item_levels:
    prod = prod * k

num_repetitions = math.ceil(expected_num_items/prod)
num_items = prod * num_repetitions
lb_workers = math.ceil(num_items * min_repetitions/total_worker_assignments);


    
instance_name = args.inst_name + "-C" + str(args.categories)  + prefix + "-P" + str(args.properties) + "-" + str(num_items)

min_quality_item = min_repetitions * expertise_level


base = {"id": f"{instance_name}",
        "min_item_repetitions": min_repetitions,
        "min_item_quality_level": min_quality_item,
        }
            
# load categories
for k in range(num_categories):
    c_id = "C" + str(k+1)
    levels = []
    for l in range(num_item_levels[k]):
        l_id = "L" + str(k+1) + "-" + str(l+1)
        levels.append(l_id)
    c_worker_assignments = int(total_worker_assignments/num_item_levels[k])
    c = Feature(c_id, levels, c_worker_assignments)
    categories_vect.append(c)
    
categories = []
for c in categories_vect:
    toadd = {
        "id": f"{c.id}",
        "levels": c.levels,
        "worker_assignments" : c.assignments
        }
    categories.append(toadd)
    
base["categories"] = categories

# load properties
for k in range(num_properties):
    p_id = "P" + str(k+1)
    levels = []
    for l in range(num_worker_levels[k]):
        l_id = "l" + str(k+1) + "-" + str(l+1)
        levels.append(l_id)
    p = Feature(p_id, levels, item_assignments)
    properties_vect.append(p)
    
properties = []
for p in properties_vect:
    toadd = {
        "id": f"{p.id}",
        "levels": p.levels,
        "item_assignments" : p.assignments
        }
    properties.append(toadd)
    
base["properties"] = properties

item_vect = []
count = 0

if(num_categories == 1):
    for rep in range(num_repetitions):
        for rel_cat_1 in categories_vect[0].levels:
            item_cat = {}
            i_id = "I" + str(count)
            item_cat[categories_vect[0].id] = rel_cat_1
            i = Object(i_id, item_cat)
            count = count + 1
            item_vect.append(i)
            
elif(num_categories == 2):
    for rep in range(num_repetitions):
        for rel_cat_1 in categories_vect[0].levels:
            for rel_cat_2 in categories_vect[1].levels:
                item_cat = {}
                i_id = "I" + str(count)
                item_cat[categories_vect[0].id] = rel_cat_1
                item_cat[categories_vect[1].id] = rel_cat_2
                i = Object(i_id, item_cat)
                count = count + 1
                item_vect.append(i)

elif(num_categories == 3):
    for rep in range(num_repetitions):
        for rel_cat_1 in categories_vect[0].levels:
            for rel_cat_2 in categories_vect[1].levels:
                    for rel_cat_3 in categories_vect[2].levels:
                        item_cat = {}
                        i_id = "I" + str(count)
                        item_cat[categories_vect[0].id] = rel_cat_1
                        item_cat[categories_vect[1].id] = rel_cat_2
                        item_cat[categories_vect[2].id] = rel_cat_3
                        i = Object(i_id, item_cat)
                        count = count + 1
                        item_vect.append(i)

items = []
for i in item_vect:
    f_vect = []          
    for id, l in i.features.items():
        toadd = {
            "id": f"{id}",
            "level": f"{l}"
        }
        f_vect.append(toadd)
    header = { "id": f"{i.id}",
                "categories": f_vect
             }
    items.append(header)
    
base["items"] = items

#print(type(item_vect))

count = 0
worker_vect = []

if(num_properties == 1):
    for rel_pro_1 in properties_vect[0].levels:
        worker_cat = {}
        w_id = "W" + str(count)
        worker_cat[properties_vect[0].id] = rel_pro_1
        w = Object(w_id, worker_cat)
        count = count + 1
        worker_vect.append(w)

elif(num_properties == 2):
    for rel_pro_1 in properties_vect[0].levels:
        for rel_pro_2 in properties_vect[1].levels:
            worker_cat = {}
            w_id = "W" + str(count)
            worker_cat[properties_vect[0].id] = rel_pro_1
            worker_cat[properties_vect[1].id] = rel_pro_2
            w = Object(w_id, worker_cat)
            count = count + 1
            worker_vect.append(w)

elif(num_properties == 3):
    for rel_pro_1 in properties_vect[0].levels:
        for rel_pro_2 in properties_vect[1].levels:
            for rel_pro_3 in properties_vect[2].levels:
                worker_cat = {}
                w_id = "W" + str(count)
                worker_cat[properties_vect[0].id] = rel_pro_1
                worker_cat[properties_vect[1].id] = rel_pro_2
                worker_cat[properties_vect[1].id] = rel_pro_3
                w = Object(w_id, worker_cat)
                count = count + 1
                worker_vect.append(w)
 
elif(num_properties == 4):
    for rel_pro_1 in properties_vect[0].levels:
        for rel_pro_2 in properties_vect[1].levels:
            for rel_pro_3 in properties_vect[2].levels:
                for rel_pro_4 in properties_vect[3].levels:
                    worker_cat = {}
                    w_id = "W" + str(count)
                    worker_cat[properties_vect[0].id] = rel_pro_1
                    worker_cat[properties_vect[1].id] = rel_pro_2
                    worker_cat[properties_vect[2].id] = rel_pro_3
                    worker_cat[properties_vect[3].id] = rel_pro_4
                    w = Object(w_id, worker_cat)
                    count = count + 1
                    worker_vect.append(w)

elif(num_properties == 6):
    for rel_pro_1 in properties_vect[0].levels:
        for rel_pro_2 in properties_vect[1].levels:
            for rel_pro_3 in properties_vect[2].levels:
                for rel_pro_4 in properties_vect[3].levels:
                    for rel_pro_5 in properties_vect[4].levels:
                        for rel_pro_6 in properties_vect[5].levels:
                            worker_cat = {}
                            w_id = "W" + str(count)
                            worker_cat[properties_vect[0].id] = rel_pro_1
                            worker_cat[properties_vect[1].id] = rel_pro_2
                            worker_cat[properties_vect[2].id] = rel_pro_3
                            worker_cat[properties_vect[3].id] = rel_pro_4
                            worker_cat[properties_vect[4].id] = rel_pro_5
                            worker_cat[properties_vect[5].id] = rel_pro_6
                            w = Object(w_id, worker_cat)
                            count = count + 1
                            worker_vect.append(w)

prod = 1
for k in num_worker_levels:
    prod = prod * k

approx = 1#1.5#1.21#1.001
count = 0
num_workers = lb_workers * approx
worker_rep = math.ceil(num_workers/prod)
worker_vect_current = []
workers = []
for w in range(num_workers):
    if len(worker_vect_current) == 0:
        worker_vect_current = list(worker_vect) #copy the list
    expertise_value = random.randint(0,100)
    worker = random.randint(0, len(worker_vect_current)-1)
    f_vect = []
    for id, l in worker_vect_current[worker].features.items():
            toadd = {
                "id": f"{id}",
                "level": f"{l}"
                }
            f_vect.append(toadd)
    header = { "id": f"W{w}",
                   "expertise": expertise_value,
                   "properties": f_vect
             }
    workers.append(header)
    del worker_vect_current[worker]

#approx = 1#1.5#1.21#1.001
#count = 0
#num_workers = lb_workers * approx
#worker_rep = math.ceil(num_workers/prod)
#workers = []
#for rep in range(worker_rep):
#    for w in worker_vect:
#        expertise_value = random.randint(0,100)
#        f_vect = []
#        for id, l in w.features.items():
#            toadd = {
#                "id": f"{id}",
#                "level": f"{l}"
#                }
#            f_vect.append(toadd)
#        header = { "id": f"W{count}",
#                   "expertise": f"{expertise_value}",
#                   "properties": f_vect
#             }
#        workers.append(header)
#        count = count + 1
#        if count == math.ceil(num_workers):
#            break
                
base["workers"] = workers

#print(lb_workers)
#print(len(worker_vect))
#print(worker_rep)

import json
import pprint

instance_file = instance_name + ".json"
with open(instance_file, 'w') as outfile:
   json.dump(base, outfile, indent=2, sort_keys=False)




