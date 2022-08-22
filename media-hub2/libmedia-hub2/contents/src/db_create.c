/*
 * =====================================================================================
 *
 *       Filename:  db_create.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/26/2014 07:11:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  donglj (), dong.lj@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include "db_create.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <mh_contents.h>

static void db_table_exist(sqlite3 * db, db_table_type_t type, char * table_name,db_table_exist_t * exist);

static const mh_db_field_t table_devices[] = {
	{"device_id", 	 "INTEGER",	"not null primary key "},
	{"serial_number","TEXT",	"not null "},
	{"private",		 "BLOB", 	""}
};

static const mh_db_field_t table_node[] = {
	{"node_id",		"INTEGER",	"primary key"},
	{"device_id",	"INTEGER",	"not null references devices(device_id) "},
	{"name",		"TEXT",		""},
	{"size",		"INTEGER",	""},
	{"type",		"INTEGER",	"not null "},
	{"valid",		"INTEGER",	"not null "},
	{"tagId","INTEGER",  ""}
};

static const mh_db_field_t table_closure[] = {
	{"closure_device_id",	"INTEGER",	""},//del db by deviceId
	{"descendant",	"INTEGER",	"not null references node(node_id) "},
	{"ancestor",	"INTEGER",	"not null references node(node_id) "},
	{"parent",		"INTEGER",	"not null references node(node_id) ,primary key(descendant,ancestor)"}
};


static const mh_db_field_t table_music[] = {
	{"node_id",		"INTEGER",	"primary key references node(node_id) "},
	{"title",		"TEXT",		""},
	{"rating",		"INTEGER",	""},
	{"duration",	"INTEGER",	""},
	{"album_title",	"TEXT",		""},
	{"track",		"INTEGER",	""},
	{"track_count",	"INTEGER",	""},
	{"disc",		"INTEGER",  ""},
	{"disc_count",	"INTEGER",	""},
	{"artist",		"TEXT",		""},
	{"album_artist","TEXT",		""},
	{"genre",		"TEXT",		""},
	{"composer",	"TEXT",		""},
	{"year",		"INTEGER",	""},
	{"media_type",	"INTEGER",	""},
	{"compliation","INTEGER",  ""},
	{"music_device_id", "INTEGER",	""},//del db by deviceId
	{"last_chgtime", "INTEGER",	""},//compare by lastchangtime
};
static const mh_db_field_t table_movie[] = {
	{"node_id",		"INTEGER",	"primary key references node(node_id)"},
	{"title",		"TEXT",		""},
	{"year",		"INTEGER",	""},
	{"director",	"TEXT",		""},
	{"duration",	"INTEGER",	""},
	{"genre",		"TEXT",		""},
	{"fps",			"INTEGER",	""},
	{"xres",		"INTEGER",	""},
	{"yres",		"INTEGER",	""},
	{"language",	"TEXT", 	""},
	{"movie_device_id", "INTEGER",	""},//del db by deviceId
};
static const mh_db_field_t table_picture[] = {
	{"node_id",		"INTEGER",	"primary key references node(node_id) "},
	{"format",		"TEXT",		""},
	{"width",		"INTEGER",	""},
	{"height",		"INTEGER",	""},
	{"bpp", 		"INTEGER",	""},
	{"pic_device_id",	"INTEGER",	""},//del db by deviceId
};
static const mh_db_field_t table_playlist[] = {
	{"playlist_id",	"INTEGER",	"primary key "},
	{"name",		"TEXT",		""},
	{"device_id",	"INTEGER",	"references devices(device_id) "},
	{"tag_id",		"INTEGER",	""},
	{"data", 		"BLOB",		""}
};
static const db_sets_t table_sets[]	=	{
	{DB_TBL_TABLE, "devices", table_devices, ARRAY_SIZE(table_devices), sizeof(table_devices) },
	{DB_TBL_TABLE, "node", table_node, ARRAY_SIZE(table_node), sizeof(table_node) },
	{DB_TBL_TABLE, "closure", table_closure, ARRAY_SIZE(table_closure), sizeof(table_closure) },
	{DB_TBL_TABLE, "music", table_music, ARRAY_SIZE(table_music), sizeof(table_music) },
	{DB_TBL_TABLE, "movie", table_movie, ARRAY_SIZE(table_movie), sizeof(table_movie) },
	{DB_TBL_TABLE, "picture", table_picture, ARRAY_SIZE(table_picture), sizeof(table_picture) },
	{DB_TBL_TABLE, "playlist", table_playlist, ARRAY_SIZE(table_playlist), sizeof(table_playlist) }
};
static const char index_node_tagId[]="create index if not exists index_node_tagId on node( tagId);";

#define MH_DB_VERSION "0001" //current db version

void checkDbVersion()
{
	char * _mhconf_path = NULL;
	char * _db_path = NULL;
	FILE * _file = NULL;
	GKeyFile * config;
	gchar *str;
	char * _path	=	getenv("MH_DB_PATH");
	_path	=	_path ? _path : "/var";

	_mhconf_path	=	g_strdup_printf("%s/mediahub.ini", _path);
	_db_path	=	g_strdup_printf("%s/mediahub.db", _path);

	config = g_key_file_new();
	if (!g_key_file_load_from_file(config, _mhconf_path, 0, NULL))
	{
		g_message("mediahub.ini not exist");
		//remove mediahub.db
		if( remove((const char *) _db_path) != 0)
		{
			g_message("remove %s error", _db_path);
		}
			//create mediahub.ini
		_file = fopen(_mhconf_path, "wt+");
		if (_file != NULL)
		{
			fprintf(_file, "[MediaHub_DB]\n");
			fprintf(_file, "VERSION=%s\n", MH_DB_VERSION);
			fclose(_file);
		}
	}
	else
	{
		str = g_key_file_get_string(config,"MediaHub_DB","VERSION",NULL);
		g_message("Version:%s", str);
		if(g_strcmp0( str, MH_DB_VERSION)	==	0)
		{
			g_message("db is OK, do nothing");
		}
		else
		{
			g_message("db is NG, need remove");
			//remove mediahub.db
			if( remove((const char *) _db_path) != 0)
			{
				g_message("remove %s error", _db_path);
			}

			//change version
			_file = fopen(_mhconf_path, "w");
			if (_file != NULL)
			{
				fprintf(_file, "[MediaHub_DB]\n");
				fprintf(_file, "VERSION=%s\n", MH_DB_VERSION);
				fclose(_file);
			}
		}
		g_free( str);
	}
	
	g_free( _db_path );
	g_free( _mhconf_path );
	g_key_file_free(config);
}

sqlite3 * db_create_database()
{
	int _sql_res;
	sqlite3 * _res	=	NULL;
	sqlite3 * _db;
	char * _path	=	getenv("MH_DB_PATH");
	char * _db_path;

	_path	=	_path ? _path : "/var";
	_db_path	=	g_strdup_printf("%s/mediahub.db", _path);
	checkDbVersion();

	_sql_res	=	sqlite3_open( _db_path, &_db );
	if( _sql_res	!=	SQLITE_OK )
	{
		g_warning("can't open database:%s\n",sqlite3_errmsg( _db ));
		sqlite3_close(_db);
	}
	else
	{
//		sqlite3_exec( _db, "PRAGMA foreign_keys = TRUE;", NULL, NULL, NULL );
		sqlite3_exec( _db, "PRAGMA page_size=8192;", NULL, NULL, NULL );
		_res	=	_db;
	}

	g_free( _db_path );

	return _res;
}
void db_create_table( sqlite3 * db)
{

	char _sql[SQL_CMD_MAX]	= "";
	char _sub[SQL_CMD_MAX]	= "";
	const mh_db_field_t  * _db_field;
	int _table_num	=	ARRAY_SIZE( table_sets );
	int _column_num 	=	0;
	int _i=0;
	int _j=0;
	sqlite3_stmt * _stmt;
	db_table_exist_t  _exist;
	for(_i =0 ; _i < _table_num ; _i++)
	{
		if( table_sets[ _i ].type == DB_TBL_TABLE )
		{
			db_table_exist( db, DB_TBL_TABLE, table_sets[_i].tablename, &_exist);
			if( _exist	==	TABLE_NOT_EXIST )
			{
				snprintf(_sql, SQL_CMD_MAX, "create table %s (", table_sets[_i].tablename );
				_column_num	=  table_sets[_i].count ;
				for(_j =0; _j < _column_num - 1; _j++)
				{
					_db_field	=	table_sets[_i].def;
					snprintf( _sub, SQL_CMD_MAX, "%s %s %s,",
							_db_field[_j].field_name,
							_db_field[_j].field_type,
							_db_field[_j].constraint);
					g_strlcat(_sql, _sub, SQL_CMD_MAX);
				}
				snprintf( _sub, SQL_CMD_MAX, "%s %s %s)",
							_db_field[_j].field_name,
							_db_field[_j].field_type,
							_db_field[_j].constraint);
				g_strlcat( _sql, _sub, SQL_CMD_MAX );
				if(sqlite3_prepare_v2( db, _sql, -1, &_stmt, NULL) != SQLITE_OK)
				{
					g_warning("sqlite3_prepare_v2 is err\n");
				}
				if(sqlite3_step( _stmt ) != SQLITE_DONE)
				{
					g_warning("sqlite3_step is err\n");
				}
				sqlite3_finalize( _stmt );
			}
		}
	
	}
	if(sqlite3_prepare_v2( db, index_node_tagId, -1, &_stmt, NULL) != SQLITE_OK)
	{
		g_message("%s\n",index_node_tagId);
		g_warning("sqlite3_prepare_v2 is err\n");
	}
	if(sqlite3_step( _stmt ) != SQLITE_DONE)
	{
		g_warning("sqlite3_step is err\n");
	}
	sqlite3_finalize( _stmt );

	
}
void db_table_exist(sqlite3 * db, db_table_type_t type, char * table_name,db_table_exist_t * exist)
{
	char _sql[SQL_CMD_MAX]="";
	sqlite3_stmt * _stmt;
	int _count;
	if( db	==	NULL )
	{
		g_warning("db value NULL\n");
		return;
	}
	if(type	==	DB_TBL_TABLE)
	{
		snprintf(_sql, SQL_CMD_MAX,
			"select count(*) from sqlite_master "
			"where type='table' and name='%s' ",table_name );
	
		if(sqlite3_prepare_v2( db, _sql, -1, &_stmt, NULL) != SQLITE_OK)
		{
			g_warning("sqlite3_prepare_v2 is err\n");
		}
		while(sqlite3_step( _stmt ) == SQLITE_ROW )
		{
			_count	=	sqlite3_column_int( _stmt, 0 );
		}
		sqlite3_finalize( _stmt );
		if( _count == 0)
		{
			*exist	=	TABLE_NOT_EXIST;
	
		}
		else
		{
			*exist	=	TABLE_EXIST;
	
		}

	}


}
