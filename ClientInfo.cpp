#include "StdAfx.h"
#include "ClientInfo.h"
#include "BaseConnection.h"
#include <stdlib.h>


char * CClientInfo::m_wangguan_table = "wangguan";
char * CClientInfo::m_onoff_record_table = "onoff_record";
char * CClientInfo::m_client_record_table = "client_record";
char * CClientInfo::m_conn_param_table = "conn_param";

CClientInfo::CClientInfo(void)
{

}

CClientInfo::~CClientInfo(void)
{

}

/* CRelationMgr::Insert()时调用，用于网关初次上线添加记录或重新上线时更新记录 */
bool CClientInfo::AddWangguan(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	char sql_line_exist[NORMAL_XG_BUF_LEN];

	int sql_len_exist = _snprintf_s(sql_line_exist, NORMAL_XG_BUF_LEN-1, _TRUNCATE,
		"SELECT serial FROM %s WHERE serial='%s'", m_wangguan_table, name);
	sql_line_exist[sql_len_exist] = '\0';
	MYSQL_RES * existresults = BeginQuery(sql_line_exist);	// 查询网关名称看是否已存在
	if (NULL == existresults)
	{
		return FALSE;
	}

	if(existresults->row_count == 0)	// 如果不存在，新添加一项网关上线记录
	{	
		EndQuery(existresults);

		int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
			"INSERT INTO %s(serial, onoff) VALUES('%s', 1)",
			m_wangguan_table, name);
		SLOG(4)("AddWangguan = %s", sql_line);

		if(-1 == sql_len)
		{
			return false;
		}
		sql_line[sql_len] = '\0';

		MYSQL_RES * results = BeginQuery(sql_line);
		EndQuery(results);
	}
	else								// 如果存在，说明网关重新上线，则更新记录
	{
		EndQuery(existresults);
		int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
			"UPDATE %s SET onoff=1 WHERE serial='%s'",
			m_wangguan_table, name);
		SLOG(4)("AddWangguan = %s", sql_line);

		if(-1 == sql_len)
		{
			return false;
		}
		sql_line[sql_len] = '\0';

		MYSQL_RES * results = BeginQuery(sql_line);
		EndQuery(results);
	}
	return TRUE;
}

/* CRelationMgr::Insert()和CRelationMgr::Remove()时调用，
   用于添加网关上下线记录。
*/
bool CClientInfo::AddOnoffRecord(const char * name, char * add, int onoff)
{
	char sql_line_exist[NORMAL_XG_BUF_LEN];
	int sql_len_exist = _snprintf_s(sql_line_exist, NORMAL_XG_BUF_LEN-1, _TRUNCATE,
		"SELECT COUNT(*) FROM %s WHERE serial='%s'", m_onoff_record_table, name);
	sql_line_exist[sql_len_exist] = '\0';
	SLOG(4)("AddOnoffRecord = %s", sql_line_exist);
	MYSQL_RES * existresults = BeginQuery(sql_line_exist);
	if (NULL == existresults)
	{
		return FALSE;
	}
	int recordnum = strtoul(*mysql_fetch_row(existresults), NULL, 0);
	EndQuery(existresults);
	if( recordnum == 120)		// 如果已有的该网关上下线记录等于120条
	{
		SLOG(4)("此处应当删除前21条记录");		
		DelOnoffRecord(name);
	}
	InsertOnoffRecord(name, add, onoff);

	return TRUE;
}

/* 当某个序列号的网关上下线记录达到120时，
   删除前21条记录。
*/
bool CClientInfo::DelOnoffRecord(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"DELETE FROM %s WHERE serial='%s' ORDER BY time LIMIT 21",
		m_onoff_record_table, name);
	SLOG(4)("DelOnoffRecord = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

bool CClientInfo::InsertOnoffRecord(const char * name, char * add, int onoff)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"INSERT INTO %s(serial,onoff,ip,time)\
		VALUES('%s', %d, '%s', now())",
		m_onoff_record_table, name, onoff, add);
	SLOG(4)("InsertOnoffRecord = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 当网关登录时，初始化客户端记录列表 */
bool CClientInfo::InitClientRecord(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	char sql_line_exist[NORMAL_XG_BUF_LEN];

	int sql_len_exist = _snprintf_s(sql_line_exist, NORMAL_XG_BUF_LEN-1, _TRUNCATE,
		"SELECT serial FROM %s WHERE serial='%s'", m_client_record_table, name);
	sql_line_exist[sql_len_exist] = '\0';
	MYSQL_RES * existresults = BeginQuery(sql_line_exist);	// 查询网关名称看是否已存在
	if (NULL == existresults)
	{
		return FALSE;
	}

	if(existresults->row_count == 0)
	{	
		EndQuery(existresults);

		// 如果不存在则初始化client_record表，各项均置为0
		int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
			"INSERT INTO %s(serial,android_phone,android_pad,pc,tmp,iOS)\
			VALUES('%s', 0, 0, 0, 0, 0)",
			m_client_record_table, name);
		if(-1 == sql_len)
		{
			return false;
		}

		SLOG(4)("InitClientRecord = %s", sql_line);

		sql_line[sql_len] = '\0';

		MYSQL_RES * results = BeginQuery(sql_line);
		EndQuery(results);
	}
	else
	{
		EndQuery(existresults);
	}
	return TRUE;
}

