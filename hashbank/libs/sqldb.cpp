// SQLDB -- sudo apt-get install sqlite3 libsqlite3-dev

#include <cstring>
#include <iostream>
#include "sqldb.h"

// Declare the static variables
std::vector<std::string> SQLDB::sql_columns;  // Columns
std::vector<std::string> SQLDB::sql_vals;     // Results

SQLDB::SQLDB(std::string db_name)
{
	this->db_name = db_name;

	// Open database
	open_db();
}

SQLDB::~SQLDB()
{
	sqlite3_close(db);

	std::cout << "DB closed!" << std::endl;
}

int SQLDB::callback(void *data, int argc, char **argv, char **azColName) {

	for(int i = 0; i<argc; i++) {
		
		// Cast char* to string
		std::string strval(argv[i]);
		std::string strcol(azColName[i]);

		// Save to vector
		sql_columns.push_back(strcol);
		sql_vals.push_back(strval);
	}

   return 0;
}

int SQLDB::open_db()
{
	// Filename
	char * cstr_file = new char [db_name.length()+1];
	std::strcpy (cstr_file, db_name.c_str()); // Copy string to cstr

	int rc;

	rc = sqlite3_open(cstr_file, &db);
	int status = 0;

	if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		status = 0;
	} else {
		fprintf(stderr, "Opened database successfully\n");
		status = 1;
	}

	delete[] cstr_file; // free memory

	return status;
}

int SQLDB::sql_query(std::string query)
{

	// Empty vectors
	sql_columns.clear();
	sql_vals.clear();

	// SQL Query
	char * sql = new char [query.length()+1];
	std::strcpy (sql, query.c_str()); // Copy string to cstr

	/* Execute SQL statement */
	char *zErrMsg = 0;
	int rc = sqlite3_exec(db, sql, callback,0, &zErrMsg);

	int status = 0;
	   
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		status = -1;
	} else {
		fprintf(stdout, "Operation done successfully\n");
		status = sql_columns.size(); // Return the size of the result (number of records returned)
	}


	delete[] sql;

	return status;

}

void SQLDB::show_results()
{
	std::cout << "Showing results: " << sql_columns.size() << std::endl;
	for( int i=0; i<sql_columns.size(); i++ )
	{
		std::cout << sql_columns[i] << ": " << sql_vals[i] <<  std::endl;
	}
	
}