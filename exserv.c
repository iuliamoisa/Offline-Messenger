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
#include <string.h>
#include "sqlite3.h"
#define _OPEN_SYS_ITOA_EXT

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
        sql = "CREATE TABLE IF NOT EXISTS Mesaje(mesaj_ID INTEGER PRIMARY KEY, expeditor TEXT NOT NULL, destinatar TEXT NOT NULL, continut_mesaj TEXT NOT NULL);";
	rc=sqlite3_exec(db, sql, 0 ,0, &err_msg);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Eroare la crearea tabelei3: %s\n",err_msg );
		sqlite3_free(err_msg );
	}
	else fprintf(stdout, "Tabela Mesaje s-a creat cu succes!\n");
 
        sql = "CREATE TABLE IF NOT EXISTS MesajeNoi(mesaj_ID INTEGER PRIMARY KEY, expeditor TEXT NOT NULL, destinatar TEXT NOT NULL, continut_mesaj TEXT NOT NULL);";
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg );
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Eroare la crearea tabelei4: %s\n",err_msg );
		sqlite3_free(err_msg );
	}
	else fprintf(stdout, "Tabela MesajeNoi s-a creat cu succes!\n");
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

int Inserare_TabMesaje(char* expeditor, char* destinatar, char* mesaj){
	//inserare in tabela mesaje -- contine toate mesajele
	sqlite3 *db;
	sqlite3_stmt * res;
	char *err_msg = 0;
	int ok = 0, rc;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	char sql[256];
	sprintf(sql, "INSERT INTO Mesaje (expeditor, destinatar, continut_mesaj) VALUES ('%s', '%s', '%s');", expeditor, destinatar, mesaj);
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
        fprintf(stderr, "Eroare la inserare in Mesaje: %s\n", err_msg);
        sqlite3_free(err_msg);        
        //sqlite3_close(db);
    } 
    else {
			ok = 1;
			sqlite3_close(db);
		}
	return ok;
}

int Inserare_TabMesajeNoi(char* expeditor, char* destinatar, char* mesaj){
	//inserare in tabela mesaje -- contine toate mesajele
	sqlite3 *db;
	sqlite3_stmt * res;
	char *err_msg = 0;
	int ok = 0, rc;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	char sql[256];
	sprintf(sql, "INSERT INTO MesajeNoi (expeditor, destinatar, continut_mesaj) VALUES ('%s', '%s', '%s');", expeditor, destinatar, mesaj);
	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
        fprintf(stderr, "Eroare la inserare in MesajeNoi: %s\n", err_msg);
        sqlite3_free(err_msg);        
        sqlite3_close(db);
    } 
    else {
			ok = 1;
			sqlite3_close(db);
		}
	return ok;
}

int Trimite_Mesaj(char* expeditor, char* destinatar, char* mesaj){
	if(Inserare_TabMesaje(expeditor, destinatar, mesaj) == 0 || Inserare_TabMesajeNoi(expeditor, destinatar, mesaj) == 0)
		return 0;
	return 1;
	
}

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
			ok = 1; //Acest utilizator exista deja!
		sqlite3_finalize(res);
    	sqlite3_close(db);
		return ok;
	}
}

int Utilizatori(char utilizatori[200][200], int opt){
	int rc, nr = 0;
    sqlite3_stmt *res;
	sqlite3 *db;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else {
		char *sql;
		if (opt == 1)
			sql = "SELECT nume_user FROM UtilizatoriInregistrati;";
		else if(opt == 2)
			sql = "SELECT nume_user FROM UtilizatoriAutentificati;";
		rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);		
		 if (rc != SQLITE_OK) 
        	//sqlite3_bind_text(res, 3, destinatar, -1, SQLITE_STATIC);//setez a 3a coloana=dest
		 //else 
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		
		while (sqlite3_step(res) != SQLITE_DONE) {
			const char* user = sqlite3_column_text(res, 0);
			printf("%s\n ", user);
			//const char* informatie = sqlite3_column_text(res, 1);
			//printf("%s\n", informatie);
			char* utilizator;
			bzero(utilizator, sizeof(utilizator));
			strcat(utilizator, user);
			///strcat(utilizator," : ");
			//strcat(utilizator, informatie);
			strcat(utilizator," \n ");
			strcpy(utilizatori[nr], utilizator);
			nr++;
		} 
	}
	return nr;
	sqlite3_finalize(res);
    sqlite3_close(db);
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
			const char *parolaExtrasa = sqlite3_column_text(res, 1);
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

