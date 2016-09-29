/*
**
** libzbxredis - A Redis monitoring module for Zabbix
** Copyright (C) 2016 - James Cook <james.cook000@gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License,or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

/*
**
** See:
**     Hiredis: https://github.com/redis/hiredis
**
*/

// Include Libraries
#include <hiredis.h>
#include <time.h>
#include <libzbxredis.h>
#include <regex.h>

/*********************************************************************************
 *                                                                               *
 * Custom Key            : redis.session.status[server,port,timeout,password]    *
 *                                                                               *
 * Function              : Gets the session status to a redis server             *
 * Parameters [server]   : Redis server address to connect                       *
 * Parameters [port]     : Redis server port to connect                          *
 * Parameters [timeout]  : Timeout in seconds                                    *
 * Parameters [password] : Redis password to connect using (blank)               *
 * Returns               : 0 (failure),1 (success)                               *
 *                                                                               *
 *********************************************************************************/
int redis_session_status(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_session_status";
	const char     *__key_name      = "redis.session.status[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {

		// Set return
		zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);

	} else {

		// Set return
		zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);

		// Free the context
		redisFree(redisC);

	}

out:

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************
 *                                                                                 *
 * Custom Key            : redis.session.duration[server,port,timeout,password]    *
 *                                                                                 *
 * Function              : Gets the session duration to a redis server             *
 * Parameters [server]   : Redis server address to connect                         *
 * Parameters [port]     : Redis server port to connect                            *
 * Parameters [timeout]  : Timeout in seconds                                      *
 * Parameters [password] : Redis password to connect using (blank)                 *
 * Returns               : 0 (success),1 (failure)                                 *
 *                                                                                 *
 ***********************************************************************************/
