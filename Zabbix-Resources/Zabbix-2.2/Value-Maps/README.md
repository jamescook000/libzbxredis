# libzbxredis Zabbix 2.X Value Maps

The following value maps are to be created prior to importing any of the Zabbix templates that use libzbxredis:

* Redis Info (persistence) : aof_enabled

  * Values[0 => No, 1 => Yes]

* Redis Info (persistence) : aof_rewrite_in_progress

  * Values [0 => No, 1 => Yes]

* Redis Info (persistence) : aof_rewrite_scheduled

  * Values [0 => No, 1 => Yes]

* Redis Info (persistence) : loading

  * Values [0 => No, 1 => Yes]

* Redis Info (persistence) : rdb_bgsave_in_progress

  * Values [0 => No, 1 => Yes]

* Redis Info (replication) : master_last_io_seconds_ago

  * Values [0 => Not Applicable]

* Redis Info (replication) : master_link_down_since_seconds

  * Values [0 => Not Applicable]

* Redis Info (replication) : master_port

  * Values [0 => Not Applicable]

* Redis Info (replication) : master_sync_in_progress

  * Values [0 => No, 1 => Yes, 2 => Not Applicable]

* Redis Info (replication) : repl_backlog_active

  * Values [0 => No, 1 => Yes]

* Redis Info (replication) : slave_priority

  * Values [0 => Not Applicable]

* Redis Info (replication) : slave_read_only

  * Values [0 => Not Applicable]

* Redis Info (replication) : slave_repl_offset

  * Values [0 => Not Applicable]

* Redis Key : Exists

  * Values [0 => No, 1 => Yes]

* Redis Server : Port Connect Status

  * Values [0 => Failed, 1 => Successful]

* Redis Server : Session Status

  * Values [0 => Failed, 1 => Successful]
