#include <stdio.h>
#include <stdlib.h>

#include <mysql/mysql.h>

//static  *host = "localhost";
//static char *user = "root";
//static char *pass = "distributedpass";
//static char *dbname = "bank";

unsigned int port = 3306;
static char *unix_socket = NULL;
unsigned int flag = 0;

int main(){
	MYSQL *conn;

	MYSQL_RES *res;
    MYSQL_ROW row;

	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, "localhost", "root", "distributedpass", "bank", port, unix_socket, flag)){

		fprintf(stderr, "%s [%d]\n", mysql_error(conn),mysql_errno(conn) );
		printf("errore");
		exit(1);
	}

	printf("Connection is ok!\n");

	if (mysql_query(conn, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }


   res = mysql_use_result(conn);

   /* output table name */
   printf("MySQL Tables in mysql database:\n");
   while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]);

   /* close connection */
   mysql_free_result(res);
   mysql_close(conn);

	return EXIT_SUCCESS;


}