int redis_session_duration(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_session_duration";
	const char     *__key_name      = "redis.session.duration[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout, clock_start, clock_finish;
	float           clock_duration;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Start the timer (microseconds)
	gettimeofday(&clock_start,NULL);

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {goto out;}

	// Finish the timer (microseconds)
	gettimeofday(&clock_finish,NULL);

	// Calculate the duration (milliseconds)
	clock_duration = (clock_finish.tv_usec - clock_start.tv_usec) * 0.001;

	// If the duration is less than zero then make it zero
	if (clock_duration < 0) {clock_duration=0;}

	// Set return
	zbx_ret_float(result, &ret, LOG_LEVEL_DEBUG, zbx_key, clock_duration, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*********************************************************************************************
 *                                                                                           *
 * Custom Key             : redis.command.supported[server,port,timeout,password,command]    *
 *                                                                                           *
 * Function               : Gets the redis lastsave time in seconds                          *
 * Parameters [server]    : Redis server address to connect                                  *
 * Parameters [port]      : Redis server port to connect                                     *
 * Parameters [timeout]   : Timeout in seconds                                               *
 * Parameters [password]  : Redis password to connect using (blank)                          *
 * Parameters [command]   : Redis command to run and time                                    *
 * Returns                : 0 (success),1 (failure)                                          *
 *                                                                                           *
 *********************************************************************************************/
int redis_command_supported(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_command_supported";
	const char     *__key_name      = "redis.command.supported[server,port,timeout,password,command]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 5;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_command;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_command  = get_rparam(request,4);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Redis command", param_command, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                  {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "COMMAND INFO", param_command, REDIS_REPLY_ARRAY)) {goto out;}

	// If the command is supported
	if (redisR->element[0]->elements) {

		// Set return
		zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);

		goto out;

	}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***************************************************************************************************
 *                                                                                                 *
 * Custom Key             : redis.command.duration[server,port,timeout,password,command,params]    *
 *                                                                                                 *
 * Function               : Gets the redis lastsave time in seconds                                *
 * Parameters [server]    : Redis server address to connect                                        *
 * Parameters [port]      : Redis server port to connect                                           *
 * Parameters [timeout]   : Timeout in seconds                                                     *
 * Parameters [password]  : Redis password to connect using (blank)                                *
 * Parameters [command]   : Redis command                                                          *
 * Parameters [params]    : Redis parameters                                                       *
 * Returns                : 0 (success),1 (failure)                                                *
 *                                                                                                 *
 ***************************************************************************************************/
int redis_command_duration(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_command_duration";
	const char     *__key_name      = "redis.command.duration[server,port,timeout,password,command,params]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_command, *param_params;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout, clock_start, clock_finish;
	float           clock_duration;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_command  = get_rparam(request,4);
	param_params   = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Redis command", param_command, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                  {return ret;}
	if (validate_param(result, zbx_key, "Redis params", param_params, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                     {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Start the timer (microseconds)
	gettimeofday(&clock_start,NULL);

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, param_command, param_params, 9999)) {goto out;}

	// Finish the timer (microseconds)
	gettimeofday(&clock_finish,NULL);

	// Calculate the duration (milliseconds)
	clock_duration = (clock_finish.tv_usec - clock_start.tv_usec) * 0.001;

	// If the duration is less than zero then make it zero
	if (clock_duration < 0) {clock_duration=0;}

	// Set return
	zbx_ret_float(result, &ret, LOG_LEVEL_DEBUG, zbx_key, clock_duration, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/****************************************************************************************************
 *                                                                                                  *
 * Custom Key            : redis.info[server,port,timeout,password,datatype,section,key,default]    *
 *                                                                                                  *
 * Function              : Gets a redis cmd info field                                              *
 * Parameters [server]   : Redis server address to connect                                          *
 * Parameters [port]     : Redis server port to connect                                             *
 * Parameters [timeout]  : Timeout in seconds                                                       *
 * Parameters [password] : Redis password to connect using (blank)                                  *
 * Parameters [datatype] : Data type to return (integer,float,string,text)                          *
 * Parameters [section]  : Section to return (Server, Stats, Cpu, Memory etc...)                    *
 * Parameters [key]      : Key to return (redis_version,redis_build_id,redis_mode etc...)           *
 * Parameters [default]  : Value default to return if no value is detected                          *
 * Returns               : 0 (success),1 (failure)                                                  *
 *                                                                                                  *
 ****************************************************************************************************/
int redis_info(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_info";
	const char     *__key_name      = "redis.info[server,port,timeout,password,datatype,section,key,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 8;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_datatype, *param_section, *param_key, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;
	regmatch_t      regexGroups[3];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_datatype = get_rparam(request,4);
	param_section  = get_rparam(request,5);
	param_key      = get_rparam(request,6);
	param_default  = get_rparam(request,7);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Datatype", param_datatype, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Section", param_section, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                        {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "INFO", param_section, REDIS_REPLY_STRING)) {goto out;}

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// Declare Variables
		char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="", redis_value[MAX_LENGTH_KEY]="";

		// If the line is matched
		if (regexec(&regexCompiled_INFO,line,3,regexGroups,0) == 0) {

			// Copy the fields
			zbx_strlcpy(redis_field,line + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);
			zbx_strlcpy(redis_data,line + regexGroups[2].rm_so,regexGroups[2].rm_eo - regexGroups[2].rm_so + 1);

			// If the line contains the key field
			if (redis_get_value(redis_field, redis_data, param_key, redis_value) == 0) {

				// Set return
				zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redis_value, param_datatype, redisR);

				goto out;

			}

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// For some values they are or are not present depending on the status of redis so this is where the default value comes into it
	// This is used to prevent Zabbix from reporting the item as "Unsupported" where it makes sense ie master_link_down_since_seconds could be defaulted to 0

	// If the info is undetected and can be set to a default value
	if (strlen(param_default) > 0 && param_default != NULL) {

		// Set return
		zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, param_default, param_datatype, redisR);

		goto out;

	}

	// Set return
	zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis information does not exist", redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*************************************************************************************
 *                                                                                   *
 * Custom Key            : redis.database.discovery[server,port,timeout,password]    *
 *                                                                                   *
 * Function              : Discovers redis databases                                 *
 * Parameters [server]   : Redis server address to connect                           *
 * Parameters [port]     : Redis server port to connect                              *
 * Parameters [timeout]  : Timeout in seconds                                        *
 * Parameters [password] : Redis password to connect using (blank)                   *
 * Returns               : 0 (success),1 (failure)                                   *
 *                                                                                   *
 *************************************************************************************/
int redis_database_discovery(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_database_discovery";
	const char     *__key_name      = "redis.database.discovery[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	struct          zbx_json j;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	int             discovered_instances = 0;
	char           *line;
	regmatch_t      regexGroups[3];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "INFO KEYSPACE", NULL, REDIS_REPLY_STRING)) {goto out;}

	// Initialise JSON for discovery
	zbx_json_init(&j,ZBX_JSON_STAT_BUF_LEN);

	// Create JSON array of discovered instances
	zbx_json_addarray(&j,ZBX_PROTO_TAG_DATA);

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// If the line is matched
		if (regexec(&regexCompiled_INFO_DATABASE,line,3,regexGroups,0) == 0) {

			// Declare variables
			char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="", redis_value[MAX_LENGTH_KEY]="";
			char redis_database[MAX_LENGTH_KEY]="";

			// Copy the fields
			zbx_strlcpy(redis_field,line + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);
			zbx_strlcpy(redis_data,line + regexGroups[2].rm_so,regexGroups[2].rm_eo - regexGroups[2].rm_so + 1);

			// Open instance in JSON
			zbx_json_addobject(&j, NULL);

			zbx_json_addstring(&j, "{#DATABASE}", redis_field, ZBX_JSON_TYPE_STRING);

			// Close instance in JSON 
			zbx_json_close(&j);

			// Increment discovered instances
			discovered_instances++;

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// Finalise JSON for discovery
	zbx_json_close(&j);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Key (%s) discovered instances (%lld)",MODULE,zbx_key,discovered_instances);

	// Set result
	SET_STR_RESULT(result, strdup(j.buffer));

	// Set return
	ret = SYSINFO_RET_OK;

	// Free the json
	zbx_json_free(&j);

	// Free the reply
	freeReplyObject(redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/******************************************************************************************************
 *                                                                                                    *
 * Custom Key            : redis.database.info[server,port,timeout,password,datatype,database,key]    *
 *                                                                                                    *
 * Function              : Gets the redis database info field                                         *
 * Parameters [server]   : Redis server address to connect                                            *
 * Parameters [port]     : Redis server port to connect                                               *
 * Parameters [timeout]  : Timeout in seconds                                                         *
 * Parameters [password] : Redis password to connect using (blank)                                    *
 * Parameters [datatype] : Data type to return (integer,float,string,text)                            *
 * Parameters [database] : Database to select the key for (0,1,2 etc...)                              *
 * Parameters [key]      : Key to return (ip,port,state,offset,lag etc...)                            *
 * Parameters [default]  : Value default to return if no value is detected                            *
 * Returns               : 0 (success),1 (failure)                                                    *
 *                                                                                                    *
 ******************************************************************************************************/
int redis_database_info(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_database_info";
	const char     *__key_name      = "redis.database.info[server,port,timeout,password,datatype,database,key,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 8;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_datatype, *param_database, *param_key, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;
	regmatch_t      regexGroups[3];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_datatype = get_rparam(request,4);
	param_database = get_rparam(request,5);
	param_key      = get_rparam(request,6);
	param_default  = get_rparam(request,7);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Datatype", param_datatype, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "INFO KEYSPACE", NULL, REDIS_REPLY_STRING)) {goto out;}

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// If the line is matched
		if (regexec(&regexCompiled_INFO_DATABASE,line,3,regexGroups,0) == 0) {

			// Declare variables
			char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="", redis_value[MAX_LENGTH_KEY]="";

			// Copy the fields
			zbx_strlcpy(redis_field,line + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);
			zbx_strlcpy(redis_data,line + regexGroups[2].rm_so,regexGroups[2].rm_eo - regexGroups[2].rm_so + 1);

			// If the database matches the requested database
			if (strcmp(param_database,redis_field) == 0) {

				// If the redis value can not be found
				if (redis_get_value(redis_field, redis_data, param_key, redis_value) == 1 ) {

					// For some values they are or are not present depending on the status of redis so this is where the default value comes into it
					// This is used to prevent Zabbix from reporting the item as "Unsupported" where it makes sense ie master_link_down_since_seconds could be defaulted to 0

					// If the info is undetected and can be set to a default value
					if (strlen(param_default) > 0 && param_default != NULL) {

						// Set return
						zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, param_default, param_datatype, redisR);

						goto out;

					}

					// Set return
					zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis database information does not exist", redisR);

					goto out;

				}

				// Set return
				zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redis_value, param_datatype, redisR);

				goto out;

			}

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// Set return
	zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis database does not exist", redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/**********************************************************************************
 *                                                                                *
 * Custom Key            : redis.slave.discovery[server,port,timeout,password]    *
 *                                                                                *
 * Function              : Discovers redis slaves                                 *
 * Parameters [server]   : Redis server address to connect                        *
 * Parameters [port]     : Redis server port to connect                           *
 * Parameters [timeout]  : Timeout in seconds                                     *
 * Parameters [password] : Redis password to connect using (blank)                *
 * Returns               : 0 (success),1 (failure)                                *
 *                                                                                *
 **********************************************************************************/
int redis_slave_discovery(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_slave_discovery";
	const char     *__key_name      = "redis.slave.discovery[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	struct          zbx_json j;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	int             discovered_instances = 0;
	char           *line;
	regmatch_t      regexGroups[3];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "INFO REPLICATION", NULL, REDIS_REPLY_STRING)) {goto out;}

	// Initialise JSON for discovery
	zbx_json_init(&j,ZBX_JSON_STAT_BUF_LEN);

	// Create JSON array of discovered instances
	zbx_json_addarray(&j,ZBX_PROTO_TAG_DATA);

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// If the line is matched
		if (regexec(&regexCompiled_INFO_SLAVE,line,3,regexGroups,0) == 0) {

			// Declare variables
			char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="";
			char redis_slave[MAX_LENGTH_KEY]="", redis_slave_ip[MAX_LENGTH_KEY]="", redis_slave_port[MAX_LENGTH_KEY]="";

			// Copy the fields
			zbx_strlcpy(redis_field,line + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);
			zbx_strlcpy(redis_data,line + regexGroups[2].rm_so,regexGroups[2].rm_eo - regexGroups[2].rm_so + 1);

			// Get the values
			redis_get_value(redis_field, redis_data, "ip", redis_slave_ip);
			redis_get_value(redis_field, redis_data, "port", redis_slave_port);

			// Form the slave
			strcat(redis_slave,redis_slave_ip);
			strcat(redis_slave,":");
			strcat(redis_slave,redis_slave_port);

			// Open instance in JSON
			zbx_json_addobject(&j, NULL);

			zbx_json_addstring(&j, "{#SLAVE}", redis_slave, ZBX_JSON_TYPE_STRING);

			// Close instance in JSON 
			zbx_json_close(&j);

			// Increment discovered instances
			discovered_instances++;

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// Finalise JSON for discovery
	zbx_json_close(&j);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Key (%s) discovered instances (%lld)",MODULE,zbx_key,discovered_instances);

	// Set result
	SET_STR_RESULT(result, strdup(j.buffer));

	// Set return
	ret = SYSINFO_RET_OK;

	// Free the json
	zbx_json_free(&j);

	// Free the reply
	freeReplyObject(redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/************************************************************************************************************************
 *                                                                                                                      *
 * Custom Key            : redis.slave.info[server,port,timeout,password,datatype,slave,key]                            *
 *                                                                                                                      *
 * Function              : Gets the redis slave info field                                                              *
 * Parameters [server]   : Redis server address to connect                                                              *
 * Parameters [port]     : Redis server port to connect                                                                 *
 * Parameters [timeout]  : Timeout in seconds                                                                           *
 * Parameters [password] : Redis password to connect using (blank)                                                      *
 * Parameters [datatype] : Data type to return (integer,float,string,text)                                              *
 * Parameters [slave]    : Slave to select the key for (192.168.1.1:6789, 192.168.1.2:6789, 192.168.1.3:6789 etc...)    *
 * Parameters [key]      : Key to return (ip,port,state,offset,lag etc...)                                              *
 * Parameters [default]  : Value default to return if no value is detected                                              *
 * Returns               : 0 (success),1 (failure)                                                                      *
 *                                                                                                                      *
 ************************************************************************************************************************/
int redis_slave_info(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_slave_info";
	const char     *__key_name      = "redis.slave.info[server,port,timeout,password,datatype,slave,key,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 8;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_datatype, *param_slave, *param_key, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;
	regmatch_t      regexGroups[3];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_datatype = get_rparam(request,4);
	param_slave    = get_rparam(request,5);
	param_key      = get_rparam(request,6);
	param_default  = get_rparam(request,7);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Datatype", param_datatype, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Slave", param_slave, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                            {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "INFO REPLICATION", NULL, REDIS_REPLY_STRING)) {goto out;}

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// Declare  Variables
		char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="", redis_value[MAX_LENGTH_KEY]="";
		char redis_slave[MAX_LENGTH_KEY]="", redis_slave_ip[MAX_LENGTH_KEY]="", redis_slave_port[MAX_LENGTH_KEY]="";

		// If the line is matched
		if (regexec(&regexCompiled_INFO_SLAVE,line,3,regexGroups,0) == 0) {

			// Copy the fields
			zbx_strlcpy(redis_field,line + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);
			zbx_strlcpy(redis_data,line + regexGroups[2].rm_so,regexGroups[2].rm_eo - regexGroups[2].rm_so + 1);

			// Get the values
			redis_get_value(redis_field, redis_data, "ip", redis_slave_ip);
			redis_get_value(redis_field, redis_data, "port", redis_slave_port);

			// Form the slave
			strcat(redis_slave,redis_slave_ip);
			strcat(redis_slave,":");
			strcat(redis_slave,redis_slave_port);

			// If the slave matches the requested slave
			if (strcmp(param_slave,redis_slave) == 0) {

				// If the redis value can not be found
				if (redis_get_value(redis_field, redis_data, param_key, redis_value) == 1 ) {

					// For some values they are or are not present depending on the status of redis so this is where the default value comes into it
					// This is used to prevent Zabbix from reporting the item as "Unsupported" where it makes sense ie master_link_down_since_seconds could be defaulted to 0

					// If the info is undetected and can be set to a default value
					if (strlen(param_default) > 0 && param_default != NULL) {

						// Set return
						zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, param_default, param_datatype, redisR);

						goto out;

					}

					// Set return
					zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis slave information does not exist", redisR);

					goto out;

				}

				// Set return
				zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redis_value, param_datatype, redisR);

				goto out;

			}

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// Set return
	zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis slave does not exist", redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************
 *                                                                     *
 * Custom Key            : redis.ping[server,port,timeout,password]    *
 *                                                                     *
 * Function              : Gets the redis ping status                  *
 * Parameters [server]   : Redis server address to connect             *
 * Parameters [port]     : Redis server port to connect                *
 * Parameters [timeout]  : Timeout in seconds                          *
 * Parameters [password] : Redis password to connect using (blank)     *
 * Returns               : 0 (success),1 (failure)                     *
 *                                                                     *
 ***********************************************************************/
int redis_ping(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_ping";
	const char     *__key_name      = "redis.ping[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "PING", NULL, REDIS_REPLY_STATUS)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************
 *                                                                     *
 * Custom Key            : redis.time[server,port,timeout,password]    *
 *                                                                     *
 * Function              : Gets the redis time in seconds              *
 * Parameters [server]   : Redis server address to connect             *
 * Parameters [port]     : Redis server port to connect                *
 * Parameters [timeout]  : Timeout in seconds                          *
 * Parameters [password] : Redis password to connect using (blank)     *
 * Returns               : 0 (success),1 (failure)                     *
 *                                                                     *
 ***********************************************************************/
int redis_time(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_time";
	const char     *__key_name      = "redis.time[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "TIME", NULL, REDIS_REPLY_ARRAY)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->element[0]->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***************************************************************************
 *                                                                         *
 * Custom Key            : redis.lastsave[server,port,timeout,password]    *
 *                                                                         *
 * Function              : Gets the redis lastsave time in seconds         *
 * Parameters [server]   : Redis server address to connect                 *
 * Parameters [port]     : Redis server port to connect                    *
 * Parameters [timeout]  : Timeout in seconds                              *
 * Parameters [password] : Redis password to connect using (blank)         *
 * Returns               : 0 (success),1 (failure)                         *
 *                                                                         *
 ***************************************************************************/
int redis_lastsave(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_lastsave";
	const char     *__key_name      = "redis.lastsave[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "LASTSAVE", NULL, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************
 *                                                                     *
 * Custom Key            : redis.role[server,port,timeout,password]    *
 *                                                                     *
 * Function              : Gets the redis role                         *
 * Parameters [server]   : Redis server address to connect             *
 * Parameters [port]     : Redis server port to connect                *
 * Parameters [timeout]  : Timeout in seconds                          *
 * Parameters [password] : Redis password to connect using (blank)     *
 * Returns               : 0 (success),1 (failure)                     *
 *                                                                     *
 ***********************************************************************/
int redis_role(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_role";
	const char     *__key_name      = "redis.role[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "ROLE", NULL, REDIS_REPLY_ARRAY)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->element[0]->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*******************************************************************************************
 *                                                                                         *
 * Custom Key            : redis.keyspace.hit.ratio[server,port,timeout,password]          *
 *                                                                                         *
 * Function              : Gets the redis keyspace hit ratio                               *
 * Parameters [server]   : Redis server address to connect                                 *
 * Parameters [port]     : Redis server port to connect                                    *
 * Parameters [timeout]  : Timeout in seconds                                              *
 * Parameters [password] : Redis password to connect using (blank)                         *
 * Returns               : 0 (success),1 (failure)                                         *
 *                                                                                         *
 *******************************************************************************************/
int redis_keyspace_hit_ratio(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char        *__function_name = "redis_keyspace_hit_ratio";
	const char        *__key_name      = "redis.keyspace.hit.ratio[server,port,timeout,password]";
	int                ret = SYSINFO_RET_FAIL;
	char               zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int                param_count = 4;
	char              *param_server, *param_port, *param_timeout, *param_password;
	redisContext      *redisC;
	redisReply        *redisR;
	struct timeval     timeout;
	char              *line;
	unsigned long long keyspace_hits = 0, keyspace_misses = 0, keyspace_total = 0;
	float              keyspace_hitrate = 0;
	regmatch_t         regexGroups[3];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "INFO", "stats", REDIS_REPLY_STRING)) {goto out;}

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// Declare Variables
		char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="", redis_value[MAX_LENGTH_KEY]="";

		// If the line is matched
		if (regexec(&regexCompiled_INFO,line,3,regexGroups,0) == 0) {

			// Copy the fields
			zbx_strlcpy(redis_field,line + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);
			zbx_strlcpy(redis_data,line + regexGroups[2].rm_so,regexGroups[2].rm_eo - regexGroups[2].rm_so + 1);

			// If the line contains the keyspace_hits
			if (redis_get_value(redis_field, redis_data, "keyspace_hits", redis_value) == 0) {keyspace_hits = atoll(redis_value);}

			// If the line contains the keyspace_misses
			if (redis_get_value(redis_field, redis_data, "keyspace_misses", redis_value) == 0) {keyspace_misses = atoll(redis_value);}

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}


	// Set the keyspace total
	keyspace_total = keyspace_hits + keyspace_misses;

	// Set the keyspace hitrate
	keyspace_hitrate = (float)keyspace_hits / (float)keyspace_total;

	// If there are zero keyspace hits
	if (keyspace_hits == 0) {keyspace_hitrate = 1;}

	// Set return
	zbx_ret_float(result, &ret, LOG_LEVEL_DEBUG, zbx_key, keyspace_hitrate, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/********************************************************************************
 *                                                                              *
 * Custom Key            : redis.slowlog.length[server,port,timeout,password]   *
 *                                                                              *
 * Function              : Gets the redis slowlog length                        *
 * Parameters [server]   : Redis server address to connect                      *
 * Parameters [port]     : Redis server port to connect                         *
 * Parameters [timeout]  : Timeout in seconds                                   *
 * Parameters [password] : Redis password to connect using (blank)              *
 * Returns               : 0 (success),1 (failure)                              *
 *                                                                              *
 ********************************************************************************/
int redis_slowlog_length(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_slowlog_length";
	const char     *__key_name      = "redis.slowlog.length[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "SLOWLOG LEN", NULL, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/**************************************************************************************
 *                                                                                    *
 * Custom Key            : redis.config[server,port,timeout,password,datatype,key]    *
 *                                                                                    *
 * Function              : Gets the redis config field                                *
 * Parameters [server]   : Redis server address to connect                            *
 * Parameters [port]     : Redis server port to connect                               *
 * Parameters [timeout]  : Timeout in seconds                                         *
 * Parameters [password] : Redis password to connect using (blank)                    *
 * Parameters [datatype] : Data type to return (integer,float,string,text)            *
 * Parameters [key]      : Key to return (dbfilename,logfile,pidfile etc...)          *
 * Parameters [default]  : Value default to return if no value is detected            *
 * Returns               : 0 (success),1 (failure)                                    *
 *                                                                                    *
 **************************************************************************************/
int redis_config(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_config";
	const char     *__key_name      = "redis.config[server,port,timeout,password,datatype,key,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 7;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_datatype, *param_key, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	char            redisCmd[MAX_LENGTH_KEY];
	struct timeval  timeout;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_datatype = get_rparam(request,4);
	param_key      = get_rparam(request,5);
	param_default  = get_rparam(request,6);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;;}
	if (validate_param(result, zbx_key, "Datatype", param_datatype, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "CONFIG GET", param_key, REDIS_REPLY_ARRAY)) {goto out;}

	// If the config key does not exist
	if (redisR->elements == 0) {

		// For some values they are or are not present depending on the status of redis so this is where the default value comes into it
		// This is used to prevent Zabbix from reporting the item as "Unsupported" where it makes sense ie master_link_down_since_seconds could be defaulted to 0

		// If the info is undetected and can be set to a default value
		if (strlen(param_default) > 0 && param_default != NULL) {

			// Set return
			zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, param_default, param_datatype, redisR);

			goto out;

		}

		// Set return
		zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis information does not exist", redisR);

		goto out;

	}

	// Set return
	zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->element[1]->str, param_datatype, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************
 *                                                                                 *
 * Custom Key            : redis.client.discovery[server,port,timeout,password]    *
 *                                                                                 *
 * Function              : Discovers redis clients                                 *
 * Parameters [server]   : Redis server address to connect                         *
 * Parameters [port]     : Redis server port to connect                            *
 * Parameters [timeout]  : Timeout in seconds                                      *
 * Parameters [password] : Redis password to connect using (blank)                 *
 * Returns               : 0 (success),1 (failure)                                 *
 *                                                                                 *
 ***********************************************************************************/
int redis_client_discovery(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_client_discovery";
	const char     *__key_name      = "redis.client.discovery[server,port,timeout,password]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 4;
	char           *param_server, *param_port, *param_timeout, *param_password;
	struct          zbx_json j;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	int             discovered_instances = 0;
	char            redis_value[MAX_LENGTH_KEY], redis_client_name[MAX_LENGTH_KEY];
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "CLIENT LIST", NULL, REDIS_REPLY_STRING)) {goto out;}

	// Initialise JSON for discovery
	zbx_json_init(&j,ZBX_JSON_STAT_BUF_LEN);

	// Create JSON array of discovered instances
	zbx_json_addarray(&j,ZBX_PROTO_TAG_DATA);

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// Declare variables
		char redis_value[MAX_LENGTH_KEY]="", redis_client_name[MAX_LENGTH_KEY]="";

		// Try to get the client name in order to skip libzbxredis connections
		redis_get_value("", line, "name", redis_client_name);
		
		// If the line contains addr and is not a libzbxredis-internal client name
		if (redis_get_value("", line, "addr", redis_value) == 0 && strcmp(redis_client_name,MODULE) != 0) {

			// Open instance in JSON
			zbx_json_addobject(&j, NULL);

			zbx_json_addstring(&j, "{#CLIENT}", redis_value, ZBX_JSON_TYPE_STRING);

			// Close instance in JSON 
			zbx_json_close(&j);

			// Increment discovered instances
			discovered_instances++;

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// Finalise JSON for discovery
	zbx_json_close(&j);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Key (%s) discovered instances (%lld)",MODULE,zbx_key,discovered_instances);

	// Set result
	SET_STR_RESULT(result, strdup(j.buffer));

	// Set return
	ret = SYSINFO_RET_OK;

	// Free the json
	zbx_json_free(&j);

	// Free the reply
	freeReplyObject(redisR);


out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*************************************************************************************************************************
 *                                                                                                                       *
 * Custom Key            : redis.client.info[server,port,timeout,password,datatype,client,key]                           *
 *                                                                                                                       *
 * Function              : Gets the redis client field                                                                   *
 * Parameters [server]   : Redis server address to connect                                                               *
 * Parameters [port]     : Redis server port to connect                                                                  *
 * Parameters [timeout]  : Timeout in seconds                                                                            *
 * Parameters [password] : Redis password to connect using (blank)                                                       *
 * Parameters [datatype] : Data type to return (integer,float,string,text)                                               *
 * Parameters [client]   : Client to select the key for (192.168.1.1:6789, 192.168.1.2:6789, 192.168.1.3:6789 etc...)    *
 * Parameters [key]      : Key to return (ip,port,state,offset,lag etc...)                                               *
 * Parameters [default]  : Value default to return if no value is detected                                               *
 * Returns               : 0 (success),1 (failure)                                                                       *
 *                                                                                                                       *
 *************************************************************************************************************************/
int redis_client_info(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_client_info";
	const char     *__key_name      = "redis.client.info[server,port,timeout,password,datatype,client,key,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 8;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_datatype, *param_client, *param_key, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_datatype = get_rparam(request,4);
	param_client   = get_rparam(request,5);
	param_key      = get_rparam(request,6);
	param_default  = get_rparam(request,7);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Datatype", param_datatype, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Client", param_client, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                          {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "CLIENT LIST", NULL, REDIS_REPLY_STRING)) {goto out;}

	// Get first line of output
	line = strtok(redisR->str, "\n\r");

	// Process every line of output
	while (line != NULL) {

		// Declare variables
		char redis_field[MAX_LENGTH_KEY]="", redis_data[MAX_LENGTH_KEY]="", redis_value[MAX_LENGTH_KEY]="";
		char redis_client[MAX_LENGTH_KEY]="";

		// Get the values
		redis_get_value("", line, "addr", redis_client);

		// If the client matches the requested client
		if (strcmp(param_client,redis_client) == 0) {

			// If the redis value can not be found
			if (redis_get_value("", line, param_key, redis_value) == 1 ) {

				// For some values they are or are not present depending on the status of redis so this is where the default value comes into it
				// This is used to prevent Zabbix from reporting the item as "Unsupported" where it makes sense ie master_link_down_since_seconds could be defaulted to 0

				// If the info is undetected and can be set to a default value
				if (strlen(param_default) > 0 && param_default != NULL) {

					// Set return
					zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, param_default, param_datatype, redisR);

					goto out;

				}

				// Set return
				zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis client information does not exist", redisR);

				goto out;

			}

			// Set return
			zbx_ret_string_convert(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redis_value, param_datatype, redisR);

			goto out;

		}

		// Get next line of output
		line = strtok(NULL,"\n\r");

	}

	// Set return
	zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis client does not exist", redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/******************************************************************************************
 *                                                                                        *
 * Custom Key            : redis.key.exists[server,port,timeout,password,database,key]    *
 *                                                                                        *
 * Function              : Check if a redis key exists                                    *
 * Parameters [server]   : Redis server address to connect                                *
 * Parameters [port]     : Redis server port to connect                                   *
 * Parameters [timeout]  : Timeout in seconds                                             *
 * Parameters [password] : Redis password to connect using (blank)                        *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)               *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                     *
 * Returns               : 0 (success),1 (failure)                                        *
 *                                                                                        *
 ******************************************************************************************/
int redis_key_exists(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_exists";
	const char     *__key_name      = "redis.key.exists[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);}

	// If the redis key does exist
	if (! redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);}

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***************************************************************************************
 *                                                                                     *
 * Custom Key            : redis.key.ttl[server,port,timeout,password,database,key]    *
 *                                                                                     *
 * Function              : Get a redis keys ttl in seconds                             *
 * Parameters [server]   : Redis server address to connect                             *
 * Parameters [port]     : Redis server port to connect                                *
 * Parameters [timeout]  : Timeout in seconds                                          *
 * Parameters [password] : Redis password to connect using (blank)                     *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)            *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                  *
 * Returns               : 0 (success),1 (failure)                                     *
 *                                                                                     *
 ***************************************************************************************/
int redis_key_ttl(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_ttl";
	const char     *__key_name      = "redis.key.ttl[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "TTL", param_key, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/****************************************************************************************
 *                                                                                      *
 * Custom Key            : redis.key.pttl[server,port,timeout,password,database,key]    *
 *                                                                                      *
 * Function              : Get a redis keys pttl in milliseconds                        *
 * Parameters [server]   : Redis server address to connect                              *
 * Parameters [port]     : Redis server port to connect                                 *
 * Parameters [timeout]  : Timeout in seconds                                           *
 * Parameters [password] : Redis password to connect using (blank)                      *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)             *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                   *
 * Returns               : 0 (success),1 (failure)                                      *
 *                                                                                      *
 ****************************************************************************************/
int redis_key_pttl(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_pttl";
	const char     *__key_name      = "redis.key.pttl[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "PTTL", param_key, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/****************************************************************************************
 *                                                                                      *
 * Custom Key            : redis.key.type[server,port,timeout,password,database,key]    *
 *                                                                                      *
 * Function              : Gets a redis key type                                        *
 * Parameters [server]   : Redis server address to connect                              *
 * Parameters [port]     : Redis server port to connect                                 *
 * Parameters [timeout]  : Timeout in seconds                                           *
 * Parameters [password] : Redis password to connect using (blank)                      *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)             *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                   *
 * Returns               : 0 (success),1 (failure)                                      *
 *                                                                                      *
 ****************************************************************************************/
int redis_key_type(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_type";
	const char     *__key_name      = "redis.key.type[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "TYPE", param_key, REDIS_REPLY_STRING)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*************************************************************************************************
 *                                                                                               *
 * Custom Key            : redis.key.string.exists[server,port,timeout,password,database,key]    *
 *                                                                                               *
 * Function              : Tests if redis string exists                                          *
 * Parameters [server]   : Redis server address to connect                                       *
 * Parameters [port]     : Redis server port to connect                                          *
 * Parameters [timeout]  : Timeout in seconds                                                    *
 * Parameters [password] : Redis password to connect using (blank)                               *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                      *
 * Parameters [key]      : Key to check (key-a, key-b, key-c etc...)                             *
 * Returns               : 0 (no),1 (yes)                                                        *
 *                                                                                               *
 *************************************************************************************************/
int redis_key_string_exists(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_string_exists";
	const char     *__key_name      = "redis.key.string.exists[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match string
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "string")) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);}

	// If the redis key type does match string
	if (! redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "string")) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);}

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/******************************************************************************************************
 *                                                                                                    *
 * Custom Key            : redis.key.string.get[server,port,timeout,password,database,key,default]    *
 *                                                                                                    *
 * Function              : Get a redis string                                                         *
 * Parameters [server]   : Redis server address to connect                                            *
 * Parameters [port]     : Redis server port to connect                                               *
 * Parameters [timeout]  : Timeout in seconds                                                         *
 * Parameters [password] : Redis password to connect using (blank)                                    *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                           *
 * Parameters [key]      : Key to get (key-a, key-b, key-c etc...)                                    *
 * Parameters [default]  : Value default to return if no value is detected                            *
 * Returns               : 0 (success),1 (failure)                                                    *
 *                                                                                                    *
 ******************************************************************************************************/
