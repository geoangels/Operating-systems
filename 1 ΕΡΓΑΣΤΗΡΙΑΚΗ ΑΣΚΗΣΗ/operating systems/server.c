/* server.c

   Sample code of 
   Assignment L1: Simple multi-threaded key-value server
   for the course MYY601 Operating Systems, University of Ioannina 

   (c) S. Anastasiadis, G. Kappes 2016

*/



#include "utils.h"
#include "kissdb.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>

#define MY_PORT                 6767
#define BUF_SIZE                1160
#define KEY_SIZE                 128
#define HASH_SIZE               1024
#define VALUE_SIZE              1024
#define MAX_PENDING_CONNECTIONS   10
#define QUEUE_SIZE			10
#define THREADS                 10

// Definition of the operation type.
typedef enum operation {
  PUT,
  GET
} Operation; 

// Definition of the request.
typedef struct request {
  Operation operation;
  char key[KEY_SIZE];  
  char value[VALUE_SIZE];
} Request;


// stoixeio ouras
// exei enan file descriptor gia ti sindesi
// kai tin ora enarksis
struct oura{
	int connection_fd;
	struct timeval start_time; 
	
};


// Oura kai nimata
int head; // head ouras
int tail; //tail ouras
int plithos=0; //plithos stoixeiwn ouras
struct oura Q[QUEUE_SIZE]; // h oura, krataei oles tis aitiseis
pthread_t tid[THREADS]; 

int total_service_time=0;
pthread_mutex_t tst = PTHREAD_MUTEX_INITIALIZER; 
int total_waiting_time=0;
pthread_mutex_t twt = PTHREAD_MUTEX_INITIALIZER; 
int completed_requests=0; 
pthread_mutex_t cr = PTHREAD_MUTEX_INITIALIZER; 


pthread_mutex_t Q_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex gia oura kai non_empty kai non_full queue
pthread_cond_t non_empty_Queue = PTHREAD_COND_INITIALIZER; 
pthread_cond_t non_full_Queue = PTHREAD_COND_INITIALIZER; 

// metavlites gia anagnostes grafeis
int reader_count = 0;
int writer_count = 0;

pthread_mutex_t reader_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;



// Definition of the database.
KISSDB *db = NULL;

// sinartiseis
void enQ(int new_connection);
struct oura * deQ();
void create_threads();
void *katanalotis(void  *x);
static void sig_handler(int signo);
void join_threads();
void ypologismos();


/**
 * @name parse_request - Parses a received message and generates a new request.
 * @param buffer: A pointer to the received message.
 *
 * @return Initialized request on Success. NULL on Error.
 */
Request *parse_request(char *buffer) {
  char *token = NULL;
  Request *req = NULL;
  
  // Check arguments.
  if (!buffer)
    return NULL;
  
  // Prepare the request.
  req = (Request *) malloc(sizeof(Request));
  memset(req->key, 0, KEY_SIZE);
  memset(req->value, 0, VALUE_SIZE);

  // Extract the operation type.
  token = strtok(buffer, ":");    
  if (!strcmp(token, "PUT")) {
    req->operation = PUT;
  } else if (!strcmp(token, "GET")) {
    req->operation = GET;
  } else {
    free(req);
    return NULL;
  }
  
  // Extract the key.
  token = strtok(NULL, ":");
  if (token) {
    strncpy(req->key, token, KEY_SIZE);
  } else {
    free(req);
    return NULL;
  }
  
  // Extract the value.
  token = strtok(NULL, ":");
  if (token) {
    strncpy(req->value, token, VALUE_SIZE);
  } else if (req->operation == PUT) {
    free(req);
    return NULL;
  }
  return req;
}

void allagi_timwn(struct timeval tv1, struct timeval tv2, struct timeval tv)
{
	struct timeval difference;
	
	 difference.tv_sec =tv1.tv_sec -tv.tv_sec ;
  	 difference.tv_usec =tv1.tv_usec-tv.tv_usec;

	  while(difference.tv_usec<0)
	  {
	    difference.tv_usec+=1000000;
	    difference.tv_sec -=1;
	  }

       pthread_mutex_lock(&twt);
		total_waiting_time = total_waiting_time +  1000000LL*difference.tv_sec+ difference.tv_usec;
		pthread_mutex_unlock(&twt);
  
		pthread_mutex_lock(&tst);
		if( tv1.tv_sec == tv2.tv_sec)
			total_service_time = total_service_time + tv2.tv_usec - tv1.tv_usec;
		else
			total_service_time = total_service_time + tv2.tv_usec + (1000000-tv1.tv_usec);
		pthread_mutex_unlock(&tst);
		
		pthread_mutex_lock(&cr);
		completed_requests++;	
		pthread_mutex_unlock(&cr);
}