int Afisare_MesajeOffline(char *destinatar, char mesaje_offline[200][200]){
	sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    int nr = 0;
    int rc = sqlite3_open("OM_BazaDeDate.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
	else {
		char *sql = "SELECT mesaj_ID, expeditor, continut_mesaj FROM MesajeNoi WHERE destinatar= ?3;";
		rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);		
		 if (rc == SQLITE_OK) 
        	sqlite3_bind_text(res, 3, destinatar, -1, SQLITE_STATIC);//setez a 3a coloana=dest
		 else 
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		while (sqlite3_step(res) != SQLITE_DONE) {
			const int id = sqlite3_column_int(res, 0);
			int lg = snprintf( NULL, 0, "%d", id);
			char* id_mesaj = malloc( lg + 1 );
			snprintf(id_mesaj, lg + 1, "%d", id );
    		printf("%s. ", id_mesaj);
			const char* user = sqlite3_column_text(res, 1);
			printf("%s: ", user);
			const char* informatie = sqlite3_column_text(res, 2);
			printf("%s\n", informatie);
			char* mesaj;
			//id. user : mesaj 
			bzero(mesaj, sizeof(mesaj));
			strcat(mesaj, id_mesaj);
			strcat(mesaj, ". ");
			strcat(mesaj, user);
			strcat(mesaj," : ");
			strcat(mesaj, informatie);
			strcat(mesaj," \n ");
			strcpy(mesaje_offline[nr], mesaj);
			nr++;
		} 
	}
	sqlite3_finalize(res);
    sqlite3_close(db);
	return nr;
}

int AfisareIstoric(char* a, char* b, char conversatii[200][200]){
	sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    int nr = 0;
    int rc = sqlite3_open("OM_BazaDeDate.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
	else {
//Mesaje(mesaj_ID  expeditor  destinatar  continut_mesaj 
		char *sql = "SELECT mesaj_ID, expeditor, continut_mesaj FROM Mesaje WHERE (expeditor=?2 AND destinatar=?3) OR (expeditor=?3 AND destinatar=?2);";
		rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);		
		 if (rc == SQLITE_OK) {
			sqlite3_bind_text(res, 2, a, -1, SQLITE_STATIC);
        	sqlite3_bind_text(res, 3, b, -1, SQLITE_STATIC);
		 }
		 else 
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
		
		while (sqlite3_step(res) != SQLITE_DONE) {
			const int id = sqlite3_column_int(res, 0);
			int lg = snprintf( NULL, 0, "%d", id);
			char* id_mesaj = malloc( lg + 1 );
			snprintf(id_mesaj, lg + 1, "%d", id );
			printf("%s. ", id_mesaj);
			const char* user = sqlite3_column_text(res, 1);
			printf("%s: ", user);
			const char* informatie = sqlite3_column_text(res, 2);
			printf("%s\n", informatie);
			char* mesaj;
			bzero(mesaj, sizeof(mesaj));
			strcat(mesaj, id_mesaj);
			strcat(mesaj, ". ");
			strcat(mesaj, user);
			strcat(mesaj," : ");
			strcat(mesaj, informatie);
			strcat(mesaj," \n ");
			strcpy(conversatii[nr], mesaj);
			nr++;
		} 
	}
	sqlite3_finalize(res);
    sqlite3_close(db);
	return nr;
}