int redis_key_string_get(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_string_get";
	const char     *__key_name      = "redis.key.string.get[server,port,timeout,password,database,key,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 7;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);
	param_default  = get_rparam(request,6);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "string")) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "GET", param_key, REDIS_REPLY_STRING)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*************************************************************************************************
 *                                                                                               *
 * Custom Key            : redis.key.string.length[server,port,timeout,password,database,key]    *
 *                                                                                               *
 * Function              : Gets a redis strings length                                           *
 * Parameters [server]   : Redis server address to connect                                       *
 * Parameters [port]     : Redis server port to connect                                          *
 * Parameters [timeout]  : Timeout in seconds                                                    *
 * Parameters [password] : Redis password to connect using (blank)                               *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                      *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                            *
 * Returns               : 0 (success),1 (failure)                                               *
 *                                                                                               *
 *************************************************************************************************/
int redis_key_string_length(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_string_length";
	const char     *__key_name      = "redis.key.string.length[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "string")) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "STRLEN", param_key, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/**************************************************************************************************
 *                                                                                                *
 * Custom Key            : redis.key.hash.discovery[server,port,timeout,password,database,key]    *
 *                                                                                                *
 * Function              : Discovers redis hash fields                                            *
 * Parameters [server]   : Redis server address to connect                                        *
 * Parameters [port]     : Redis server port to connect                                           *
 * Parameters [timeout]  : Timeout in seconds                                                     *
 * Parameters [password] : Redis password to connect using (blank)                                *
 * Parameters [datatype] : Data type to return (integer,float,string,text)                        *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                       *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                             *
 * Returns               : 0 (success),1 (failure)                                                *
 *                                                                                                *
 **************************************************************************************************/
int redis_key_hash_discovery(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_hash_discovery";
	const char     *__key_name      = "redis.key.hash.discovery[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key, *param_field, *param_default;
	struct          zbx_json j;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	int             discovered_instances = 0;
	int             count = 0;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "HKEYS", param_key, REDIS_REPLY_ARRAY)) {goto out;}

	// Initialise JSON for discovery
	zbx_json_init(&j,ZBX_JSON_STAT_BUF_LEN);

	// Create JSON array of discovered instances
	zbx_json_addarray(&j,ZBX_PROTO_TAG_DATA);

	// For every element
	for (count = 0; count < redisR->elements; count++) {

		// Open instance in JSON
		zbx_json_addobject(&j, NULL);

		zbx_json_addstring(&j, "{#DATABASE}", param_database, ZBX_JSON_TYPE_STRING);
		zbx_json_addstring(&j, "{#KEY}", param_key, ZBX_JSON_TYPE_STRING);
		zbx_json_addstring(&j, "{#FIELD}", redisR->element[count]->str, ZBX_JSON_TYPE_STRING);

		// Close instance in JSON 
		zbx_json_close(&j);

		// Increment discovered instances
		discovered_instances++;

	}

	// Finalise JSON for discovery
	zbx_json_close(&j);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Key (%s) discovered instances (%lld)",MODULE,zbx_key,discovered_instances);

	// Set result
	SET_STR_RESULT(result, strdup(j.buffer));

	// Set return
	ret = SYSINFO_RET_OK;

	// Free the json
	zbx_json_free(&j);

	// Free the reply
	freeReplyObject(redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/**********************************************************************************************
 *                                                                                            *
 * Custom Key            : redis.key.hash.count[server,port,timeout,password,database,key]    *
 *                                                                                            *
 * Function              : Gets a redis hash field count                                      *
 * Parameters [server]   : Redis server address to connect                                    *
 * Parameters [port]     : Redis server port to connect                                       *
 * Parameters [timeout]  : Timeout in seconds                                                 *
 * Parameters [password] : Redis password to connect using (blank)                            *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                   *
 * Parameters [key]      : Key to select the field from  (key-a, key-b, key-c etc...)         *
 * Returns               : 0 (success),1 (failure)                                            *
 *                                                                                            *
 **********************************************************************************************/
int redis_key_hash_count(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_hash_count";
	const char     *__key_name      = "redis.key.hash.count[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "HLEN", param_key, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************************
 *                                                                                             *
 * Custom Key            : redis.key.hash.exists[server,port,timeout,password,database,key]    *
 *                                                                                             *
 * Function              : Tests if redis hash exists                                          *
 * Parameters [server]   : Redis server address to connect                                     *
 * Parameters [port]     : Redis server port to connect                                        *
 * Parameters [timeout]  : Timeout in seconds                                                  *
 * Parameters [password] : Redis password to connect using (blank)                             *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                    *
 * Parameters [key]      : Key to check (key-a, key-b, key-c etc...)                           *
 * Returns               : 0 (no),1 (yes)                                                      *
 *                                                                                             *
 ***********************************************************************************************/
int redis_key_hash_exists(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_hash_exists";
	const char     *__key_name      = "redis.key.hash.exists[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database;
	char           *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match hash
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);}

	// If the redis key type does match hash
	if (! redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);}

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************************************
 *                                                                                                         *
 * Custom Key            : redis.key.hash.field.exists[server,port,timeout,password,database,key,field]    *
 *                                                                                                         *
 * Function              : Checks hash field exists                                                        *
 * Parameters [server]   : Redis server address to connect                                                 *
 * Parameters [port]     : Redis server port to connect                                                    *
 * Parameters [timeout]  : Timeout in seconds                                                              *
 * Parameters [password] : Redis password to connect using (blank)                                         *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                                *
 * Parameters [key]      : Key to select the field from (key-a, key-b, key-c etc...)                       *
 * Parameters [field]    : Field to check (field-a, field-b, field-c etc...)                               *
 * Returns               : 0 (no),1 (yes)                                                                  *
 *                                                                                                         *
 ***********************************************************************************************************/