void readerr(Request *request,char response_str[BUF_SIZE])
{
	
	
	pthread_mutex_lock(&reader_count_mutex);
    reader_count++;
    if(reader_count==1)
    	pthread_mutex_lock(&db_mutex);
    pthread_mutex_unlock(&reader_count_mutex);
    if (KISSDB_get(db, request->key, request->value))
      sprintf(response_str, "GET ERROR\n");
    else
      sprintf(response_str, "GET OK: %s\n", request->value);
     
	pthread_mutex_lock(&reader_count_mutex);
    reader_count--;
    if(reader_count==0)
    	pthread_mutex_unlock(&db_mutex);
    pthread_mutex_unlock(&reader_count_mutex); 
}

void writerr(Request *request, char response_str[BUF_SIZE])
{
	
	
	pthread_mutex_lock(&db_mutex);
	 
			
    if (KISSDB_put(db, request->key, request->value)) 
      sprintf(response_str, "PUT ERROR\n");
    else
      sprintf(response_str, "PUT OK\n");
	           
            
	pthread_mutex_unlock(&db_mutex);
	
}

/*
 * @name process_request - Process a client request.
 * @param socket_fd: The accept descriptor.
 *
 * @return
 */
void process_request(const int socket_fd, struct timeval tv) {
  char response_str[BUF_SIZE], request_str[BUF_SIZE];
    int numbytes = 0;
    Request *request = NULL;
    struct timeval tv1_end;
    struct timeval tv2_end;
	

	gettimeofday(&tv1_end,NULL); // telos waiting time

    // Clean buffers.
    memset(response_str, 0, BUF_SIZE);
    memset(request_str, 0, BUF_SIZE);
    
    // receive message.
    numbytes = read_str_from_socket(socket_fd, request_str, BUF_SIZE);

    
    // parse the request.
    if (numbytes) {
      request = parse_request(request_str);
      if (request) {
        switch (request->operation) {
          case GET:
            // Read the given key from the database.
            readerr(request,response_str);
		
            break;
          case PUT:
            // Write the given key/value pair to the database.
		
            writerr(request,response_str);
      
            break;
          default:
            // Unsupported operation.
            sprintf(response_str, "UNKOWN OPERATION\n");
        }
        
	    
		gettimeofday(&tv2_end,NULL); // telos service time
	
		allagi_timwn(tv1_end,tv2_end,tv);
        
        
        // Reply to the client.
        write_str_to_socket(socket_fd, response_str, strlen(response_str));
        if (request)
          free(request);
        request = NULL;
        return;
      }
    }
    // Send an Error reply to the client.
    sprintf(response_str, "FORMAT ERROR\n");
    write_str_to_socket(socket_fd, response_str, strlen(response_str));
}



/*
 * @name main - The main routine.
 *
 * @return 0 on success, 1 on error.
 */
int main() {


  
  int socket_fd,              // listen on this socket for new connections
      new_fd;                 // use this socket to service a new connection
  socklen_t clen;
  struct sockaddr_in server_addr,  // my address information
                     client_addr;  // connector's address information




	if(signal(SIGTSTP,sig_handler)==SIG_ERR)
	{
		printf("error \n");
	}


  // create socket
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    ERROR("socket()");


  // Ignore the SIGPIPE signal in order to not crash when a
  // client closes the connection unexpectedly.
  signal(SIGPIPE, SIG_IGN);
  
  // create socket adress of server (type, IP-adress and port number)
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // any local interface
  server_addr.sin_port = htons(MY_PORT);
  
  // bind socket to address
  if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    ERROR("bind()");
  
  // start listening to socket for incomming connections
  listen(socket_fd, MAX_PENDING_CONNECTIONS);
  fprintf(stderr, "(Info) main: Listening for new connections on port %d ...\n", MY_PORT);
  clen = sizeof(client_addr);


  // Allocate memory for the database.
  if (!(db = (KISSDB *)malloc(sizeof(KISSDB)))) {
    fprintf(stderr, "(Error) main: Cannot allocate memory for the database.\n");
    return 1;
  }
  


  // Open the database.
  if (KISSDB_open(db, "mydb.db", KISSDB_OPEN_MODE_RWCREAT, HASH_SIZE, KEY_SIZE, VALUE_SIZE)) {
    fprintf(stderr, "(Error) main: Cannot open the database.\n");
    return 1;
  }

	// dimiourgia nimatwn katanalwtwn
	create_threads();

  // main loop: wait for new connection/requests
  while (1) { 
	
  
  // an h oura einai gemati tote to nima paragwgou prepei na perimenei
   pthread_mutex_lock(&Q_mutex); 
   while(plithos==QUEUE_SIZE)
   	 pthread_cond_wait(&non_full_Queue, &Q_mutex);
   pthread_mutex_unlock(&Q_mutex);
  
    // wait for incomming connection
    if ((new_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &clen)) == -1) {
      ERROR("accept()");
    }

	fprintf(stderr, "(Info) main: Got connection from '%s'\n", inet_ntoa(client_addr.sin_addr));
	pthread_mutex_lock(&Q_mutex);
    enQ(new_fd); // eisagogi neas aitisis stin oura
	pthread_mutex_unlock(&Q_mutex); 
  }  

  
  return 0; 
}

