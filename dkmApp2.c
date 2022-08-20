/* includes */

#include "vxWorks.h"
#include "taskLib.h"
#include "semLib.h"
#include "usrLib.h"
#include "sysLib.h"
#include "tickLib.h"
#include <stdio.h>
#include <string.h> 
#include "socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include "ipcom_type.h"
#include "ipcom_time.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include <errno.h>
#include "time.h"
#include "subsys/timer/vxbTimerLib.h"
#include "netDrv.h"

//DEFINIZIONE PARAMETRI E VARIABILI GLOBALI


#define SERV_PORT 8000
#define MAXLINE 100000


//array contenente i dati trasmessi dalla prima raspberry
int data[2];


//modifica della pagina html
char *result[2];
char *color[2];
char string[10000];


//log
char log[30];
char logExt[1000];
UINT32 timestamp_i;


//task id relativi alle esecuzioni dei tasks
TASK_ID firstTaskId;
TASK_ID secondTaskId;
TASK_ID thirdTaskId;
//task id relativi ai rilasci dei tasks
TASK_ID firstsecondTaskReleaseId;
TASK_ID thirdTaskReleaseId;
//semaforo che definisce quale task sta usando la risorsa memory
SEM_ID MutexId; 
//usato per forzare il programma ad usare una sola cpu
cpuset_t affinity; 
// task id relativo alla creazione del file di log
TASK_ID taskFileLogId;



//funzione che crea la stringa costituente il log
void concat(void)
{
	strncat(logExt,log,strlen(log));
	return;
}


//DEFINIZIONE FUNZIONI PER CONNESSIONE CON RPI1 TRAMITE SOCKET


//funzione che crea una socket e restituisce il suo descriptor fd
int createSocket(int family, int type, int proto) 
{
	int fd = socket(family,type,proto);
	if (fd == -1)
	{
	 	printf ("create socket error: %s",strerror(errno));
	}
	return fd;
}


//funzione che esegue la binding della socket
int binding(fd)
{
	int res;
	struct sockaddr_in sad;
	memset((void *)&sad, 0, sizeof(sad));
  sad.sin_family=AF_INET;
	sad.sin_port=htons(5193);
	sad.sin_addr.s_addr= htonl(INADDR_ANY);
	if ((res= bind(fd, (struct sockaddr *)&sad, sizeof(sad))) < 0)
	{
		close(fd);
    printf ("bind error: %s",strerror(errno));
	} 	
  return res;
}


//funzione chiama le routine relative alla connessione con la RP1 e ne imposta le opzioni
int connection(void)
{	
	int fd;
	int res;
	struct timeval tv;
	tv.tv_sec=2;
	tv.tv_usec=0;
	printf("\n creazione socket\n");
  fflush(stdout);
  if((fd=createSocket(AF_INET, SOCK_DGRAM,0))<0)
	return -1; 
	if ((res=binding(fd))<0)
		return -1;
	printf("\n binding eseguito correttamente \n");
	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv,sizeof(tv)); //prova a leggere per 2 secondi
	return fd;
}


//funzione che legge i dati sulla socket
long receive(fd)
{
 	long ret; 
 	if ((ret=read(fd,data,MAXLINE))<0)
 		printf ("recv error: %s\n", strerror(errno));
 	else
 		printf("\n recved %d bytes : %d,%d\n", ret, data[0],data[1]);
  return ret;	
}


//DEFINIZIONI FUNZIONI LOCALI, TASKS E ROUTINES DEI TASKS


//dichiarazioni routine task 2
void elaboration(void)
{
	int value1= data[0];
	int value2=data[1];
	//inserisco dei dati arbitrari per simulare i dati ricevuti dall'app 1
	int soglia1=25;
	int soglia2=75;

	if(value1>soglia1)
	{
		result[0]="Temperatura sopra la soglia";
	  color[0]="#ff8566";
	}
	else 
	{
		result[0]="Temperatura sotto la soglia";
		color[0]="#b3ffb3";
	}
	     

	if(value2>soglia2)
	{
		result[1]="Umidita' sopra la soglia";
		color[1]="#ff8566";
	}
	else 
	{
		result[1]="Umidita' sotto la soglia";
		color[1]="#b3ffb3";
	}
	printf("\n %s \n %s \n",result[0],result[1]);
}


//dichiarazione routine task 3