void stergeMesajeOffline(char* destinatar){
	sqlite3 *db;
	sqlite3_stmt * res;
	if (sqlite3_open("OM_BazaDeDate.db", &db) != SQLITE_OK)
	{
		fprintf(stderr, "Baza de date nu poate fi deschisa: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else {
		char* sql = "DELETE FROM MesajeNoi WHERE destinatar=?3;";
		int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
		if (rc == SQLITE_OK)
		{
			printf("stergem mesajeeeeee\n");
			sqlite3_bind_text(res, 3, destinatar, -1, SQLITE_STATIC);
			sqlite3_step(res);
			sqlite3_finalize(res);
		}
		else printf("Ceva nu e ok la stergere mesaje\n");
		sqlite3_close(db);
	}
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
	char raspuns[1000], comanda[50], mesaj[500];
	char nume_user[25], username[25], destinatar[25];
    char parola[25];
    int nr;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	int autentificat = 0;
	int pot_primi_mesaj, opt, ok;
	while(1){
		if (write (tdL.cl, &autentificat, sizeof(autentificat)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la trimitere stare catre client.\n");
					}
		//DACA SUNT AUTENTIFICAT, FAC O VERIFICARE DACA EXISTA MSJ NOI
		//adica if(autentificat==1 && verificamsjnoi(...)==1){
		//	read si write
		//}
//---------------
		bzero(comanda, sizeof(comanda));
		bzero(nume_user, sizeof(nume_user));
		bzero(parola, sizeof(parola));
		fflush (stdout);
		// citirea mesajului 
        if(read(tdL.cl, &comanda, sizeof(comanda)) <=0){
			printf("[Thread %d]\n",tdL.idThread);
			perror ("Eroare la citire comanda de la client.\n");
		}
        if (strlen(comanda) == 0)
            break;
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
				pot_primi_mesaj = 0;
				if (write(tdL.cl, &pot_primi_mesaj, sizeof(pot_primi_mesaj)) <= 0){//echivalent cu var ''da'' din client
						printf("[Thread %d] ", tdL.idThread);
						perror("[Thread]Eroare la write() catre client.\n");
					}

				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				if(autentificat == 1) 
					strcpy(raspuns, "Sunteti autentificat deja!\n");
				else if(UtilizatorAutentificatDeja(nume_user) == 1)
					strcpy(raspuns, "Datele introduse sunt asociate unui cont autentificat deja!\n");
				
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				
			}
			else { //poate avea loc autentif; verif daca datele sunt corecte 
				if(DateValide(nume_user, parola) == 0)
				{
					pot_primi_mesaj = 0;
					if (write(tdL.cl, &pot_primi_mesaj, sizeof(pot_primi_mesaj)) <= 0){//echivalent cu var ''da'' din client
						printf("[Thread %d] ", tdL.idThread);
						perror("[Thread]Eroare la write() catre client.\n");
					}
					bzero(raspuns, sizeof(raspuns));
					fflush (stdout);
					strcpy(raspuns, "Datele introduse nu sunt valide. Cererea a fost respinsa.");
					if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
						{
						printf("[Thread %d] ",tdL.idThread);
						perror ("[Thread]Eroare la write() catre client.\n");
						}
				}
				else if(Autentificare(nume_user, parola) == 1)
					{
						autentificat = 1;
						pot_primi_mesaj = 1;
						if (write(tdL.cl, &pot_primi_mesaj, sizeof(pot_primi_mesaj)) <= 0){//echivalent cu var ''da'' din client
							printf("[Thread %d] ", tdL.idThread);
							perror("[Thread]Eroare la write() catre client.\n");
						}						
						strcpy(username, nume_user);
						bzero(raspuns, sizeof(raspuns));
						fflush (stdout);
						strcpy(raspuns, "V-ati autentificat cu succes! \n");
						if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0){
							printf("[Thread %d] ", tdL.idThread);
							perror("[Thread]Eroare la write() catre client.\n");
						}
					}
				}
				if(pot_primi_mesaj == 1){
					char mesaje_offline[200][200];
					int nrmesajeoffline = Afisare_MesajeOffline(username, mesaje_offline);
					printf("Nr mesaje primite offline = %d\n", nrmesajeoffline);
					if(nrmesajeoffline == 0){
						bzero(raspuns, sizeof(raspuns));
						fflush (stdout);
						strcpy(raspuns, "Nu exista mesaje primite in timp ce nu erati online. \n");
						if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0){
							printf("[Thread %d] ", tdL.idThread);
							perror("[Thread]Eroare la write() catre client.\n");
						}
					}
					else {
						bzero(raspuns, sizeof(raspuns));
						fflush (stdout);
						strcat(raspuns, "- Cat timp erati offline ati primit mesajele: \n");
						for(int i = 0; i < nrmesajeoffline; i++){
							strcat(raspuns,"\n");
							strcat(raspuns,mesaje_offline[i]);
						}
						if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
						{
							printf("[Thread %d] ",tdL.idThread);
							perror ("[Thread]Eroare la write() catre client.\n");
						}
						stergeMesajeOffline(username);
					}
					
				}
			}//sfarsit autentificare

