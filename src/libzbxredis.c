/*
**
** libzbxredis - A Redis monitoring module for Zabbix
** Copyright (C) 2016 - James Cook <james.cook000@gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
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

// Include libraries
#include <hiredis.h>
#include "libzbxredis.h"
#include "regex.h"

// Defines timeout setting for item processing
static int item_timeout = 0;

// Define custom keys
static ZBX_METRIC keys[] =
/*      KEY					FLAG		FUNCTION				TEST PARAMETERS */
{
	// Redis Item Keys
	{"libzbxredis.version",			0,		libzbxredis_version,			NULL},
	{"redis.session.status",		CF_HAVEPARAMS,	redis_session_status,			",,,,"},
	{"redis.session.duration",		CF_HAVEPARAMS,	redis_session_duration,			",,,,"},
	{"redis.command.supported",		CF_HAVEPARAMS,	redis_command_supported,		",,,,,PING"},
	{"redis.command.duration",		CF_HAVEPARAMS,	redis_command_duration,			",,,,,PING,"},
	{"redis.info",				CF_HAVEPARAMS,	redis_info,				",,,,,string,server,redis_version,"},
	{"redis.database.discovery",		CF_HAVEPARAMS,	redis_database_discovery,		",,,,"},
	{"redis.database.info",			CF_HAVEPARAMS,	redis_database_info,			",,,,,string,db0,keys,"},
	{"redis.slave.discovery",		CF_HAVEPARAMS,	redis_slave_discovery,			",,,,"},
	{"redis.slave.info",			CF_HAVEPARAMS,	redis_slave_info,			",,,,,string,slave0,ip,"},
	{"redis.ping",				CF_HAVEPARAMS,	redis_ping,				",,,,"},
	{"redis.time",				CF_HAVEPARAMS,	redis_time,				",,,,"},
	{"redis.lastsave",			CF_HAVEPARAMS,	redis_lastsave,				",,,,"},
	{"redis.role",				CF_HAVEPARAMS,	redis_role,				",,,,"},
	{"redis.keyspace.hit.ratio",		CF_HAVEPARAMS,	redis_keyspace_hit_ratio,		",,,,"},
	{"redis.slowlog.length",		CF_HAVEPARAMS,	redis_slowlog_length,			",,,,"},
	{"redis.config",			CF_HAVEPARAMS,	redis_config,				",,,,string,logfile,"},
	{"redis.client.discovery",		CF_HAVEPARAMS,	redis_client_discovery,			",,,,"},
	{"redis.client.info",			CF_HAVEPARAMS,	redis_client_info,			",,,,,string,clientname,addr"},
	{"redis.key.exists",			CF_HAVEPARAMS,	redis_key_exists,			",,,,,key-a"},
	{"redis.key.ttl",			CF_HAVEPARAMS,	redis_key_ttl,				",,,,,key-a"},
	{"redis.key.pttl",			CF_HAVEPARAMS,	redis_key_pttl,				",,,,,key-a"},
	{"redis.key.type",			CF_HAVEPARAMS,	redis_key_type,				",,,,,key-a"},
	{"redis.key.string.exists",		CF_HAVEPARAMS,	redis_key_string_exists,		",,,,,key-a"},
	{"redis.key.string.get",		CF_HAVEPARAMS,	redis_key_string_get,			",,,,,key-a,"},
	{"redis.key.string.length",		CF_HAVEPARAMS,	redis_key_string_length,		",,,,,key-a"},
	{"redis.key.hash.discovery",		CF_HAVEPARAMS,	redis_key_hash_discovery,		",,,,,key-a"},
	{"redis.key.hash.count",		CF_HAVEPARAMS,	redis_key_hash_count,			",,,,,key-a"},
	{"redis.key.hash.exists",		CF_HAVEPARAMS,	redis_key_hash_exists,			",,,,,key-a"},
	{"redis.key.hash.field.exists",		CF_HAVEPARAMS,	redis_key_hash_field_exists,		",,,,,key-a,field-a"},
	{"redis.key.hash.field.get",		CF_HAVEPARAMS,	redis_key_hash_field_get,		",,,,,key-a,field-a,"},
	{"redis.key.hash.field.length",		CF_HAVEPARAMS,	redis_key_hash_field_length,		",,,,,key-a,field-a"},
	{"redis.key.list.exists",		CF_HAVEPARAMS,	redis_key_list_exists,			",,,,,key-a"},
	{"redis.key.list.get",			CF_HAVEPARAMS,	redis_key_list_get,			",,,,,key-a,element-a,"},
	{"redis.key.list.length",		CF_HAVEPARAMS,	redis_key_list_length,			",,,,,key-a"},

	// Null terminator
	{NULL}
};