//funzione che modifica la pagina html
void htmlCreate(void)
{	
	snprintf(string,sizeof(string),"<!DOCTYPE html> <html lang='en'>  <head> <title>test pagina progetto SEES</title> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1'> <style>  * {   box-sizing: border-box; }  .row {    display: -ms-flexbox; /* IE10 */   display: flex;   -ms-flex-wrap: wrap; /* IE10 */   flex-wrap: wrap; }  .side {   -ms-flex: 30%; /* IE10 */   flex: 30%;   padding: 20px;   border-style:double;   border-width:5px;   border-right-color:#1B3409;   border-left-style:none;   border-top-style:none;   border-bottom-style:none;   } .fakeimg {   width: 80%;   padding: 20px;   height:140px;   margin:10px; }  .main {     -ms-flex: 70%; /* IE10 */   flex: 70%;   padding: 20px; }  .impianto{   background-color:white;   float:left;white;   width:75%;   padding:20px;   border-style:double;   border-color:grey;   border-width:5px;   border-right-style:none;   height:160px;   margin-left:20px  }  .stato{   background-color:white;   float:left;   width:20%;   padding:20px;   height:160px;  }  body{  margin: 0; } .navbar {   overflow: hidden;   background-color: #1B3409;   position: sticky;   position: -webkit-sticky;   top: 0;   margin-bottom:0; }  /* Style the navigation bar links */ .navbar a {   float: left;   display: block;   color: white;   text-align: center;   padding: 14px 20px;   text-decoration: none;   margin-bottom:0; }  .navbar a.active {   background-color: #66B032;   color: white; }  .navbar a:hover {   background-color: #ddd;   color: black; }  .div1{  background-color:#375F1B;  padding-top:15px;  height:100px;  } footer{         background-color: #1B3409;         padding:10px;         width: 100%;         height: 80px;         bottom: 0;       } </style> </head>  <body>    <div class='div1' style='padding-top:5px;'>  <h1 style='text-align:center;color:white;font-family: Merriweather, serif;'>MONITORAGGIO TRAMITE RASPBERRY</h1>  </div>   <div class='navbar'>   <a href='/' class='active'>Home</a> </div>  <div style='background-color:#EBF7E3;  background-size:cover;'> <div class='row'> <div class='side'> <div style='border-style:double;border-color:#375F1B;margin:25px;margin-top:30px;background-color:#1B3409;padding:5px'>  <p style='padding-left:10px;color:white;'>Il progetto simula un sistema di monitoraggio realizzato mediante l'utilizzo di Raspberry pi 4 e di VxWorks. Una raspberry ha il compito di leggere i dati da alcuni sensori e di inviarli alla seconda raspberry. Quest'ultima riceve i dati, li elabora ed invia i risultati di tale elaborazione a questa pagina</p>  </div>  <div class='fakeimg'> <img src='https://images.vexels.com/media/users/3/136913/isolated/preview/8eeb83410d11efae50bda7dd229c92f4-stadium-icon-by-vexels.png' style='width:300px;height:240px;z-index:1'> </div>  </div> <br>  <div class='main'> <section style='background-color:#1B3409;border-style:double;border-color:#375F1B;margin-left:50px;margin-right:50px;margin-bottom:20px;margin-top:30px'>  <div style='overflow:auto'>      <div class='impianto' style='margin-top:30px'>    <p><b style='font-size:20px;'>SENSORE DI UMIDITA'</b></p>    <p><b>%s</b></p>    </div>       <div class='stato' style='margin-top:30px; background-color:%s;border-style:double;border-width:5px'>    <p style='text-align:center;font-size:20px'><b>VALORE</b></p>    <p style='text-align:center'><b>%d</b></p>    </div>    </div>  <br><br>  <div style='overflow:auto'>       <div class='impianto'>    <p><b style='font-size:20px;'>SENSORE DI TEMPERATURA</b></p>    <p><b>%s</b></p>    </div>       <div class='stato' style='background-color:%s;border-style:double;border-width:5px'>    <p style='text-align:center;font-size:20px'><b>VALORE</b></p>    <p style='text-align:center'><b>%d</b></p>    </div>    </div>  <br><br>      </div> </section> </div> </div> </div><footer>             <h5 style='text-align:center; color:white'>Copyright &copy;&reg; 2022, Leonardo Di Iorio & Carlo Ceccherelli</h5>  </footer> </body> </html>",result[1],color[1],data[1],result[0],color[0],data[0]);	
}