//---------------------------------------DECONECTARE----------------------------------------
		else if (strcmp(comanda, "Deconectare") == 0){
			printf("Clientul %d (%s) vrea sa se deconecteze...\n", tdL.idThread, username);
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
				}
			}
			else 
				{
				strcpy(raspuns, "Deconectarea nu poate fi realizata.");
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
					{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
					}
				}
		}//sfarsit deconectare
//---------------------------------------TRIMITE MESAJ----------------------------------------		
		else if(strcmp(comanda, "Trimite_Mesaj") == 0){
			bzero(destinatar, sizeof(destinatar));
			printf("Cineva vrea sa trimita un mesaj...\n");
			if (read(tdL.cl, &destinatar, sizeof(destinatar)) <= 0)
				{
					printf("[Thread %d]\n", tdL.idThread);
					perror("Eroare la citire nume destinatar de la client.\n");
				}
			printf("Clientul %d (%s) vrea sa trimita un mesaj userului %s.\n", tdL.idThread, username, destinatar);
		
			bzero(raspuns, sizeof(raspuns));
			fflush (stdout);
			int exista;
			if(UtilizatorExistentDeja(destinatar) == 0){
				exista = 0;
				if (write (tdL.cl, &exista, sizeof(exista)) <= 0)
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				} 
			}
			else {//utilizatorul exista; poate fi transmis mesajul
			//username=expeditor; 
				exista = 1;
				if (write (tdL.cl, &exista, sizeof(exista)) <= 0)
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				} 
				bzero(mesaj, 500);
				//citesc mesajul si il adaug in tabelele mesaje+mesajeNoi
				if (read(tdL.cl, &mesaj, sizeof(mesaj)) <= 0)
				{
					printf("[Thread %d]\n", tdL.idThread);
					perror("Eroare la citire mesaj transmis de client.\n");
				}
				else {
					printf("Mesajul primit este: %s\n", mesaj);
					if(Trimite_Mesaj(username, destinatar, mesaj) == 0)
					{
						printf("Mesajul nu s-a trimis\n");
					}
					else {
						printf("Mesajul s-a trimis\n");
					}
				}

			}
		}
//---------------------------------------AFISARE UTILIZATORI INREGISTRATI----------------------------------------		
		else if (strcmp(comanda, "Utilizatori") == 0){
			opt = 1;
			char toti_userii[200][200];
			int nr_useri_total = Utilizatori(toti_userii, opt);
			if (write(tdL.cl, &nr_useri_total, sizeof(nr_useri_total)) <= 0){
				printf("[Thread %d] ", tdL.idThread);
				perror("[Thread]Eroare la write() catre client.\n");
			}
			if(nr_useri_total != 0){
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				strcat(raspuns, "-- Utilizatorii inregistrati sunt:");
				for(int i = 0; i < nr_useri_total; i++){
					strcat(raspuns, "\n");
					//add nr utilizatorului in fata; un strcpy 
					strcat(raspuns, toti_userii[i]);
				}
				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				}
			}

		}
