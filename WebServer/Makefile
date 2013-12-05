MAKE=gmake
RM=rm
MKDIR=mkdir
OUTFILE1=551server_threaded
OUTFILE2=551server_eventdriven
OBJ=content.o csock.o request.o serv.o
ALL_OBJ=content.o csock.o emain.o eserv.o request.o serv.o tmain.o tserv.o
OBJ1=$(OBJ) tmain.o tserv.o
OBJ2=$(OBJ) emain.o eserv.o

COMPILE=g++ -c   -g -std=c++0x -o "$(*F).o" $<
LINK1=g++  -g -nostdlibs -lpthread -o "$(OUTFILE1)" $(OBJ1)
LINK2=g++  -g -nostdlibs -o "$(OUTFILE2)" $(OBJ2)

# Pattern rules
%.o : %.cpp
	$(COMPILE)

# Build rules
all: $(OUTFILE1) $(OUTFILE2)

$(OUTFILE1): $(OBJ1)
	$(LINK1)

$(OUTFILE2): $(OBJ2)
	$(LINK2)


# Rebuild this project
rebuild: cleanall all

# Clean this project
clean:
	$(RM) -f $(OUTFILE1) 
	$(RM) -f $(OUTFILE2)
	$(RM) -f $(ALL_OBJ)

# Clean this project and all dependencies
cleanall: clean

