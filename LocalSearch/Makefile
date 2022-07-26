EASYLOCAL = ../../../EasyLocal/easylocal-3
#EASYLOCAL = ../../easylocal-3
DATA_DIR = ../InputData
FLAGS = -std=c++14 -Wall -Wfatal-errors -Wsign-compare -O3 #-pg # -g #
COMPOPTS = -I$(DATA_DIR) -I$(EASYLOCAL)/include $(FLAGS)
LINKOPTS = -lboost_program_options -pthread # -pg
CC = g++

SOURCE_FILES = $(DATA_DIR)/IA_Data.cc $(DATA_DIR)/Utils.cc IA_Basics.cc IA_Helpers.cc  IA_Main.cc
OBJECT_FILES = $(DATA_DIR)/IA_Data.o $(DATA_DIR)/Utils.o IA_Basics.o IA_Helpers.o IA_Main.o
HEADER_FILES = $(DATA_DIR)/IA_Data.hh $(DATA_DIR)/Utils.h IA_Basics.hh IA_Helpers.hh  

IA_solver: $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) $(LINKOPTS) -o IA_solver

$(DATA_DIR)/Utils.o: $(DATA_DIR)/Utils.cc $(DATA_DIR)/Utils.hh 
	$(CC) $(COMPOPTS) -c $(DATA_DIR)/Utils.cc -o $(DATA_DIR)/Utils.o

$(DATA_DIR)/IA_Data.o: $(DATA_DIR)/IA_Data.cc $(DATA_DIR)/IA_Data.hh $(DATA_DIR)/Utils.hh
	$(CC) $(COMPOPTS) -c $(DATA_DIR)/IA_Data.cc -o $(DATA_DIR)/IA_Data.o

IA_Basics.o: IA_Basics.cc IA_Basics.hh $(DATA_DIR)/IA_Data.hh $(DATA_DIR)/Utils.hh
	$(CC) -c $(COMPOPTS) IA_Basics.cc

IA_Helpers.o: IA_Helpers.cc IA_Helpers.hh IA_Basics.hh $(DATA_DIR)/IA_Data.hh
	$(CC) -c $(COMPOPTS) IA_Helpers.cc

IA_Main.o: IA_Main.cc IA_Helpers.hh IA_Basics.hh $(DATA_DIR)/IA_Data.hh
	$(CC) -c $(COMPOPTS) IA_Main.cc

clean:
	rm -f $(OBJECT_FILES) IA_solver