//funzione che riceve una richiesta dal client http
void rcvmsg (int fd)
{
  char buff[1024];
  long ret;
  if ((ret=recv (fd, buff, sizeof(buff), 0))== -1)
    printf ("recv error: %s\n", strerror(errno));
  else
    printf ("recved %d bytes : %s\n", ret, buff);
}


//funzione che crea un server
int createServer(void)
{
	struct sockaddr_in srv;
	memset((void *)&srv, 0, sizeof(srv));
  srv.sin_family=AF_INET;
	srv.sin_port=htons(SERV_PORT);
	srv.sin_addr.s_addr= htonl(INADDR_ANY);
  int ws=socket(AF_INET, SOCK_STREAM,0);
  int optval=1;
  setsockopt (ws, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
	bind(ws, (struct sockaddr *)&srv, sizeof(srv));
	listen(ws,1);
	//ioctl(ws,FIONBIO);
	int flags = fcntl(ws, F_GETFL, 0);
	fcntl(ws, F_SETFL, flags | O_NONBLOCK);
	printf("listening on port 8000 ...");
	return ws;
}	


//funzione che completa la connessione TCP e invia la pagina html al client
void httpMsg(ws)
{
  int connsd;
  printf("\n Creazione pagina html \n");
  timestamp_i = sysTimestamp();
  htmlCreate();
  UINT32 temp= (sysTimestamp())-timestamp_i;
  snprintf(log + strlen(log), sizeof(log), "t11\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
  concat();
  printf("\n Controllo presenza di richieste in coda \n");	  
  timestamp_i = sysTimestamp();
  if ((connsd=accept(ws,NULL,NULL)) == -1)
    perror("\n Nessuna richiesta");
  else	     		          	              
    rcvmsg(connsd);	      	             
  char html[MAXLINE];
  snprintf(html, sizeof(html), "HTTP/1.0 200 OK\n\n %s",string);
  write(connsd,html,strlen(html));
  close(connsd);	
}


//dichiarazioni parte locale


//funzione che vincola il programma ad usare una singola cpu
void affinitySet (void)
{  
	CPUSET_ZERO (affinity);
  CPUSET_SET  (affinity, 1);
  printf("\n Il codice e' vincolato a usare una sola CPU\n");
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


//routine del task 1
void firstTask(fd)
{
  UINT32 temp;
  taskCpuAffinitySet (0, affinity);
  timestamp_i = sysTimestamp();
  temp= (sysTimestamp())-timestamp_i;
	snprintf(log, sizeof(log), "t1\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	printf("\n Task %s is starting its execution and attempting to take memory\n", taskName(0));
	fflush(stdout);
	timestamp_i = sysTimestamp();
	semTake(MutexId, WAIT_FOREVER);	
  temp = sysTimestamp()-timestamp_i;
	snprintf(log, sizeof(log), "t2\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
	concat();
	printf("\n Task %s has taken resource memory and is writing new data\n", taskName(0));
	fflush(stdout);
	timestamp_i = sysTimestamp();
	receive(fd);
	semGive(MutexId);	
  temp = sysTimestamp()-timestamp_i;
 	snprintf(log, sizeof(log), "t3\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
 	concat();
 	timestamp_i=sysTimestamp();
}


//routine del task 2
void secondTask(void)
{
  UINT32 temp;
  taskCpuAffinitySet (0, affinity);	
	printf("\n Task %s is starting its execution and attempting to take memory\n", taskName(0));
	fflush(stdout);
  timestamp_i = sysTimestamp();
  taskPrioritySet(0,110);
  temp= (sysTimestamp())-timestamp_i;
  snprintf(log, sizeof(log), "t5\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
  concat();
  timestamp_i = sysTimestamp();
	semTake(MutexId, WAIT_FOREVER);		
	temp= (sysTimestamp())-timestamp_i;
  snprintf(log, sizeof(log), "t6\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
  concat();
	printf("\n Task %s has taken resource memory and is completing its execution\n", taskName(0));
	fflush(stdout);
	timestamp_i = sysTimestamp();
	elaboration();
	semGive(MutexId);
	taskPrioritySet(0,111);
	temp= (sysTimestamp())-timestamp_i;
  snprintf(log, sizeof(log), "t7\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
  concat();
 	timestamp_i = sysTimestamp();
}


//routine del task 3
void thirdTask(ws)
{
  UINT32 temp;
  taskCpuAffinitySet (0, affinity);					
	printf("\n Task %s is starting its execution and attempting to take memory\n", taskName(0));
	fflush(stdout);
	timestamp_i = sysTimestamp();
	taskPrioritySet(0,110);
	temp= (sysTimestamp())-timestamp_i;
  snprintf(log, sizeof(log), "t9\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
  concat();
  timestamp_i = sysTimestamp();
	semTake(MutexId, WAIT_FOREVER);	
	temp= (sysTimestamp())-timestamp_i;
  snprintf(log, sizeof(log), "t10\n%f\n", (int)(round((double)(temp)/sysTimestampFreq())));
  concat();
	printf("\n Task %s has taken resource memory and is completing its execution\n", taskName(0));
	fflush(stdout);
	timestamp_i = sysTimestamp();
	httpMsg(ws);
	semGive(MutexId);
	taskPrioritySet(0,112);
  temp = sysTimestamp()-timestamp_i;
  snprintf(log, sizeof(log), "t12\n%f\n", ((double)((temp)*1000/sysTimestampFreq())));
  concat();
	timestamp_i = sysTimestamp();
}


//routine relativa ai rilasci del task 1 e 2
void firstandsecondTaskReleaseSpawn(fd)
{
	taskCpuAffinitySet (0, affinity);
  timestamp_i =sysTimestamp();
  UINT32 temp;
	FOREVER
	{
		taskDelay(10*sysClkRateGet());
  	temp= (sysTimestamp())-timestamp_i;
		snprintf(log,sizeof(log),"t0\n%f\nt4\n%f\n", ((double)((temp)*1000/54000000)),((double)((temp)*1000/54000000)));
		concat();
		printf("\n Task %s has spawned\n", taskName(0));
		fflush(stdout);
		printf("\n Task 1 and Task 2 can start a new execution\n");
    fflush(stdout);
    firstTaskId = taskSpawn("tsk1", 110, 0, 5000, (FUNCPTR)firstTask, fd, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    secondTaskId = taskSpawn("tsk2", 111, 0, 5000, (FUNCPTR)secondTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    timestamp_i = sysTimestamp();
	}
}


//routine relativa ai rilasci del task 3
void thirdTaskReleaseSpawn(ws)
{
	taskCpuAffinitySet (0, affinity);
	timestamp_i = sysTimestamp();
	UINT32 temp;
	FOREVER
	{			
		taskDelay(20*sysClkRateGet());
		temp= (sysTimestamp())-timestamp_i;
    snprintf(log, sizeof(log), "t8\n%f\n", ((double)(temp)*1000/54000000));
    concat();
		printf("\n Task %s has spawned\n", taskName(0));
    fflush(stdout);
    printf("\n Task 3 can start a new execution\n");	
    fflush(stdout);
    thirdTaskId = taskSpawn("tsk3", 112, 0, 5000, (FUNCPTR)thirdTask, ws, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    timestamp_i = sysTimestamp();	 
	}
}


//funzione che esegue con priorità minima e scrive un file con il contenuto della stringa logExt
void fileLog(void)
{
	taskDelay(120*sysClkRateGet());
	printf("\n il programma e' terminato e posso scrivere il file\n");
	fflush(stdout);
	int fp;
	fp = open("/tmp/log.txt", O_APPEND | O_RDWR);
  write(fp,logExt,sizeof(logExt));
	close(fp);
	taskDelete(firstsecondTaskReleaseId);
	taskDelete(thirdTaskReleaseId);
}


//routine di partenza del programma
void init(void)
{	
	sysClkRateSet(250);
	sysTimestampEnable();
	taskFileLogId = taskSpawn("tskFileLog", 120, 0, 4000, (FUNCPTR)fileLog, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	MutexId = semMCreate(SEM_Q_PRIORITY);
	printf("\n");
	fflush(stdout);
	printf("\n Task t1 created a mutex with id %d \n", MutexId);		
	fflush(stdout);
	affinitySet();
	int fd=connection();
	int ws=createServer();
	firstsecondTaskReleaseId = taskSpawn ("TskRel1_Rel2", 101, 0, 4000, (FUNCPTR)firstandsecondTaskReleaseSpawn, fd, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	thirdTaskReleaseId = taskSpawn ("TaskRel3", 101, 0, 4000, (FUNCPTR)thirdTaskReleaseSpawn,ws, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}