// Define compiled regular expressions
regex_t regexCompiled_INFO;
regex_t regexCompiled_INFO_MULTI_VALUE;
regex_t regexCompiled_INFO_SINGLE_VALUE;
regex_t regexCompiled_INFO_SLAVE;
regex_t regexCompiled_INFO_DATABASE;

/******************************************************************************
 *                                                                            *
 * Function   : Returns the version of the module api                         *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int zbx_module_api_version()
{

	// Return version
	return ZBX_MODULE_API_VERSION_ONE;

}

/*******************************************************************************
*                                                                              *
* Function   : This function will initialise the zabbix module                 *
* Returns    : 0 (success), 1 (failure)                                        *
*                                                                              *
*******************************************************************************/
int zbx_module_init() { 

	// log version on startup
	zabbix_log(LOG_LEVEL_INFORMATION,"Module (%s): Initialising",MODULE);

	// Compile regular expression and fail if so
	if (regcomp(&regexCompiled_INFO,REGEX_MATCH_INFO,REG_EXTENDED)) {

		// Log message
		zabbix_log(LOG_LEVEL_ERR,"Module (%s): Compile regular expression failed ([%s])",MODULE,REGEX_MATCH_INFO);

		return ZBX_MODULE_FAIL;

	}

	// Compile regular expression and fail if so
	if (regcomp(&regexCompiled_INFO_DATABASE,REGEX_MATCH_INFO_DATABASE,REG_EXTENDED)) {

		// Log message
		zabbix_log(LOG_LEVEL_ERR,"Module (%s): Compile regular expression failed ([%s])",MODULE,REGEX_MATCH_INFO_DATABASE);

		return ZBX_MODULE_FAIL;

	}

	// Compile regular expression and fail if so
	if (regcomp(&regexCompiled_INFO_MULTI_VALUE,REGEX_MATCH_INFO_MULTI_VALUE,REG_EXTENDED)) {

		// Log message
		zabbix_log(LOG_LEVEL_ERR,"Module (%s): Compile regular expression failed ([%s])",MODULE,REGEX_MATCH_INFO_MULTI_VALUE);

		return ZBX_MODULE_FAIL;

	}

	// Compile regular expression and fail if so
	if (regcomp(&regexCompiled_INFO_SINGLE_VALUE,REGEX_MATCH_INFO_SINGLE_VALUE,REG_EXTENDED)) {

		// Log message
		zabbix_log(LOG_LEVEL_ERR,"Module (%s): Compile regular expression failed ([%s])",MODULE,REGEX_MATCH_INFO_SINGLE_VALUE);

		return ZBX_MODULE_FAIL;

	}

	// Compile regular expression and fail if so
	if (regcomp(&regexCompiled_INFO_SLAVE,REGEX_MATCH_INFO_SLAVE,REG_EXTENDED)) {

		// Log message
		zabbix_log(LOG_LEVEL_ERR,"Module (%s): Compile regular expression failed ([%s])",MODULE,REGEX_MATCH_INFO_SLAVE);

		return ZBX_MODULE_FAIL;

	}

	// Return success
	return ZBX_MODULE_OK;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will uninitialise the zabbix module             *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int zbx_module_uninit() { 

	// Free compiled regex
	regfree(&regexCompiled_INFO);
	regfree(&regexCompiled_INFO_DATABASE);
	regfree(&regexCompiled_INFO_MULTI_VALUE);
	regfree(&regexCompiled_INFO_SINGLE_VALUE);
	regfree(&regexCompiled_INFO_SLAVE);

	// log version on startup
	zabbix_log(LOG_LEVEL_INFORMATION,"Module (%s): Uninitialising",MODULE);

	// Return success
	return ZBX_MODULE_OK; 

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will set the item timeout                       *
 * Returns    : Void                                                          *
 *                                                                            *
 ******************************************************************************/
void zbx_module_item_timeout(int timeout) { 

	// Set item timeout
	item_timeout = timeout;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will return a list of supported keys            *
 * Returns    : Zabbix Metric                                                 *
 *                                                                            *
 ******************************************************************************/
ZBX_METRIC *zbx_module_item_list() {

	// Return keys
	return keys;

}

/********************************************************************************************************
 *                                                                                                      *
 * Function   : This function will construct a complete zabbix key including parameters from a request  *
 * Returns    : 0 (success), 1 (failure)                                                                *
 *                                                                                                      *
 ********************************************************************************************************/
int zbx_key_gen(AGENT_REQUEST *request, char *zbx_key)
{

	// Declare Variables
	int count = 0;

	// Initialise string
	zbx_strlcpy(zbx_key,"",sizeof(zbx_key));

	// Append keyname
	strcat(zbx_key,request->key);

	// Ammend params if present
	if(request->nparam) {

		// Open the parameters
		strcat(zbx_key,"[");

		// For every parameter
		for(count=0;count<request->nparam;count++) {

			// If there are more parameters then append a comma
			if(count) {

				strcat(zbx_key,",");

			}

			// Append the parameter
			strcat(zbx_key,request->params[count]);

		}

		// Close the parameters
		strcat(zbx_key,"]");

	}

	// Return success
	return 0;

}

/*************************************************************
 *                                                           *
 * Function   : This function will set the return and log    *
 * Returns    : 0 (success), 1 (failure)                     *
 *                                                           *
 *************************************************************/
int zbx_ret_fail(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *zbx_msg, redisReply *redisR)
{

	// Set return
	*ret = SYSINFO_RET_FAIL;

       	// Log message
       	zabbix_log(log_level,"Module (%s) - %s - Key %s",MODULE,zbx_msg,zbx_key);

       	// Set message
       	SET_MSG_RESULT(result,strdup(zbx_msg));

	// If the reply needs to be cleaned up
	if (redisR != NULL) {freeReplyObject(redisR);}

	// Return success
	return 0;

}

/*************************************************************
 *                                                           *
 * Function   : This function will set the return and log    *
 * Returns    : 0 (success), 1 (failure)                     *
 *                                                           *
 *************************************************************/
int zbx_ret_string(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *value, redisReply *redisR)
{

	// Set return
	*ret = SYSINFO_RET_OK;

	// Log message
	zabbix_log(log_level,"Module (%s): Key (%s) returned value (%s)",MODULE,zbx_key,value);

	// Set result
	SET_STR_RESULT(result,zbx_strdup(NULL,value));

	// If the reply needs to be cleaned up
	if (redisR != NULL) {freeReplyObject(redisR);}

	// Return success
	return 0;

}

/*************************************************************
 *                                                           *
 * Function   : This function will set the return and log    *
 * Returns    : 0 (success), 1 (failure)                     *
 *                                                           *
 *************************************************************/
int zbx_ret_string_convert(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *value, char *datatype, redisReply *redisR)
{

	// For integer keys
	if (strcmp(datatype,"integer") == 0) {zbx_ret_integer(result, ret, log_level, zbx_key, atoi(value), redisR);}

	// For double keys
	if (strcmp(datatype,"float") == 0) {zbx_ret_float(result, ret, log_level, zbx_key, atof(value), redisR);}

	// For string keys
	if (strcmp(datatype,"string") == 0) {zbx_ret_string(result, ret, log_level, zbx_key, value, redisR);}

	// For text keys
	if (strcmp(datatype,"text") == 0) {zbx_ret_string(result, ret, log_level, zbx_key, value, redisR);}

	// Return success
	return 0;

}

/*************************************************************
 *                                                           *
 * Function   : This function will set the return and log    *
 * Returns    : 0 (success), 1 (failure)                     *
 *                                                           *
 *************************************************************/
int zbx_ret_text(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *value, redisReply *redisR)
{

	// Set return
	*ret = SYSINFO_RET_OK;

	// Log message
	zabbix_log(log_level,"Module (%s): Key (%s) returned value (%s)",MODULE,zbx_key,value);

	// Set result
	SET_STR_RESULT(result,zbx_strdup(NULL,value));

	// If the reply needs to be cleaned up
	if (redisR != NULL) {freeReplyObject(redisR);}

	// Return success
	return 0;

}

/*************************************************************
 *                                                           *
 * Function   : This function will set the return and log    *
 * Returns    : 0 (success), 1 (failure)                     *
 *                                                           *
 *************************************************************/
int zbx_ret_integer(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, unsigned long long value, redisReply *redisR)
{

	// Set return
	*ret = SYSINFO_RET_OK;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Key (%s) returned value (%llu)",MODULE,zbx_key,value);

	// Set result
	SET_UI64_RESULT(result,value);

	// If the reply needs to be cleaned up
	if (redisR != NULL) {freeReplyObject(redisR);}

	// Return success
	return 0;

}

/*************************************************************
 *                                                           *
 * Function   : This function will set the return and log    *
 * Returns    : 0 (success), 1 (failure)                     *
 *                                                           *
 *************************************************************/
int zbx_ret_float(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, float value, redisReply *redisR)
{

	// Set return
	*ret = SYSINFO_RET_OK;

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Key (%s) returned value (%f)",MODULE,zbx_key,value);

	// Set result
	SET_DBL_RESULT(result,value);

	// If the reply needs to be cleaned up
	if (redisR != NULL) {freeReplyObject(redisR);}

	// Return success
	return 0;

}

/******************************************************************************
 *                                                                            *
 * Custom key : libzbxredis.version                                           *
 *                                                                            *
 * Function   : Returns the version string of the libzbxredis module          *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int libzbxredis_version(AGENT_REQUEST *request, AGENT_RESULT *result)
{

	// Declare Variables
	int         ret = SYSINFO_RET_FAIL;
	const char  *__function_name = "libzbxredis_version";
	char         zbx_msg[MAX_LENGTH_MSG] = "";

	// Log message 
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Enter function %s",MODULE,__function_name);

	// Set result
	SET_STR_RESULT(result, strdup(PACKAGE_STRING));

	// Set return
	ret = SYSINFO_RET_OK;
    
	// Log message 
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s): Exit function %s",MODULE,__function_name);

	return ret;

}

/*************************************************************
 *                                                           *
 * Function   : This function will validate standard params  *
 * Returns    : 0 (valid), 1 (invalid)                       *
 *                                                           *
 *************************************************************/
int validate_param_count(AGENT_RESULT *result, char *zbx_key, int param_count, int nparam, char *condition)
{

	// Declar variables
	char zbx_msg[MAX_LENGTH_MSG] = "";

	// If the condition is !=
	if (strcmp(condition,"!=") == 0 && param_count != nparam) {

		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Invalid parameter count specified, expected %d received %d",param_count,nparam);

		goto param_count_invalid;

	}

param_count_valid:

	return 0;

param_count_invalid:

        // Log message
        zabbix_log(LOG_LEVEL_DEBUG,"Module (%s) - %s - Key %s",MODULE,zbx_msg,zbx_key);

        // Set message
        SET_MSG_RESULT(result,strdup(zbx_msg));

	return 1;

}

/*************************************************************
 *                                                           *
 * Function   : This function will validate standard params  *
 * Returns    : 0 (valid), 1 (invalid)                       *
 *                                                           *
 *************************************************************/
int validate_param(AGENT_RESULT *result,char *zbx_key, char *param, char *value, char *value_default, int allow_empty, int min, int max)
{

	// Declare variables
	char zbx_msg[MAX_LENGTH_MSG] = "";

	// If the value is empty and there is a default value
	if (strlen(value) == 0 && strlen(value_default) > 0) {

		// Set the value to default
		zbx_strlcpy(value,value_default,MAX_LENGTH_PARAM);

	}

	// If the value is empty and thats not allowed
	if (strlen(value) == 0 && ! allow_empty) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"%s must not be empty",param);

		goto param_invalid;

	}

	// If there is a minimum required
	if (min > 0 && atol(value) < min) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"%s must be an integer greater than or equal to %lld",param,min);

		goto param_invalid;

	}

	// If there is a maximum required
	if (max > 0 && atol(value) > max) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"%s must be an integer less than or equal to %lld",param,max);

		goto param_invalid;

	}

	// For specific parameters that required specific values
	if (strcmp(param,"Datatype") == 0) {

		// Validate value contents
		if (strcmp(value,"integer") != 0 &&
		    strcmp(value,"float") != 0 &&
		    strcmp(value,"string") != 0 &&
		    strcmp(value,"text") != 0) {

			// Form message
			zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"%s must be an integer,float,string,text",param);

			goto param_invalid;

		}

	}