int redis_key_hash_field_exists(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_hash_field_exists";
	const char     *__key_name      = "redis.key.hash.field.exists[server,port,timeout,password,database,key,field]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 7;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key, *param_field;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);
	param_field    = get_rparam(request,6);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Field", param_field, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                             {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {goto out;}

	// If the redis hash field does not exist
	if (redis_hash_field_check_exists(result, &ret, zbx_key, &redisC, param_key, param_field)) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);}

	// If the redis hash field does exist
	if (! redis_hash_field_check_exists(result, &ret, zbx_key, &redisC, param_key, param_field)) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);}

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/****************************************************************************************************************
 *                                                                                                              *
 * Custom Key            : redis.key.hash.field.get[server,port,timeout,password,database,key,field,default]    *
 *                                                                                                              *
 * Function              : Gets a redis hash field                                                              *
 * Parameters [server]   : Redis server address to connect                                                      *
 * Parameters [port]     : Redis server port to connect                                                         *
 * Parameters [timeout]  : Timeout in seconds                                                                   *
 * Parameters [password] : Redis password to connect using (blank)                                              *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                                     *
 * Parameters [key]      : Key to select the field from (key-a, key-b, key-c etc...)                            *
 * Parameters [field]    : Field to return (field-a, field-b, field-c etc...)                                   *
 * Parameters [default]  : Value default to return if no value is detected                                      *
 * Returns               : 0 (success),1 (failure)                                                              *
 *                                                                                                              *
 ****************************************************************************************************************/
