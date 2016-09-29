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

#ifndef LIBZBXREDIS_H
#define LIBZBXREDIS_H

#include <ctype.h>
#include <stdarg.h>
#include <arpa/inet.h>

// Hiredis headers
#include <hiredis.h>

// Zabbix source headers
#include <sysinc.h>
#include <module.h>
#include <common.h>
#include <time.h>
#include <log.h>
#include <zbxjson.h>
#include <regex.h>

// Default module name
#define MODULE "libzbxredis.so"

// Define max lengths
#define MAX_LENGTH_PARAM 255
#define MAX_LENGTH_STRING 255
#define MAX_LENGTH_KEY 8192
#define MAX_LENGTH_VALUE 8192
#define MAX_LENGTH_MSG 8192
#define MAX_LENGTH_LOG 8192
#define MAX_LENGTH_TEXT 8192
#define MAX_LENGTH_LINE 8192
#define MAX_LENGTH_REGEX 8192

// Default values
#define DEFAULT_REDIS_SERVER  "127.0.0.1"
#define DEFAULT_REDIS_PORT "6379"
#define DEFAULT_REDIS_PASS ""
#define DEFAULT_REDIS_TIMEOUT "5"

// Min & Max values
#define MIN_REDIS_PORT 1
#define MAX_REDIS_PORT 65535
#define MIN_REDIS_TIMEOUT 1
#define MAX_REDIS_TIMEOUT 30

// Regular expressions
#define REGEX_MATCH_INFO_MULTI_VALUE "^.*=.*$"
#define REGEX_MATCH_INFO_SINGLE_VALUE "^[^:]+:[^=]+$"
#define REGEX_MATCH_INFO "^([^:]*):(.*)$" 
#define REGEX_MATCH_INFO_SLAVE "^slave([0-9]+):(.*)$" 
#define REGEX_MATCH_INFO_DATABASE "^db([0-9]+):(.*)$" 

// Parameter validation
#define NO_DEFAULT ""
#define NO_MIN -1
#define NO_MAX -1
#define ALLOW_NULL_TRUE 1
#define ALLOW_NULL_FALSE 0

// Define compiled regular expressions
extern regex_t regexCompiled_INFO;
extern regex_t regexCompiled_INFO_MULTI_VALUE;
extern regex_t regexCompiled_INFO_SINGLE_VALUE;
extern regex_t regexCompiled_INFO_SLAVE;
extern regex_t regexCompiled_INFO_DATABASE;

// function to determine if a string is null or empty
#define strisnull(c) (NULL == c || '\0' == *c)

// Define generic functions
int zbx_key_gen(AGENT_REQUEST *request, char *zbx_key);
int zbx_ret_fail(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *zbx_msg, redisReply *redisR);
int zbx_ret_string(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *value, redisReply *redisR);
int zbx_ret_string_convert(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, char *value, char *datatype, redisReply *redisR);
int zbx_ret_integer(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, unsigned long long value, redisReply *redisR);
int zbx_ret_float(AGENT_RESULT *result, int *ret, int log_level, char *zbx_key, float value, redisReply *redisR);
int libzbxredis_version(AGENT_REQUEST *request, AGENT_RESULT *result);

// Define validation functions
int validate_param(AGENT_RESULT *result, char *zbx_key, char *param, char *value, char *value_default, int allow_empty, int min, int max);
int validate_param_count(AGENT_RESULT *result, char *zbx_key, int param_count, int nparam, char *condition);

// Define redis functions
redisContext * redis_session(AGENT_RESULT *result, char *zbx_key, char *redis_server, char *redis_port, char *redis_timeout, char *redis_password);
int redis_command(AGENT_RESULT *result, char *zbx_key, redisContext *redisC, redisReply **redisRptr, char *command, char *param, int redisReplyType);
int redis_command_is_supported(redisContext *redisC, char *command, char *zbx_key, char *zbx_msg);
int redis_reply_valid(int reply_received, int reply_expected, char *command, char *zbx_key, char *zbx_msg);
int redis_get_value(char *redis_field, char *redis_data, char *redis_search, char *redis_value);
int redis_select_database(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *database);
int redis_key_check_exists(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *key);
int redis_key_check_type(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *key, char *type);
int redis_hash_field_check_exists(AGENT_RESULT *result, int *ret, char *zbx_key, redisContext **redisCptr, char *hash, char *field);

// Define redis key functions
int redis_session_status(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_session_duration(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_command_duration(AGENT_REQUEST *request,AGENT_RESULT *result);
int redis_command_supported(AGENT_REQUEST *request,AGENT_RESULT *result);
int redis_info(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_database_discovery(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_database_info(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_slave_discovery(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_slave_info(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_ping(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_time(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_lastsave(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_role(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_keyspace_hit_ratio(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_slowlog_length(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_config(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_client_discovery(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_client_info(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_exists(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_ttl(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_pttl(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_type(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_string_exists(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_string_get(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_string_length(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_hash_discovery(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_hash_count(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_hash_exists(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_hash_field_exists(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_hash_field_get(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_hash_field_length(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_list_exists(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_list_get(AGENT_REQUEST *request, AGENT_RESULT *result);
int redis_key_list_length(AGENT_REQUEST *request, AGENT_RESULT *result);

#endif