param_valid:

	return 0;

param_invalid:

        // Log message
        zabbix_log(LOG_LEVEL_DEBUG,"Module (%s) - %s - Key %s",MODULE,zbx_msg,zbx_key);

        // Set message
        SET_MSG_RESULT(result,strdup(zbx_msg));

	return 1;

}

/*************************************************************
 *                                                           *
 * Function   : This function confirm redis command support  *
 * Returns    : 0 (supported), 1 (unsupported)               *
 *                                                           *
 *************************************************************/
int redis_command_is_supported(redisContext *redisC, char *command, char *zbx_key, char *zbx_msg)
{

	// Declare variables
	redisReply *redisR;

	// Run the redis command
	redisR = redisCommand(redisC,"COMMAND INFO %s",command);

	// If the connection is lost
	if(redisR == NULL) {goto error_connection_lost;}

	// If the reply is not valid
	if (redis_reply_valid(redisR->type,REDIS_REPLY_ARRAY,"COMMAND INFO",zbx_key,zbx_msg) == 1) {goto error_free_reply;}

	// If the command is supported
	if (redisR->element[0]->elements) {

		// Free the reply
		freeReplyObject(redisR);

		// Return supported
		return 0;

	}

	// Form message
	zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Required redis command (%s) is not supported by the redis server",MODULE,command);

	// Free the reply
	freeReplyObject(redisR);

	// Return unsupported
	return 1;

error_connection_lost:

	// Form message
	zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis connection lost (%s)",redisC->errstr);

	goto error;

error_free_reply:

	// Free the reply
	freeReplyObject(redisR);

error:

        // Log message
        zabbix_log(LOG_LEVEL_DEBUG,"Module (%s) - %s - Key %s",MODULE,zbx_msg,zbx_key);

out:

	// Return unsupported
	return 1;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will create a redis session on a redis server   *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
redisContext * redis_session(AGENT_RESULT *result, char *zbx_key, char *redis_server, char *redis_port, char *redis_timeout, char *redis_password)
{

	// Declare Variables
	char            zbx_msg[MAX_LENGTH_MSG] = "";
	redisReply     *redisR;
	redisContext   *redisC;
	struct timeval  timeout;

	// Set Timeout
	timeout.tv_sec = atol(redis_timeout);
	timeout.tv_usec = 0;

	// Attempt the connection
	redisC = redisConnectWithTimeout(redis_server,atol(redis_port),timeout);

	// If there was an error connecting
	if (redisC == NULL || redisC->err) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis connection failed (Unknown)");

		// If there is an error message
		if (redisC->err) {

			// Form message
			zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis connection failed (%s)",redisC->errstr);

		}

		goto session_invalid;

	}

	// Authenticate with blank password (The assumption is that there could be a blank password)
	if (strlen(redis_password) == 0) {redisR = redisCommand(redisC,"AUTH ''");}
	if (strlen(redis_password) > 0)  {redisR = redisCommand(redisC,"AUTH %s",redis_password);}

	// If the connection is lost
	if(redisR == NULL) {goto error_connection_lost;}

	// Free the reply
	freeReplyObject(redisR);

	// Test whether authentication has been successful
	redisR = redisCommand(redisC,"ECHO Authentication-Test");

	// If the connection is lost
	if(redisR == NULL) {goto error_connection_lost;}

	// If the authentication failed
	if (strcmp(redisR->str,"NOAUTH Authentication required.") == 0) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis authentication failed",MODULE);

		// Free the reply
		freeReplyObject(redisR);

		goto session_invalid;

	}

	// Free the reply
	freeReplyObject(redisR);

	// We want to set the client name in order to exclude from client discovery
	redisR = redisCommand(redisC,"CLIENT SETNAME %s",MODULE);

	// If the connection is lost
	if(redisR == NULL) {goto error_connection_lost;}

	// Free the reply
	freeReplyObject(redisR);

	goto session_valid;