int redis_key_hash_field_get(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_hash_field_get";
	const char     *__key_name      = "redis.key.hash.field.get[server,port,timeout,password,database,key,field,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 8;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key, *param_field, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;
	char            redisParams[MAX_LENGTH_STRING];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);
	param_field    = get_rparam(request,6);
	param_default  = get_rparam(request,7);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Field", param_field, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                             {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

        // Append the parameters
        strcat(redisParams,param_key);
        strcat(redisParams," ");
        strcat(redisParams,param_field);
 
	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {goto out;}

	// If the redis hash field does not exist
	if (redis_hash_field_check_exists(result, &ret, zbx_key, &redisC, param_key, param_field)) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "HGET", redisParams, REDIS_REPLY_STRING)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************************************
 *                                                                                                         *
 * Custom Key            : redis.key.hash.field.length[server,port,timeout,password,database,key,field]    *
 *                                                                                                         *
 * Function              : Gets a redis hash field length                                                  *
 * Parameters [server]   : Redis server address to connect                                                 *
 * Parameters [port]     : Redis server port to connect                                                    *
 * Parameters [timeout]  : Timeout in seconds                                                              *
 * Parameters [password] : Redis password to connect using (blank)                                         *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                                *
 * Parameters [key]      : Key to select the field from (key-a, key-b, key-c etc...)                       *
 * Parameters [field]    : Field to return (field-a, field-b, field-c etc...)                              *
 * Returns               : 0 (success),1 (failure)                                                         *
 *                                                                                                         *
 ***********************************************************************************************************/