/* 当网关下线时，删除客户端列表中改网关的记录 */
bool CClientInfo::DelClientRecord(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"DELETE FROM %s WHERE serial='%s'",m_client_record_table, name);
	if(-1 == sql_len) 
	{
		return false;
	}
	SLOG(4)("DelClientRecord = %s", sql_line);

	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* HandleRMLogin()中调用，即当有客户端登录时，更新相应网关的客户端列表 */
bool CClientInfo::IncreClientRecord(const char * name, int type)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	int sql_len;
	switch(type)
	{
	case RM_PHONE:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET android_phone=android_phone+1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("IncreClientRecord = %s", sql_line);
			break;
		}
	case RM_PY:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET android_pad=android_pad+1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("IncreClientRecord = %s", sql_line);
			break;
		}
	case RM_PC:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET pc=pc+1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("IncreClientRecord = %s", sql_line);
			break;
		}
	case TMP:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET tmp=tmp+1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("IncreClientRecord = %s", sql_line);
			break;
		}
	case iOS:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET iOS=iOS+1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("IncreClientRecord = %s", sql_line);
			break;
		}
	default:
		break;
	}
	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* CClientContainer::Delete()中调用，即当删除客户端时，更新其对应网关的客户端记录 */
bool CClientInfo::DecreClientRecord(const char * name, int type)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	int sql_len;
	switch(type)
	{
	case RM_PHONE:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET android_phone=android_phone-1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("DecreClientRecord = %s", sql_line);
			break;
		}
	case RM_PY:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET android_pad=android_pad-1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("DecreClientRecord = %s", sql_line);
			break;
		}
	case RM_PC:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET pc=pc-1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("DecreClientRecord = %s", sql_line);
			break;
		}
	case TMP:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET tmp=tmp-1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("DecreClientRecord = %s", sql_line);
			break;
		}
	case iOS:
		{
			sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
				"UPDATE %s SET iOS=iOS-1 WHERE serial='%s'",
				m_client_record_table, name);
			SLOG(4)("DecreClientRecord = %s", sql_line);
			break;
		}
	default:
		break;
	}
	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 更新iHarbor连接参数 */
bool CClientInfo::UpdateiHarborConn(int size)
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"UPDATE %s SET value=%d WHERE param='iHarbor_conn'",
		m_conn_param_table, size);
	SLOG(4)("UpdateiHarborConn = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 更新Client连接参数 */
bool CClientInfo::UpdateClientConn(int size)
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"UPDATE %s SET value=%d WHERE param='client_conn'",
		m_conn_param_table, size);
	SLOG(4)("UpdateClientConn = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 每次转发服务器重新启动时，更新最大连接参数 */
bool CClientInfo::UpdateMaxConn(int size)
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"UPDATE %s SET value=%d WHERE param='max_conn'",
		m_conn_param_table, size);
	SLOG(4)("UpdateMaxConn = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 每次转发服务器重新启动时，添加转发服务器上线时间记录 */
bool CClientInfo::AddStartRecord()
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"INSERT INTO %s(serial,onoff,ip,time) VALUES(0, 1, '116.255.180.115', now())",
		m_onoff_record_table);
	SLOG(4)("AddStartRecord = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 转发服务器退出后，将所有网关置为下线 */
bool CClientInfo::ZeroWangguan()
{
	char sql_line[NORMAL_XG_BUF_LEN];

	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"UPDATE %s SET onoff=0", m_wangguan_table);
	SLOG(4)("ZeroWangguan = %s", sql_line);

	if(-1 == sql_len) 
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);

	EndQuery(results);
	return TRUE;
}

/* 当某个网关下线时，更新网关状态 */
bool CClientInfo::UpdateWangguan(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"UPDATE %s SET onoff=0 WHERE serial='%s'",
		m_wangguan_table, name);
	SLOG(4)("UpdateWangguan = %s", sql_line);

	if(-1 == sql_len)
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);
	EndQuery(results);
	return TRUE;
}

/* 转发服务器退出后，清空网关客户端记录 */
bool CClientInfo::ZeroClientRecord()
{
	char sql_line[NORMAL_XG_BUF_LEN];
	int sql_len = _snprintf_s(sql_line, NORMAL_XG_BUF_LEN - 1, _TRUNCATE, 
		"DELETE FROM %s", m_client_record_table);
	SLOG(4)("ZeroClientRecord = %s", sql_line);

	if(-1 == sql_len)
	{
		return false;
	}
	sql_line[sql_len] = '\0';

	MYSQL_RES * results = BeginQuery(sql_line);
	EndQuery(results);
	return TRUE;
}