error_connection_lost:

	// Form message
	zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis connection lost (%s)",redisC->errstr);

	goto session_invalid;

session_valid:

	return redisC;

session_invalid:

	// Free the context
	redisFree(redisC);

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s) - %s - Key %s",MODULE,zbx_msg,zbx_key);

	// Set message
	SET_MSG_RESULT(result,strdup(zbx_msg));

	return NULL;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will run a redis command and set the reply      *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int redis_command(AGENT_RESULT *result, char *zbx_key, redisContext *redisC, redisReply **redisRptr, char *command, char *param, int redisReplyType)
{

	// Declare Variables
	char          zbx_msg[MAX_LENGTH_MSG] = "";
	char          redisCmd[MAX_LENGTH_STRING];
	redisReply   *redisR;

	// If there are parameters
	if (param != NULL) {zbx_snprintf(redisCmd,MAX_LENGTH_STRING,"%s %s",command,param);}

	// If there are no parameters
	if (param == NULL) {zbx_snprintf(redisCmd,MAX_LENGTH_STRING,"%s",command);}

	// If the connection is lost
	if (redisC == NULL || redisC->err) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis connection lost (%s)",redisC->errstr);

		goto command_invalid;

	}

	// Run the redis command
	redisR = redisCommand(redisC,redisCmd);

	// If the connection is lost
	if (redisR == NULL) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis connection lost (%s)",redisC->errstr);

		goto command_invalid;

	}

	// If the reply type is an error
	if (redisR->type == REDIS_REPLY_ERROR) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis command error (%s)",redisR->str);

		goto command_invalid;


	}

	// If the redis reply type is to be checked
	if (redisReplyType == 9999) {goto command_valid;}

	// If the reply is not valid
	if (redis_reply_valid(redisR->type,redisReplyType,command,zbx_key,zbx_msg) == 1) {goto command_invalid;}

