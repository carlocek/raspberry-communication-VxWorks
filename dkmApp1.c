/* includes */

#include "vxWorks.h"
#include "taskLib.h"
#include "semLib.h"
#include "usrLib.h"
#include "sysLib.h"
#include "subsys/timer/vxbTimerLib.h"
#include "tickLib.h"
#include "cpuset.h"
#include "clockLib.h"
#include "sockLib.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "timerLib.h"
#include "timers.h"
#include <unistd.h>
#include <stdlib.h>
#include <math.h>


//DEFINIZIONE PARAMETRI

//parametri usati nelle primitive di connect() e binding()
#define SERVER_PORT 8000
#define CLIENT_PORT 1235
//locazioni di memoria dove si salvano i valori letti dai sensori
int mem1;
int mem2;
//buffer dove si memorizzano i valori di mem1 e mem2 e che sarà inviato all'app2
int buf[2];
//stringa dove si memorizza il log degli eventi relativi alle transizioni
char log[30];
char logExt[100000];


//task id relativi alle esecuzioni dei tasks
TASK_ID firstTaskId;
TASK_ID secondTaskId;
//task id relativi ai rilasci dei tasks
TASK_ID firstTaskReleaseId;
TASK_ID secondTaskReleaseId;
// task id relativo alla creazione del file di log
TASK_ID taskFilelogId;
//semaforo che definisce quale task sta usando la risorsa mem1
SEM_ID firstMutexId;
//semaforo che definisce quale task sta usando la risorsa mem2
SEM_ID secondMutexId;
//usato per forzare il programma ad usare una sola cpu
cpuset_t affinity;

//variabile usata per le misurazioni
UINT32 timestamp_i;


//DEFINIZIONE FUNZIONI PER CONNESSIONE CON RPI2 TRAMITE SOCKET

//funzione che crea una socket e restituisce il suo descriptor sd
int createSocket(int family, int type, int proto) 
{
	int sd = socket(family,type,proto);
	if (sd < 0)
	{
		printf ("\n create socket error: %s", strerror(errno));
		exit(-1);
	}
	return sd;
}

