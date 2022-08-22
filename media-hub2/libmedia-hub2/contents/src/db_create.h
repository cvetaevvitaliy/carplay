/*
 * =====================================================================================
 *
 *       Filename:  db_create.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/15/2014 04:38:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  donglj (), dong.lj@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <sqlite3.h>
#define MH_FIELD_NAME_MAX 20
#define MH_FIELD_TYPE_MAX 10
#define MH_FIELD_CONSTRAINT_MAX 100

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

typedef struct _mh_db_field 
{
	char  field_name[MH_FIELD_NAME_MAX];
	char  field_type[MH_FIELD_TYPE_MAX];
	char  constraint[MH_FIELD_CONSTRAINT_MAX];
} mh_db_field_t;				/* ----------  end of struct mh_db_field  ---------- */

typedef enum _db_table_type_t
{
	DB_TBL_TABLE,
	DB_TBL_VIEW,
	DB_TBL_INDEX,
	DB_TBL_MAX
} db_table_type_t;				/* ----------  end of enum db_table_type_t  ---------- */

typedef enum _db_table_exist_t
{
	TABLE_EXIST,
	TABLE_NOT_EXIST,
	TABLE_MAX
} db_table_exist_t;				/* ----------  end of enum db_table_exist_t  ---------- */

typedef struct _db_sets_t 
{
	db_table_type_t type;
	char * tablename;
	const void * def;
	int count;
	int size;
} db_sets_t;				/* ----------  end of struct db_sets_t  ---------- */
extern sqlite3 * db_create_database();
extern void db_create_table( sqlite3 *db );