int redis_key_hash_field_length(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_hash_field_length";
	const char     *__key_name      = "redis.key.hash.field.length[server,port,timeout,password,database,key,field]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 7;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key, *param_field;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;
	char            redisParams[MAX_LENGTH_STRING];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);
	param_field    = get_rparam(request,6);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Field", param_field, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                             {return ret;}

        // Append the parameters
        strcat(redisParams,param_key);
        strcat(redisParams," ");
        strcat(redisParams,param_field);
 
	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "hash")) {goto out;}

	// If the redis hash field does not exist
	if (redis_hash_field_check_exists(result, &ret, zbx_key, &redisC, param_key, param_field)) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "HSTRLEN", redisParams, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************************
 *                                                                                             *
 * Custom Key            : redis.key.list.exists[server,port,timeout,password,database,key]    *
 *                                                                                             *
 * Function              : Tests if redis list exists                                          *
 * Parameters [server]   : Redis server address to connect                                     *
 * Parameters [port]     : Redis server port to connect                                        *
 * Parameters [timeout]  : Timeout in seconds                                                  *
 * Parameters [password] : Redis password to connect using (blank)                             *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                    *
 * Parameters [key]      : Key to check (key-a, key-b, key-c etc...)                           *
 * Returns               : 0 (no),1 (yes)                                                      *
 *                                                                                             *
 ***********************************************************************************************/
