#Makefile test
CC := g++
TAR = test.out
TPOBJ = ThreadPool.o
MAINOBJ = main.o
OBJDIR = ./obj/
SRCDIR = ./src/
INCDIR = ./include
LINK = -lpthread
OUTDIR = ./bin/

$(TAR):$(TPOBJ) $(MAINOBJ)
	$(CC) ${OBJDIR}$(TPOBJ) ${OBJDIR}$(MAINOBJ) -I ${INCDIR} -o ${OUTDIR}$(TAR) $(LINK)

$(TPOBJ):${SRCDIR}NManage.cpp
	$(CC) -c ${SRCDIR}NManage.cpp -I ${INCDIR} -o ${OBJDIR}$(TPOBJ) $(LINK)

$(MAINOBJ):${SRCDIR}main.cpp
	$(CC) -c ${SRCDIR}main.cpp -I ${INCDIR} -o ${OBJDIR}$(MAINOBJ) $(LINK)