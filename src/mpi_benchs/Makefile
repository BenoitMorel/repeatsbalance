libpllpath=../../lib/current

CC=mpic++
DBG= -O3 -g 
CFLAGS=  $(DBG) -std=gnu++11 -W -Wall -ansi -pedantic -I$(libpllpath)
LDFLAGS= $(DBG) -std=gnu++11 -L$(libpllpath) -I$(libpllpath) -L../common
OBJDIR=obj

EXEC=main
OBJ= $(addprefix $(OBJDIR)/, main.o full_tree_traversal.o synchronized_ftt.o)


all: dir $(EXEC) 

dir:
	mkdir -p obj 


main: $(OBJ) 
	$(CC) $(LDFLAGS) -o $@  $^ -lpll -lrepeatsbalance

$(OBJDIR)/%.o : %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<


clean:
	rm -rf $(OBJDIR)/*.o $(EXEC)
