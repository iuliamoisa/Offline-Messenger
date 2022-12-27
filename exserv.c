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
		fprintf(stderr, "Eroare la crearea tabelei1: %s\n", err_msg );
		sqlite3_free(err_msg);//The allocated message string must be freed 
		sqlite3_close(db);
	}
	else fprintf(stdout, "Tabela UtilizatoriInregistrati s-a creat cu succes!\n");
	
	sql = "CREATE TABLE IF NOT EXISTS UtilizatoriAutentificati (user_ID INTEGER PRIMARY KEY, nume_user TEXT NOT NULL);";
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg );
		if(rc != SQLITE_OK)
		{
			fprintf(stderr, "Eroare la crearea tabelei2: %s\n",err_msg );
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

int Inregistrare(char* nume_user, char* parola)
{
	sqlite3 *db;
	sqlite3_stmt * res;//a single sql statement
	char *err_msg = 0;
	int ok = 0;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	char sql[256];
	sprintf(sql, "INSERT INTO UtilizatoriInregistrati (nume_user, parola) VALUES ('%s', '%s');", nume_user, parola);
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	//allows an application to run multiple sql stmts without having to use a lot of C code
	if (rc != SQLITE_OK) {
        fprintf(stderr, "Eroare la inregistrare: %s\n", err_msg);
        sqlite3_free(err_msg);        
        sqlite3_close(db);
    } 
    else {
			ok = 1;
			printf("Inregistrarea a avut loc cu succes!\n");
			sqlite3_close(db);
		}
	return ok;
}
/*
  static int callback(
    char **value_of_count, // will be either 0 or 1 in this case 
    char **label_for_count) { // will be COUNT(*) normally,
                 //but modified via 'AS table_tablename' in this case
      printf("COUNT(*) %s\t=>\t%s\n", label_for_count[0], value_of_count[0] );
      return 0;
    }
*/
int UtilizatorExistentDeja(char* nume_user){
	int rc, ok = 0;
    sqlite3_stmt *res;
	sqlite3 *db;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else {
		sqlite3_prepare_v2(db, "SELECT nume_user, parola FROM UtilizatoriInregistrati WHERE nume_user = ?2;", -1, &res, NULL);
		
		sqlite3_bind_text(res, 2, nume_user, -1, SQLITE_STATIC);

		if((rc = sqlite3_step(res)) == SQLITE_ROW)
			ok = 1; //Acest utilizator exista deja!")
		sqlite3_finalize(res);
    	sqlite3_close(db);
		return ok;
	}
}

int Autentificare(char *nume_user, char *parola)
{
	sqlite3 *db;
	sqlite3_stmt *res; // a single sql statement
	char *err_msg = 0;
	int ok = 0;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	char sql[256];
	sprintf(sql, "INSERT INTO UtilizatoriAutentificati (nume_user) VALUES ('%s');", nume_user);
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	// allows an application to run multiple sql stmts without having to use a lot of C code
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Eroare la autentificare: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
	else
	{
		ok = 1;
		printf("Autentificarea a avut loc cu succes!\n");
		sqlite3_close(db);
	}
	return ok;
}

int UtilizatorAutentificatDeja(char *nume_user)
{
	int rc, ok = 0;
	sqlite3_stmt *res;
	sqlite3 *db;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else
	{
		char* sql = "SELECT nume_user FROM UtilizatoriAutentificati WHERE nume_user = ?2;";
		sqlite3_prepare_v2(db, sql, -1, &res, NULL);

		sqlite3_bind_text(res, 2, nume_user, -1, SQLITE_STATIC);

		if ((rc = sqlite3_step(res)) == SQLITE_ROW)
			ok = 1; // utilizatorul se regaseste in tabela deja
		sqlite3_finalize(res);
		sqlite3_close(db);
		return ok;
	}
}

int DateValide(char *nume_user, char *parola)
{
	sqlite3 *db;
	sqlite3_stmt *res;
	int ok = 0;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else
	{
		sqlite3_prepare_v2(db, "SELECT nume_user, parola FROM UtilizatoriInregistrati WHERE nume_user=?2;", -1, &res, NULL);
		sqlite3_bind_text(res, 2, nume_user, -1, SQLITE_STATIC);
		while (sqlite3_step(res) == SQLITE_ROW)
		{
			const char *numeExtras = sqlite3_column_text(res, 0);
			printf("numele extras: %s\n", numeExtras);
			const char *parolaExtrasa = sqlite3_column_text(res, 1);
			printf("parola extrasa: %s\n", parolaExtrasa);
			if (strcmp(nume_user, numeExtras) == 0 && strcmp(parola, parolaExtrasa) == 0)
			{
				ok = 1;
				break;
			}
		}
	}
	sqlite3_finalize(res);
	sqlite3_close(db);
	return ok;
}
//ATENTIE LA INT RC=... SA VERIFIC ERORI LA FEL PT FIECARE DINTRE FUNCTII
int Deconectare(char* nume_user){
	int rc, ok = 0;
	sqlite3 *db;
	sqlite3_stmt * res;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else
	{
		char* sql = "DELETE FROM UtilizatoriAutentificati WHERE nume_user=?2;";
		rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
		if (rc == SQLITE_OK)
		{
			ok = 1;
			sqlite3_bind_text(res, 2, nume_user, -1, SQLITE_STATIC);
			sqlite3_step(res);
			sqlite3_finalize(res);
		}
		else printf("Ceva nu e ok la deconectare\n");
		sqlite3_close(db);
	}
	return ok;
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
	char raspuns[1000], comanda[50];
	char mesaj[1000];
	char nume_user[25], username[25];
    char parola[25];
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	int autentificat = 0;
	while(1){
		bzero(comanda, sizeof(comanda));
		bzero(nume_user, sizeof(nume_user));
		bzero(parola, sizeof(parola));
		fflush (stdout);
		// citirea mesajului 
        if(read(tdL.cl, &comanda, sizeof(comanda)) <=0){
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la read() de la client.\n");
		}
        if (strlen(comanda) == 0)
            break;
        printf("[Thread %d]Am receptionat comanda: %s\n\n", tdL.idThread, comanda);
//---------------------------------------INREGISTRARE----------------------------------------
		if (strcmp(comanda, "Inregistrare") == 0)
        {
			printf("Clientul %d vrea sa se inregistreze...\n", tdL.idThread);
			if(read(tdL.cl, &nume_user, sizeof(nume_user)) <=0){
				printf("[Thread %d]\n",tdL.idThread);
				perror ("Eroare la citire username de la client.\n");
			}
			else printf("Numele primit este: %s\n", nume_user);
			if(read(tdL.cl, &parola, sizeof(parola)) <=0){
				printf("[Thread %d]\n",tdL.idThread);
				perror ("Eroare la citire parola de la client.\n");
			}
			else printf("Parola primita este: %s\n\n", parola);

			if(UtilizatorExistentDeja(nume_user) || autentificat == 1)//add ceva limita de nr caractere?ma mai gandesc
			{//inreg nu e permisa daca exista deja date sau userul este deja autentificat!
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				if(UtilizatorExistentDeja(nume_user))
					strcpy(raspuns, "Inregistrarea cu acest nume de utilizator nu este permisa!\n");
				else if(autentificat == 1)
					strcpy(raspuns, "Nu va puteti inregistra daca sunteti deja autentificat!\n Pentru a completa cererea, e necesar sa va deconectati mai intai.\n");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
			}		
            else if(Inregistrare(nume_user, parola) == 1) {
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				strcpy(raspuns, "V-ati inregistrat cu succes! \n");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
			}
        } else 
//---------------------------------------AUTENTIFICARE----------------------------------------
        if (strcmp(comanda, "Autentificare") == 0)
        {
            printf("Clientul %d vrea sa se autentifice...\n", tdL.idThread);
			bzero(nume_user, sizeof(nume_user));
			bzero(parola, sizeof(parola));
			fflush (stdout);
			if (read(tdL.cl, &nume_user, sizeof(nume_user)) <= 0)
				{
					printf("[Thread %d]\n", tdL.idThread);
					perror("Eroare la citire username de la client.\n");
				}
			else printf("Numele primit este: %s\n", nume_user);
			
			if (read(tdL.cl, &parola, sizeof(parola)) <= 0)
				{
					printf("[Thread %d]\n", tdL.idThread);
					perror("Eroare la citire parola de la client.\n");
				}
			else printf("Parola primita este: %s\n\n", parola);
			
			if(UtilizatorAutentificatDeja(nume_user) == 1 || autentificat == 1)
			{
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				if(autentificat == 1) strcpy(raspuns, "Sunteti autentificat deja!\n");
				else if(UtilizatorAutentificatDeja(nume_user) == 1)
					strcpy(raspuns, "Datele introduse sunt asociate unui cont autentificat deja!\n");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
			}
			else { //poate avea loc autentif; verif daca datele sunt corecte 
				if(DateValide(nume_user, parola) == 0)
				{
					bzero(raspuns, sizeof(raspuns));
					fflush (stdout);
					strcpy(raspuns, "Datele introduse nu sunt valide. Cererea a fost respinsa.");
					if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
						{
						printf("[Thread %d] ",tdL.idThread);
						perror ("[Thread]Eroare la write() catre client.\n");
						}
					else
						printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
				}
				else if(Autentificare(nume_user, parola) == 1)
					{
						autentificat = 1;
						strcpy(username, nume_user);
						bzero(raspuns, sizeof(raspuns));
						fflush (stdout);
						strcpy(raspuns, "V-ati autentificat cu succes! \n");
						if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0){
							printf("[Thread %d] ", tdL.idThread);
							perror("[Thread]Eroare la write() catre client.\n");
						}
						else printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
					}
				}

			}//sfarsit autentificare
		/*
			if(DateValide(nume_user, parola) == 1)
			{//
				//transmitem ca datele introduse sunt corecte
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				strcpy(raspuns, "Datele introduse sunt valide. \n");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
				//verif daca nu e deja autentificat
				if(UtilizatorAutentificatDeja(nume_user) == 1 || autentificat == 1){
					bzero(raspuns, sizeof(raspuns));
					fflush (stdout);
					strcpy(raspuns, "Sunteti autentificat deja! \n");
					if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
						printf("[Thread %d] ", tdL.idThread);
						perror("[Thread]Eroare la write() catre client.\n");
					}
					else
						printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
				}
				else if(Autentificare(nume_user, parola) == 1){
					autentificat = 1;
					bzero(raspuns, sizeof(raspuns));
					fflush (stdout);
					strcpy(raspuns, "V-ati autentificat cu succes! \n");
					if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0){
						printf("[Thread %d] ", tdL.idThread);
						perror("[Thread]Eroare la write() catre client.\n");
					}
					else printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
				}
			} //
			else {//date incorecte
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				strcpy(raspuns, "Datele introduse nu sunt valide. Cererea a fost respinsa.");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
				
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				strcpy(raspuns, "Incercati din nou! \n");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
			}
		}*/

		//---------------------------------------DECONECTARE----------------------------------------
		else if (strcmp(comanda, "Deconectare") == 0){
			printf("Clientul %d (%s) vrea sa se deconecteze...\n", tdL.idThread, username);
			/* MA GANDESC CA AR TRB SA STIU DEJA NUMELE !!...!!
			bzero(nume_user, sizeof(nume_user));
			fflush (stdout);
			if (read(tdL.cl, &nume_user, sizeof(nume_user)) <= 0)
				{
					printf("[Thread %d]\n", tdL.idThread);
					perror("Eroare la citire username de la client.\n");
				}
			else printf("Numele primit este: %s\n", nume_user);
			*/
			bzero(raspuns, sizeof(raspuns));
			fflush (stdout);
			
			if(autentificat == 1 && Deconectare(username) == 1)
			{
				autentificat = 0;
				strcpy(raspuns, "V-ati deconectat cu succes!");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				} else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);

			}
			else 
				{
				printf("DECONECTAREA A ESUAT\n");
				strcpy(raspuns, "Deconectarea nu poate fi realizata.");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				else
					printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
				}
		}//sfarsit deconectare
		else {
			printf("Comanda neidentificata.\n");
			bzero(raspuns, sizeof(raspuns));
			fflush (stdout);
			strcpy(raspuns, "Comanda introdusa nu este recunoscuta. Incercati din nou!\n");
		}
		
	}
}
