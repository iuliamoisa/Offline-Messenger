#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "sqlite3.h"

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
	int user_ID;
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

void creareTabele(){
	char *err_msg = 0;
	sqlite3* db;
	char *sql;
	int rc = sqlite3_open("OM_BazaDeDate.db", &db); //deschidere baza de date
	if(rc != SQLITE_OK) {
		fprintf(stderr, "baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
		}
    else
        printf("Baza de date a fost deschisa cu succes\n");
    //creare tabele
    sql = "CREATE TABLE IF NOT EXISTS UtilizatoriInregistrati (user_ID INTEGER PRIMARY KEY NOT NULL, nume_user TEXT NOT NULL, parola TEXT NOT NULL);";
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg );
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", err_msg );
		sqlite3_free(err_msg);//The allocated message string must be freed 
		sqlite3_close(db);
	}
	else fprintf(stdout, "Tabela UtilizatoriInregistrati s-a creat cu succes!\n");
	
	sql = "CREATE TABLE IF NOT EXISTS UtilizatoriAutentificati (user_ID INTEGER PRIMARY KEY, username varchar(100));";
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg );
		if(rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n",err_msg );
			sqlite3_free(err_msg );
		}
	else fprintf(stdout, "Tabela UtilizatoriAutentificati s-a creat cu succes!\n");
/*
        sql = "CREATE TABLE IF NOT EXISTS Mesaje(mesaj_ID INTEGER PRIMARY KEY, expeditor varchar(100), destinatar varchar(100), continut_mesaj varchar(100));";
	rc=sqlite3_exec(db, sql, 0 ,0, &err_msg);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n",err_msg );
		sqlite3_free(err_msg );
	}
	else fprintf(stdout, "Tabela Mesaje s-a creat cu succes!\n");
 
        sql = "CREATE TABLE IF NOT EXISTS MesajeNoi(mesaj_ID INTEGER PRIMARY KEY, expeditor varchar(100), destinatar varchar(100), continut_mesaj varchar(100));";
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg );
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n",err_msg );
		sqlite3_free(err_msg );
	}
	else fprintf(stdout, "Tabela MesajeNoi s-a creat cu succes!\n");
*/
}
/*
void Inregistrare(char* nume_user, char* parola)
{
	sqlite3 *db;
	sqlite3_stmt * res;//a single sql statement
	char *err_msg = 0;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "BD nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else //if (sqlite3_open("OM_BazaDeDate.db", &db) == SQLITE_OK)
	{
		printf("BD e gata de folosire; incerc sa inregistrez userul\n");
		char* sql = "INSERT INTO UtilizatoriInregistrati (nume_user, parola) VALUES (?, ?);";
		int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);//compiling statement into
  //a byte code before execution
		if (rc != SQLITE_OK ) {
			fprintf(stderr, "SQL error: %s\n", err_msg);
			sqlite3_free(err_msg);        
			sqlite3_close(db);
    	} 
		else //if (rc == SQLITE_OK)
		{ 
			printf("Inregistraaaaaaam...\n");
			//sqlite3_bind_blob(res, 2, nume_user, strlen(nume_user), SQLITE_TRANSIENT); // store application data into parameters 
			//sqlite3_bind_blob(res, 3, parola, strlen(parola),  SQLITE_TRANSIENT);
			if(sqlite3_bind_text(res, 2, nume_user, strlen(nume_user), SQLITE_TRANSIENT) != SQLITE_OK){
				printf("nu e ceva ok cu numele\n");
			}
			if(sqlite3_bind_text(res, 3, parola, strlen(parola), SQLITE_TRANSIENT) != SQLITE_OK){
				printf("nu e ceva ok cu numele\n");
			}
			if(sqlite3_step(res) != SQLITE_DONE){
				printf("execution failed: %s", sqlite3_errmsg(db));
			}
			sqlite3_finalize(res);
			printf("Inregistrarea a avut loc cu succes!\n");
		}
	}
} */

void Inregistrare(char* nume_user, char* parola)
{
	sqlite3 *db;
	sqlite3_stmt * res;//a single sql statement
	char *err_msg = 0;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	char sql[256];
	sprintf(sql, "INSERT INTO UtilizatoriInregistrati (nume_user, parola) VALUES ('%s', '%s');", nume_user, parola);
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	//allows an application to run multiple sql stmts without having to use a lot of C code
	if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
    } 
    else printf("Inregistrarea a avut loc cu succes!\n");
    sqlite3_close(db);
}


int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 10) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
	creareTabele();
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
		{
		perror ("[server]Eroare la accept().\n");
		continue;
		}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

		td=(struct thData*)malloc(sizeof(struct thData));	
		td->idThread=i++;
		td->cl=client;

		pthread_create(&th[i], NULL, &treat, td);	    

		/* Reduce CPU usage */
					sleep(1);  
					
		}//while    
};		

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]-  Hello client %d!!\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		// am terminat cu acest client, inchidem conexiunea 
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
	char raspuns[1000];
	char mesaj[1000];
	char nume_user[25];
    char parola[25];
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	while(1){
		bzero(raspuns, sizeof(raspuns));
		bzero(nume_user, sizeof(nume_user));
		bzero(parola, sizeof(parola));
		fflush (stdout);
		// citirea mesajului 
        if(read(tdL.cl, &raspuns, sizeof(raspuns)) <=0){
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
        if (strlen(raspuns) == 0)
            break;
        printf("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, raspuns);
//---------------------------------------INREGISTRARE----------------------------------------
		if (strcmp(raspuns, "Inregistrare") == 0)
        {
			printf("Vrea sa se inregistreze...\n");
			if(read(tdL.cl, &nume_user, sizeof(nume_user)) <=0){
				printf("[Thread %d]\n",tdL.idThread);
				perror ("Eroare la read() de la client.\n");
			}
			else printf("Numele primit este: %s\n", nume_user);
			if(read(tdL.cl, &parola, sizeof(parola)) <=0){
				printf("[Thread %d]\n",tdL.idThread);
				perror ("Eroare la read() de la client.\n");
			}
			else printf("Parola primita este: %s\n", parola);
            Inregistrare(nume_user, parola);
        } else 
//---------------------------------------AUTENTIFICARE----------------------------------------
        if (strcmp(raspuns, "Autentificare") == 0)
        {
            printf("Vrea sa se autentifice...\n");
        }else printf("Comanda introdusa nu este recunoscuta. Incercati din nou!\n");
		/*
		
					
					//pregatim mesajul de raspuns 
					nr++;      
			printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);
					
					
					// returnam mesajul clientului 
			if (write (tdL.cl, &nr, sizeof(int)) <= 0)
				{
				printf("[Thread %d] ",tdL.idThread);
				perror ("[Thread]Eroare la write() catre client.\n");
				}
			else
				printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
				*/
	}
	

}