void create_threads()
{
	int i;
  for (i=0;i<THREADS;i++)
  	pthread_create(&tid[i], NULL,katanalotis, NULL); 

}


static void sig_handler(int signo)
{


	// termatismos katanalwtwn
	join_threads();
	
	// kleisimo vasis
	KISSDB_close(db);
	
	  // Free memory.
	  if (db)
	    free(db);
	  db = NULL;

	// ypologismos kai typwma statistikwn apotelesmatwn
	ypologismos();

	exit(0);
}

void ypologismos()
{
	pthread_mutex_lock(&cr);
	if(completed_requests!=0)
	{
		pthread_mutex_lock(&twt);
		printf(" mesos total_waiting_time  %f  \n",(double)(total_waiting_time/completed_requests));
		pthread_mutex_unlock(&twt);
		pthread_mutex_lock(&tst);
		printf(" mesos total_service_time : %f  \n",(double)(total_service_time/completed_requests));
		pthread_mutex_unlock(&tst);
	}

	printf(" completed_requests: %d \n",completed_requests);
	pthread_mutex_unlock(&cr);
}

void join_threads()
{
	int i;
	for(i=0;i<THREADS;i++)
		pthread_kill(tid[i],SIGTSTP); // o paragogos stelnei sima termatismou se olous tous katanalwtes
	
	for(i=0;i<THREADS;i++)
		pthread_join(tid[i],NULL);
}

//Eisagogi stin oura, sto tail tis ouras
void enQ(int new_connection)
{
	struct timeval tv;
	
		tail++;
		plithos++;
		
		gettimeofday(&tv,NULL);
		Q[tail-1].start_time=tv;
		Q[tail-1].connection_fd=new_connection;
		
		if(tail==QUEUE_SIZE)
		{
			tail=0;  // ksana apo tin arxi
		}
		
		if(plithos==1) // gia na ginei to plithos 1 , simainei oti prin htan 0
		// yparxei pithanotita na perimenoun nimata stin metavliti sinthikis
		// non_empty_Queue, kai prepei na ta ksipnisoun
		{
			pthread_cond_signal(&non_empty_Queue);
		}
		
	
	
	return;
}

// eksagogi apo tin oura
// apo to head tis ouras
struct oura * deQ()
{
	struct oura * x = NULL;

	pthread_mutex_lock(&Q_mutex);

	while(plithos == 0)
		pthread_cond_wait(&non_empty_Queue,&Q_mutex);

	head++; // h head proxoraei parakatw
	plithos--; // to plithos twn stoixeiwn tis ouras mwiwnetai
	x = &Q[head-1];
	// kikliki arithmisi, opote an h head exei ftasei sto QUEUE_SIZE
	// tin girnaw sto 0
	if(head==QUEUE_SIZE)
		head=0;

	// prin htan gemati kai tora adeiase mia thesi
	// opote mporei na mpei nea aitisi, gia auto prepei na ksipnisei to nima
	// paragwgou (an koimatai)
	if(plithos==QUEUE_SIZE-1) 
		pthread_cond_signal(&non_full_Queue);
		
	pthread_mutex_unlock(&Q_mutex);

	return x;
}



void *katanalotis(void  *x)
{
	struct oura * xx;
	
	while(1){
		xx=deQ();
		if(xx!=NULL)
			process_request(xx->connection_fd,xx->start_time);
	}
}