int redis_key_list_exists(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_list_exists";
	const char     *__key_name      = "redis.key.list.exists[server,port,timeout,password,database,key]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match list
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "list")) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 0, redisR);}

	// If the redis key type does match list
	if (! redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "list")) {zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, 1, redisR);}

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/************************************************************************************************************
 *                                                                                                          *
 * Custom Key            : redis.key.list.get[server,port,timeout,password,database,key,element,default]    *
 *                                                                                                          *
 * Function              : Gets a redis list element                                                        *
 * Parameters [server]   : Redis server address to connect                                                  *
 * Parameters [port]     : Redis server port to connect                                                     *
 * Parameters [timeout]  : Timeout in seconds                                                               *
 * Parameters [password] : Redis password to connect using (blank)                                          *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                                 *
 * Parameters [key]      : Key to select the field from (key-a, key-b, key-c etc...)                        *
 * Parameters [element]  : Element to return (0, 1, 2 etc...)                                               *
 * Parameters [default]  : Value default to return if no value is detected                                  *
 * Returns               : 0 (success),1 (failure)                                                          *
 *                                                                                                          *
 ************************************************************************************************************/
int redis_key_list_get(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_list_get";
	const char     *__key_name      = "redis.key.list.get[server,port,timeout,password,database,key,element,default]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 8;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key, *param_element, *param_default;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;
	char            redisParams[MAX_LENGTH_STRING];

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);
	param_element  = get_rparam(request,6);
	param_default  = get_rparam(request,7);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}
	if (validate_param(result, zbx_key, "Element", param_element, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}
	if (validate_param(result, zbx_key, "Default", param_default, NO_DEFAULT, ALLOW_NULL_TRUE, NO_MIN, NO_MAX))                                         {return ret;}

        // Append the parameters
        strcat(redisParams,param_key);
        strcat(redisParams," ");
        strcat(redisParams,param_element);
 
	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match list
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "list")) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "LLEN", param_key, REDIS_REPLY_INTEGER)) {goto out;}

	// If the redis list element does not exist
	if (redisR->integer < atoi(param_element)) {

		// Set return
		zbx_ret_fail(result, &ret, LOG_LEVEL_DEBUG, zbx_key, "Redis list element does not exist", redisR);

		goto out;

	}

	// Free the reply
	freeReplyObject(redisR);

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "LINDEX", redisParams, REDIS_REPLY_STRING)) {goto out;}

	// Set return
	zbx_ret_string(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->str, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/***********************************************************************************************
 *                                                                                             *
 * Custom Key            : redis.key.list.length[server,port,timeout,password,database,key]    *
 *                                                                                             *
 * Function              : Gets the length of a redisk list key                                *
 * Parameters [server]   : Redis server address to connect                                     *
 * Parameters [port]     : Redis server port to connect                                        *
 * Parameters [timeout]  : Timeout in seconds                                                  *
 * Parameters [password] : Redis password to connect using (blank)                             *
 * Parameters [database] : Database to select the key from (1, 2, 3 etc...)                    *
 * Parameters [key]      : Key to return (key-a, key-b, key-c etc...)                          *
 * Returns               : 0 (success),1 (failure)                                             *
 *                                                                                             *
 ***********************************************************************************************/
int redis_key_list_length(AGENT_REQUEST *request,AGENT_RESULT *result)
{

	// Declare Variables
	const char     *__function_name = "redis_key_list_length";
	const char     *__key_name      = "redis.key.list.length[server,port,timeout,password,database,key,field]";
	int             ret = SYSINFO_RET_FAIL;
	char            zbx_key[MAX_LENGTH_KEY], zbx_msg[MAX_LENGTH_MSG];
	int             param_count = 6;
	char           *param_server, *param_port, *param_timeout, *param_password, *param_database, *param_key;
	redisContext   *redisC;
	redisReply     *redisR;
	struct timeval  timeout;
	char           *line;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Generate the zabbix key
	zbx_key_gen(request,zbx_key);

	// Validate parameter count
	if (validate_param_count(result, zbx_key, param_count, request->nparam, "!=")) {return ret;}

	// Assign parameters
	param_server   = get_rparam(request,0);
	param_port     = get_rparam(request,1);
	param_timeout  = get_rparam(request,2);
	param_password = get_rparam(request,3);
	param_database = get_rparam(request,4);
	param_key      = get_rparam(request,5);

	// If parameters are invalid
	if (validate_param(result, zbx_key, "Redis server", param_server, DEFAULT_REDIS_SERVER, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                          {return ret;}
	if (validate_param(result, zbx_key, "Redis port", param_port, DEFAULT_REDIS_PORT, ALLOW_NULL_FALSE, MIN_REDIS_PORT, MAX_REDIS_PORT))                {return ret;}
	if (validate_param(result, zbx_key, "Redis timeout", param_timeout, DEFAULT_REDIS_TIMEOUT, ALLOW_NULL_FALSE, MIN_REDIS_TIMEOUT, MAX_REDIS_TIMEOUT)) {return ret;}
	if (validate_param(result, zbx_key, "Database", param_database, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                      {return ret;}
	if (validate_param(result, zbx_key, "Key", param_key, NO_DEFAULT, ALLOW_NULL_FALSE, NO_MIN, NO_MAX))                                                {return ret;}

	// Create the redis session
	if ((redisC = redis_session(result, zbx_key, param_server, param_port, param_timeout, param_password)) == NULL) {return ret;}

	// Select the redis database
	if (redis_select_database(result, &ret, zbx_key, &redisC, param_database)) {goto out;}

	// If the redis key does not exist
	if (redis_key_check_exists(result, &ret, zbx_key, &redisC, param_key)) {goto out;}

	// If the redis key type does not match list
	if (redis_key_check_type(result, &ret, zbx_key, &redisC, param_key, "list")) {goto out;}

	// Run redis command
	if (redis_command(result, zbx_key, redisC, &redisR, "LLEN", param_key, REDIS_REPLY_INTEGER)) {goto out;}

	// Set return
	zbx_ret_integer(result, &ret, LOG_LEVEL_DEBUG, zbx_key, redisR->integer, redisR);

out:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