//funzione che esegue il binding della socket sd all'interfaccia locale
int binding(int sd)
{
	struct sockaddr_in clientaddr;
	memset((void *)&clientaddr, 0, sizeof(clientaddr));
    clientaddr.sin_family=AF_INET;
    clientaddr.sin_port=htons(CLIENT_PORT);
    clientaddr.sin_addr.s_addr= htonl(INADDR_ANY);//accetta qualsiasi interfaccia di rete locale
    int retbind = bind(sd, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
	if (retbind < 0)
	{
		close(sd);
	    printf ("\n bind error: %s",strerror(errno));
	    exit(-1);
	} 	
    return retbind;
}

//funzione che esegue la connect della socket al server creato sulla rpi2
int connecting(int sd)
{
	struct sockaddr_in servaddr;
	memset((void *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(SERVER_PORT);
	servaddr.sin_addr.s_addr= inet_addr("192.168.1.98");//indirizzo ip raspberry 2
	int retconnect = connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (retconnect < 0)
	{
		close(sd);
	    printf ("\n connect error: %s",strerror(errno));
	    exit(-1);
	} 	
    return retconnect;
}

//funzione che manda i dati contenuti in buf sulla socket sd
long writing(int sd, int *buf)
{
	long retwrite = write(sd, buf, sizeof(buf));
	if (retwrite < 0)
	{
		printf("\n write error: %s", strerror(errno));
		fflush(stdout);
		exit(-1);
	}
	printf("\n(%d, %d)\n", buf[0], buf[1]);
	fflush(stdout);
	return retwrite;
}


//ALTRE FUNZIONI 

//funzione che vincola il programma ad usare una singola cpu
void affinitySet(void)
{  
	CPUSET_ZERO (affinity);
    CPUSET_SET  (affinity, 1);
    printf("\n Il codice e' vincolato a usare una sola CPU\n");
    fflush(stdout);
}

//funzione che occupa la risorsa per un numero di secondi specificato da time e che stampa ogni secondo passato
void resourceBusy(int time)
{
	int startingticks = tickGet();	
	int diff = 0;	
	for(int sec = 1, target = sysClkRateGet(); target <= time*sysClkRateGet(); target = sec*sysClkRateGet())
	{
		while(diff < target)
	    {	
			diff = tickGet() - startingticks;	    	    	
	    }
		printf("\n %d sec\n",sec);
		fflush(stdout);
		sec++;	  
	}
}

/*
//versione della resourceBusy che permette l'occupazione per tempi dell'ordine delle decine di ms
void resourceBusy(int ticks)
{
	int startingticks = tickGet();	
	int diff = 0;
	while(diff < ticks)
	{
		diff = tickGet() - startingticks;
	}
}
*/


//funzione che genera un intero casuale tra min e max
int randRange(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

//funzione che crea la stringa costituente il log
void concat(void)
{
	strncat(logExt, log, strlen(log));
	return;
}

//routine del task 1
void firstTask(void)
{
	UINT32 temp;
	taskCpuAffinitySet(0, affinity);
	printf("\n Task %s is starting its execution and attempting to take memory1\n", taskName(0));
	fflush(stdout);
	
	//misurazione relativa alla transizione t1
	timestamp_i = sysTimestamp();
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t1\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	//misurazione relativa alla transizione t2
	timestamp_i = sysTimestamp();
	semTake(firstMutexId, WAIT_FOREVER);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t2\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	printf("\n Task %s has taken resource memory1 and is writing data from sensor1\n", taskName(0));
	fflush(stdout);
	
	//misurazione relativa alla transizione t3
	timestamp_i = sysTimestamp();
	mem1 = randRange(20, 30);
	semGive(firstMutexId);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t3\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();

	printf("\n Task %s is attempting to take memory2\n", taskName(0));
	fflush(stdout);
	
	//misurazione relativa alla transizioen t4
	timestamp_i = sysTimestamp();
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t4\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	//misurazione relativa alla transizione t5
	timestamp_i = sysTimestamp();
	semTake(secondMutexId, WAIT_FOREVER);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t5\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();

	printf("\n Task %s has taken resource memory2 and is writing data from sensor2\n", taskName(0));
	fflush(stdout);
	
	//misurazione relativa alla transizione t6
	timestamp_i = sysTimestamp();
	mem2 = randRange(50, 100);
	semGive(secondMutexId);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t6\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
}



//routine del task 2
void secondTask(int sd)
{
	UINT32 temp;
	taskCpuAffinitySet(0, affinity);
	printf("\n Task %s is starting its execution and attempting to take memory1 and memory2\n", taskName(0));
	fflush(stdout);
	
	//misurazione relativa alla transizione t8
	timestamp_i = sysTimestamp();
	taskPrioritySet(0, 110);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t8\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	//misurazione relativa alla transizione t9
	timestamp_i = sysTimestamp();
	semTake(firstMutexId, WAIT_FOREVER);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t9\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	//misurazione relativa alla transizione t10
	timestamp_i = sysTimestamp();
	semTake(secondMutexId, WAIT_FOREVER);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t10\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	printf("\n Task %s has taken resources memory1 and memory2 and is reading and transmitting data\n", taskName(0));
	fflush(stdout);
	
	//misurazione relativa alla transizione t11
	timestamp_i = sysTimestamp();
	buf[0] = mem1;
	buf[1] = mem2;
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t11\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	
	printf("\n(data: (%d, %d)\n", buf[0], buf[1]);
	fflush(stdout);
	
	//misurazione relativa alla transizione t12
	timestamp_i = sysTimestamp();
	writing(sd, buf);
	semGive(firstMutexId);
	semGive(secondMutexId);
	taskPrioritySet(0,111);
	temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t12\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
}

//routine relativa ai rilasci del task 1
void firstTaskReleaseSpawn(void)
{
	UINT32 temp;
	taskCpuAffinitySet(0, affinity);
	//si inizializza timestamp_i solo per il primo spawning di task1, in modo da calcolare
	//esattamente 10 secondi prima della prima esecuzione reale di task1
	timestamp_i = sysTimestamp();
	FOREVER 
	{
		timestamp_i = sysTimestamp();
		//la routine relativa ai rilasci esegue a priorità massima ogni 10 secondi
		taskDelay(10*sysClkRateGet());
		temp = sysTimestamp()-timestamp_i;
		//si scrive in log il tempo di esecuzione relativo ad una certa porzione di codice
		snprintf(log, sizeof(log), "t0\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
		//si appende alla fine della stringa logExt il nome della transizione ed il tempo di esecuzione
		//calcolato dal firing dell'ultima transizione eseguita
		concat();
		printf("\n Task %s has spawned\n", taskName(0));
		fflush(stdout);
		printf("\n Task tsk1 can start a new execution\n");
		fflush(stdout);
		//spawning del task relativo all'esecuzione reale del task1
		firstTaskId = taskSpawn("tsk1", 110, 0, 5000, (FUNCPTR)firstTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		//si salva in timestamp_i i ticks eseguiti all'inizio di ogni esecuzione reale del task1
		timestamp_i = sysTimestamp();
	}
}

//routine relativa ai rilasci del task 2
void secondTaskReleaseSpawn(int sd)
{
	UINT32 temp;
	taskCpuAffinitySet(0, affinity);
	timestamp_i = sysTimestamp();
	FOREVER
	{
		timestamp_i = sysTimestamp();
		//la routine relativa ai rilasci esegue a priorità massima ogni 20 secondi
		taskDelay(20*sysClkRateGet());
		temp = sysTimestamp()-timestamp_i;
		snprintf(log, sizeof(log), "t7\n%d\n", (int)(round((double)(temp)/sysTimestampFreq())));
		concat();
		printf("\n Task %s has spawned\n", taskName(0));
		fflush(stdout);
		printf("\n Task tsk2 can start a new execution\n");
		fflush(stdout);
		//spawning del task relativo all'esecuzione reale del task2
	    secondTaskId = taskSpawn("tsk2", 111, 0, 5000, (FUNCPTR)secondTask, sd, 0, 0, 0, 0, 0, 0, 0, 0, 0);                                           
	    timestamp_i = sysTimestamp();
	}
}

//funzione che esegue con priorità minima e scrive un file con il contenuto della stringa log
void filelog(void)
{
	taskDelay(120*sysClkRateGet());
	printf("\n sto scrivendo il file di log\n");
	fflush(stdout);
	int fd = open("/tmp/log.txt", O_APPEND | O_RDWR);
	write(fd, logExt, sizeof(logExt));
	close(fd);
	//si termina l'esecuzione dei task una volta scritto il file di log
	taskDelete(firstTaskReleaseId);
	taskDelete(secondTaskReleaseId);
}

//routine di partenza del programma
void init(void)
{	
	sysClkRateSet(250);
	sysTimestampEnable();
	//spawning del task relativo alla creazione del file di log
	taskFilelogId = taskSpawn("tskFilelog", 120, 0, 4000, (FUNCPTR)filelog, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	//creazione dei due mutex
	firstMutexId = semMCreate(SEM_Q_PRIORITY);
	secondMutexId = semMCreate(SEM_Q_PRIORITY);
	printf("\n Task %s created two mutex with id %d and %d\n", taskName(0), firstMutexId, secondMutexId);
	fflush(stdout);
	affinitySet();
	//inizializzazione della connessione tramite socket
	int sd = createSocket(AF_INET, SOCK_DGRAM, 0);
	binding(sd);
	connecting(sd);
	//spawning dei due task relativi ai rilasci
	firstTaskReleaseId = taskSpawn("tskRel1", 101, 0, 4000, (FUNCPTR)firstTaskReleaseSpawn, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	secondTaskReleaseId = taskSpawn("tskRel2", 101, 0, 4000, (FUNCPTR)secondTaskReleaseSpawn, sd, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}