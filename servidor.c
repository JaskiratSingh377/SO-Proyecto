#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <mysql.h>
#include <netinet/in.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	MYSQL *conn;
	int err;
	
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	
	char consulta [80];
	//Creamos una conexion al servidor MYSQL 
	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	//inicializar la conexion, entrando nuestras claves de acceso y
	//el nombre de la base de datos a la que queremos acceder 
	conn = mysql_real_connect (conn, "localhost","root", "mysql", "bd",0, NULL, 0);
	if (conn==NULL) 
	{
		printf ("Error al inicializar la conexion: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	char peticion[512];
	char respuesta[512];
	
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(7900);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen");
	
	int i;
	// Bucle infinito
	for (;;)
	{
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		//sock_conn es el socket que usaremos para este cliente
		
		int terminar =0;
		// Entramos en un bucle para atender todas las peticiones de este cliente
		//hasta que se desconecte
		while (terminar ==0)
		{
		
			// Ahora recibimos la petici?n
			ret=read(sock_conn,peticion, sizeof(peticion));
			printf ("Recibido\n");
			
			// Tenemos que a?adirle la marca de fin de string 
			// para que no escriba lo que hay despues en el buffer
			peticion[ret]='\0';
			
			
			printf ("Peticion: %s\n",peticion);
			
			// vamos a ver que quieren
			char *p = strtok( peticion, "/");
			int codigo =  atoi (p);
			// Ya tenemos el c?digo de la petici?n
			char username[20];
			char password[20];
			
			if (codigo !=0)
			{
				p = strtok( NULL, "/");
				
				strcpy (username, p);
				
				p = strtok( NULL, "/");
				strcpy (password, p);
				// Ya tenemos el username y el password
				// printf ("Codigo: %d, username: %s, password:UNKNOWN\n", codigo,username);

			}
			
			if (codigo ==0) //petici?n de desconexi?n
				terminar=1;
			else if (codigo ==1) //piden la longitd del nombre
			{
			
					p = strtok( NULL, "/");
					strcpy (password, p);
					
					sprintf (respuesta,"%d",strlen (username));
					
					strcpy (consulta, "SELECT Jugador.ID FROM (Jugador) WHERE Jugador.PASSWORD='");
					strcat (consulta, password); 
					strcat (consulta, "';");
					
					printf("consulta = %s\n", consulta);
					
					err = mysql_query(conn, consulta);
					if (err!=0)
					{
						printf ("El USERNAME y el PASSWORD no coinciden %u %s\n", mysql_errno(conn), mysql_error(conn));
						exit (1);
					}
					
					resultado = mysql_store_result (conn);
					row = mysql_fetch_row (resultado);
					
					if (row == NULL)
						printf ("El USERNAME y el PASSWORD no coinciden\n");
					else
						while (row !=NULL) 
					{
							printf ("Username: %s\n", row[1]);
							
							row = mysql_fetch_row (resultado);
							
							strcpy(respuesta,"Correctamente");
					}
						
					
						exit(0);
						write (sock_conn,respuesta, strlen(respuesta));
			}
			
			else if (codigo == 2)
			{
				printf("Aquí tienes una lista con los jugadores que actualmente hay en el sistema\n");
				err=mysql_query (conn, "SELECT * FROM Jugador");
				if (err!=0) 
				{
					printf ("Error al consultar datos de la base %u %s\n",
							mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				
				resultado = mysql_store_result (conn);
				
				row = mysql_fetch_row (resultado);
				
				if (row == NULL)
					printf ("No se han obtenido datos en la consulta\n");
				else
					while (row !=NULL) 
				{
						
						printf ("Username: %s\n", row[1]);

						row = mysql_fetch_row (resultado);
				}
					
				
					printf("\n");
					 
					strcpy (consulta, "DELETE FROM Jugador WHERE Jugador.USERNAME='");
				 
					strcat (consulta, username); 
					strcat (consulta, "';");
					
					
					printf("consulta = %s\n", consulta);
					
					strcpy(respuesta,"El usuario ha sido eliminado de la base de datos ");
				
					err = mysql_query(conn, consulta);
					if (err!=0)
					{
						printf ("Error al introducir datos la base %u %s\n", mysql_errno(conn), mysql_error(conn));
						strcpy(respuesta,"El usuario NO ha sido eliminado de la base de datos ");
						exit (1);
					}
					
					printf("\n");
					printf("Despues de dar baja al jugador deseado la BBDD queda de la siguiente forma:\n");
					err=mysql_query (conn, "SELECT * FROM Jugador");
					if (err!=0) 
					{
						printf ("Error al consultar datos de la base %u %s\n",
								mysql_errno(conn), mysql_error(conn));
						exit (1);
					}
					
					resultado = mysql_store_result (conn);
					
					row = mysql_fetch_row (resultado);

					if (row == NULL)
						printf ("No se han obtenido datos en la consulta\n");
					else
						while (row !=NULL) 
					{
							 
							printf ("Username: %s\n", row[1]);
							row = mysql_fetch_row (resultado);							
					}
						
						mysql_close (conn);
			}
			
			
				//}
		}
		// Se acabo el servicio para este cliente
		close(sock_conn); 
		mysql_close (conn);
	}
}