//---------------------------------------AFISARE UTILIZATORI ONLINE----------------------------------------		
		else if (strcmp(comanda, "Utilizatori_Online") == 0){
			opt = 2;
			char utilizatori_on[200][200];
			int nr_useri_on = Utilizatori(utilizatori_on, opt);
			if (write(tdL.cl, &nr_useri_on, sizeof(nr_useri_on)) <= 0){
				printf("[Thread %d] ", tdL.idThread);
				perror("[Thread]Eroare la write() catre client.\n");
			}
			if(nr_useri_on != 0){
				bzero(raspuns, sizeof(raspuns));
				fflush (stdout);
				strcat(raspuns, "-- Utilizatorii online sunt:");
				for(int i = 0; i < nr_useri_on; i++){
					strcat(raspuns, "\n");
					//add nr utilizatorului in fata; un strcpy 
					strcat(raspuns, utilizatori_on[i]);
				}

				if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				}
			}
		}
//---------------------------------------AFISARE ISTORIC CU UN ANUMIT USER----------------------------------------		
		else if (strcmp(comanda, "Afiseaza_Istoric") == 0){
			printf("Clientul %d (%s) vrea sa accese un istoricul...", tdL.idThread, username);
				if(read(tdL.cl, &nume_user, sizeof(nume_user)) <=0){
					printf("[Thread %d]\n",tdL.idThread);
					perror ("Eroare la citire username de la client.\n");
				}
				else printf("cu userul: %s\n", nume_user);
			ok = UtilizatorExistentDeja(nume_user);
			
			if (write (tdL.cl, &ok, sizeof(ok)) <= 0) //pot vedea istoricul sau nu
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				} 
			if(ok == 1){
				char conversatii[200][200];
				int nr_mesaje = AfisareIstoric(username, nume_user, conversatii);
				if(nr_mesaje) //nume_user=utilizatorul cu care vreau sa vad conv
				{
					//write conversatie
						bzero(raspuns, sizeof(raspuns));
						fflush (stdout);
						strcat(raspuns, "- Istoricul conversatiei: \n");
						for(int i = 0; i < nr_mesaje; i++){
							strcat(raspuns,"\n");
							strcat(raspuns,conversatii[i]);
						}
						if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
						{
							printf("[Thread %d] ",tdL.idThread);
							perror ("[Thread]Eroare la write() catre client.\n");
						}
				}
				else {
					//write nu exista conversatii cu userul
						bzero(raspuns, sizeof(raspuns));
						fflush (stdout);
						strcpy(raspuns, "Nu exista conversatii cu acest utilizator. \n");
						if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0){
							printf("[Thread %d] ", tdL.idThread);
							perror("[Thread]Eroare la write() catre client.\n");
						}
				}



			}
		}
//---------------------------------------IESIRE----------------------------------------		
		else if (strcmp(comanda, "Iesire") == 0){
			bzero(raspuns, sizeof(raspuns));
			fflush (stdout);
			if (read(tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
				{
					printf("[Thread %d]\n", tdL.idThread);
					perror("Eroare la citire info de la client.\n");
				}
			else printf("Clientul %d (%s) a transmis mesajul: %s\n", tdL.idThread, username, raspuns);
		}
//---------------------------------------COMANDA NERECUNOSCUTA----------------------------------------		
		else { //alta comanda
			printf("Comanda neidentificata.\n");
			bzero(raspuns, sizeof(raspuns));
			fflush (stdout);
			strcpy(raspuns, "Comanda introdusa nu este recunoscuta. Incercati din nou!\n");
			if (write (tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
				{
					printf("[Thread %d] ",tdL.idThread);
					perror ("[Thread]Eroare la write() catre client.\n");
				}
		}//else
		
	}
}
