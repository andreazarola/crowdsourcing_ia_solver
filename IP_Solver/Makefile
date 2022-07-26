DATA_DIR = ../InputData
FLAGS = -Wall -std=c++14 -Wfatal-errors -Wsign-compare -Wparentheses -O3  -Wignored-attributes # -g -DCPLEX # -DVERBOSE 
LINUX = no# yes

ifeq ($(LINUX), yes)
	CPLEX = /export/local/share/ILOG/cplex
	CPLEX_LIB = $(CPLEX)/lib/x86-64_linux/static_pic
	CONCERT = /export/local/share/ILOG/concert
	CONCERT_LIB = $(CONCERT)/lib/x86-64_linux/static_pic
#	LINKOPTS =  -pthread -L$(CPLEX_LIB) -L$(CONCERT_LIB) -lboost_program_options -lilocplex -lcplex -lconcert -ldl
else
	CPLEX = /Applications/CPLEX_Studio128/cplex
	CPLEX_LIB = $(CPLEX)/lib/x86-64_osx/static_pic
	CONCERT = /Applications/CPLEX_Studio128/concert
	CONCERT_LIB = $(CONCERT)/lib/x86-64_osx/static_pic
endif	
OPTIONS = -I$(DATA_DIR) -I$(CPLEX)/include -I$(CONCERT)/include $(FLAGS)
LINKOPTS = -L$(CPLEX_LIB) -L$(CONCERT_LIB) -pthread -lconcert -lilocplex -lcplex -ldl # -static

IA_Test.exe: IA_Driver.o IA_IPSolve.o $(DATA_DIR)/IA_Data.o $(DATA_DIR)/Utils.o
	g++ -o IA_Test.exe IA_Driver.o IA_IPSolve.o $(DATA_DIR)/IA_Data.o $(DATA_DIR)/Utils.o $(LINKOPTS)

$(DATA_DIR)/Utils.o: $(DATA_DIR)/Utils.cc $(DATA_DIR)/Utils.hh 
	g++ $(COMPOPTS) -c $(DATA_DIR)/Utils.cc -o $(DATA_DIR)/Utils.o

$(DATA_DIR)/IA_Data.o: $(DATA_DIR)/IA_Data.cc $(DATA_DIR)/IA_Data.hh $(DATA_DIR)/Utils.hh
	g++ $(OPTIONS) -c $(DATA_DIR)/IA_Data.cc -o $(DATA_DIR)/IA_Data.o

IA_IPSolve.o: IA_IPSolve.cc IA_IPSolve.hh $(DATA_DIR)/IA_Data.hh
	g++ $(OPTIONS) -Wignored-attributes  -c IA_IPSolve.cc

IA_Driver.o: IA_Driver.cc $(DATA_DIR)/IA_Data.hh 
	g++ $(OPTIONS) -c IA_Driver.cc

clean:
	rm -f $(DATA_DIR)/IA_Data.o $(DATA_DIR)/Utils.o IA_IPSolve.o IA_Driver.o IA_Test.exe
