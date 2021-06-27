all: numcheckseq numcheckdatapar numcheckfuncpar 

clean: 
	/bin/rm -f *.o numcheckseq
	/bin/rm -f *.o numcheckdata
	/bin/rm -f *.o numcheckfunc

# vcoê deve criar as regras para gerar os dois outros programas.

numcheckseq: conditions.o numchecks.o timediff.o numcheckseq.o
	gcc -Wall conditions.o numchecks.o timediff.o numcheckseq.o -o numcheckseq

numcheckdatapar: conditions.o numchecks.o timediff.o numcheckdatapar.o
	gcc -Wall conditions.o numchecks.o timediff.o numcheckdatapar.o -o numcheckdatapar -lpthread

numcheckfuncpar: conditions.o numchecks.o timediff.o numcheckfuncpar.o
	gcc -Wall conditions.o numchecks.o timediff.o numcheckfuncpar.o -o numcheckfuncpar -lpthread

numcheckseq.o: numcheckseq.c conditions.h numchecks.h timediff.h
	gcc -Wall -c numcheckseq.c

numcheckdatapar.o: numcheckdatapar.c conditions.h numchecks.h timediff.h
	gcc -Wall -c numcheckdatapar.c -lpthread

numcheckfuncpar.o: numcheckfuncpar.c conditions.h numchecks.h timediff.h
	gcc -Wall -c numcheckfuncpar.c -lpthread

conditions.o: conditions.c conditions.h
	gcc -Wall -c conditions.c

numchecks.o: numchecks.c numchecks.h
	gcc -Wall -c numchecks.c

timediff.o: timediff.c timediff.h
	gcc -Wall -c timediff.c
