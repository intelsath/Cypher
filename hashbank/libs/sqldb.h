#ifndef _SQLDB_H
#define _SQLDB_H

extern "C"
{
	#include <stdio.h>
	#include <sqlite3.h> 
}

#include <vector>
#include <string>

class SQLDB{
	public:
		SQLDB(std::string);
		~SQLDB();

		// SQL query methods
		int open_db();
		int sql_query(std::string);

		// SQL results
		static int callback(void *, int, char **, char **);

		// Show the results within the vectors
		void show_results();
		// Vector holding latest results
		static std::vector<std::string> sql_columns;  // Columns
		static std::vector<std::string> sql_vals;     // Resultss
		
	private:
		sqlite3 *db;
		std::string db_name;

};


#endif