command_valid:

	// Assign the reply
	*redisRptr = redisR;

	return 0;

command_invalid:

	// Log message
	zabbix_log(LOG_LEVEL_DEBUG,"Module (%s) - %s - Key %s",MODULE,zbx_msg,zbx_key);

	// Set message
	SET_MSG_RESULT(result,strdup(zbx_msg));

	return 1;

}

/*************************************************************************
 *                                                                       *
 * Function   : This function will check if a redis reply is valid       *
 * Returns    : 0 (success), 1 (failure)                                 *
 *                                                                       *
 *************************************************************************/
int redis_reply_valid(int reply_received, int reply_expected, char *command, char *zbx_key, char *zbx_msg)
{

	// Declare Variables
	char reply_received_text[128] = "Unknown";
	char reply_expected_text[128] = "Unknown";

	// Set the reply type that has been received and expected
	if(reply_received == REDIS_REPLY_STRING)  {zbx_strlcpy(reply_received_text,"STRING",sizeof(reply_received_text));}
	if(reply_received == REDIS_REPLY_INTEGER) {zbx_strlcpy(reply_received_text,"INTEGER",sizeof(reply_received_text));}
	if(reply_received == REDIS_REPLY_ARRAY)   {zbx_strlcpy(reply_received_text,"ARRAY",sizeof(reply_received_text));}
	if(reply_received == REDIS_REPLY_STATUS)  {zbx_strlcpy(reply_received_text,"STATUS",sizeof(reply_received_text));}
	if(reply_received == REDIS_REPLY_NIL)     {zbx_strlcpy(reply_received_text,"NIL",sizeof(reply_received_text));}
	if(reply_received == REDIS_REPLY_ERROR)   {zbx_strlcpy(reply_received_text,"ERROR",sizeof(reply_received_text));}
	if(reply_expected == REDIS_REPLY_STRING)  {zbx_strlcpy(reply_expected_text,"STRING",sizeof(reply_expected_text));}
	if(reply_expected == REDIS_REPLY_INTEGER) {zbx_strlcpy(reply_expected_text,"INTEGER",sizeof(reply_expected_text));}
	if(reply_expected == REDIS_REPLY_ARRAY)   {zbx_strlcpy(reply_expected_text,"ARRAY",sizeof(reply_expected_text));}
	if(reply_expected == REDIS_REPLY_STATUS)  {zbx_strlcpy(reply_expected_text,"STATUS",sizeof(reply_expected_text));}
	if(reply_expected == REDIS_REPLY_NIL)     {zbx_strlcpy(reply_expected_text,"NIL",sizeof(reply_expected_text));}
	if(reply_expected == REDIS_REPLY_ERROR)   {zbx_strlcpy(reply_expected_text,"ERROR",sizeof(reply_expected_text));}

	// If the reply is not a match
	if (reply_received != reply_expected) {

		// Form message
		zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Redis reply invalid - Error (Command %s Expected %s: Received %s)",command,reply_expected_text,reply_received_text);

		// Return failure
		return 1;

	}

	// Return success
	return 0;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will get the value depending on the required    *
 *              field name and whether the data is single or multi value      *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int redis_get_value(char *redis_field, char *redis_data, char *redis_search, char *redis_value)
{

	// Declare variables
	char            regex[MAX_LENGTH_REGEX] = "";
        regex_t         regexCompiled;
        regmatch_t      regexGroups[2];
	char            zbx_msg[MAX_LENGTH_MSG] = "";

	// If the line is a multi value (ie field=val,field=val)
	if (regexec(&regexCompiled_INFO_MULTI_VALUE,redis_data,1,regexGroups,0) == 0) {

		// Lets build the regex to match the required field
		strcat(regex,"");
		strcat(regex,redis_search);
		strcat(regex,"=([^, ]*)");

		// Compile the regular expression and if it fails
		if (regcomp(&regexCompiled,regex,REG_EXTENDED)) {

			// Form message
			zbx_snprintf(zbx_msg,MAX_LENGTH_MSG,"Unable to compile regular expression ([%s])",regex);

			// Log message
			zabbix_log(LOG_LEVEL_DEBUG,"%s",zbx_msg);

			goto failure;

		}

		// If the line contains the searched field
		if (regexec(&regexCompiled,redis_data,2,regexGroups,0) == 0) {

			// Copy the value
			zbx_strlcpy(redis_value,redis_data + regexGroups[1].rm_so,regexGroups[1].rm_eo - regexGroups[1].rm_so + 1);

			// Free compiled regex
			regfree(&regexCompiled);

			goto success;

		}

	}

	// If the line is a single value (ie val)
	if (regexec(&regexCompiled_INFO_MULTI_VALUE,redis_data,1,regexGroups,0) == 1) {

		// If the field matches the search
		if (strcmp(redis_field,redis_search) == 0) {

			// Copy the value
			zbx_strlcpy(redis_value,redis_data,strlen(redis_data)+1);

			goto success;

		}

	}

failure:

	// Return success
	return 1;

success:

	// Return success
	return 0;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will select a redis database                    *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int redis_select_database(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *database)
{

	// Declare Variables
	redisReply     *redisR;

	// Run redis command
	if (redis_command(result, zbx_key, *redisCptr, &redisR, "SELECT", database, REDIS_REPLY_STATUS)) {return 1;}

	// If the database has not been selected
	if (strcmp(redisR->str,"OK") != 0) {

		// Set the return
		zbx_ret_fail(result, ret, LOG_LEVEL_DEBUG, zbx_key, "Redis database does not exist", redisR);

		return 1;

	}

	// Free the reply
	freeReplyObject(redisR);

	return 0;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will check if a redis key exists                *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int redis_key_check_exists(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *key)
{

	// Declare Variables
	redisReply *redisR;

        // Run redis command
        if (redis_command(result, zbx_key, *redisCptr, &redisR, "EXISTS", key, REDIS_REPLY_INTEGER)) {return 1;}

        // If the key does not exist
        if (redisR->integer != 1) {

                // Set the return
                zbx_ret_fail(result, ret, LOG_LEVEL_DEBUG, zbx_key, "Redis key does not exist", redisR);

                return 1;

        }

        // Free the reply
        freeReplyObject(redisR);

	return 0;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will check if a redis key type matches          *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int redis_key_check_type(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *key, char *type)
{

	// Declare Variables
	redisReply *redisR;

        // Run redis command
        if (redis_command(result, zbx_key, *redisCptr, &redisR, "TYPE", key, REDIS_REPLY_INTEGER)) {return 1;}

        // If the key is not the correct type
        if (strcmp(redisR->str,type) != 0) {

                // Set the return
                zbx_ret_fail(result, ret, LOG_LEVEL_DEBUG, zbx_key, "Redis key type does not match", redisR);

                return 1;

        }

        // Free the reply
        freeReplyObject(redisR);

	return 0;

}

/******************************************************************************
 *                                                                            *
 * Function   : This function will check if a redis hash field exists         *
 * Returns    : 0 (success), 1 (failure)                                      *
 *                                                                            *
 ******************************************************************************/
int redis_hash_field_check_exists(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *hash, char *field)
{

	// Declare Variables
	redisReply *redisR;
	char        redisParams[MAX_LENGTH_STRING];

	// Append the parameters
	strcat(redisParams,hash);
	strcat(redisParams," ");
	strcat(redisParams,field);

        // Run redis command
        if (redis_command(result, zbx_key, *redisCptr, &redisR, "HEXISTS", redisParams, REDIS_REPLY_INTEGER)) {return 1;}

        // If the hash field does not exist
        if (redisR->integer != 1) {

                // Set the return
                zbx_ret_fail(result, ret, LOG_LEVEL_DEBUG, zbx_key, "Redis hash field does not exist", redisR);

                return 1;

        }

        // Free the reply
        freeReplyObject(redisR);

	return 0;

}

