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

/* CRelationMgr::Insert()ʱ���ã��������س���������Ӽ�¼����������ʱ���¼�¼ */
bool CClientInfo::AddWangguan(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	char sql_line_exist[NORMAL_XG_BUF_LEN];

	int sql_len_exist = _snprintf_s(sql_line_exist, NORMAL_XG_BUF_LEN-1, _TRUNCATE,
		"SELECT serial FROM %s WHERE serial='%s'", m_wangguan_table, name);
	sql_line_exist[sql_len_exist] = '\0';
	MYSQL_RES * existresults = BeginQuery(sql_line_exist);	// ��ѯ�������ƿ��Ƿ��Ѵ���
	if (NULL == existresults)
	{
		return FALSE;
	}

	if(existresults->row_count == 0)	// ��������ڣ������һ���������߼�¼
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
	else								// ������ڣ�˵�������������ߣ�����¼�¼
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

/* CRelationMgr::Insert()��CRelationMgr::Remove()ʱ���ã�
   ����������������߼�¼��
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
	if( recordnum == 120)		// ������еĸ����������߼�¼����120��
	{
		SLOG(4)("�˴�Ӧ��ɾ��ǰ21����¼");		
		DelOnoffRecord(name);
	}
	InsertOnoffRecord(name, add, onoff);

	return TRUE;
}

/* ��ĳ�����кŵ����������߼�¼�ﵽ120ʱ��
   ɾ��ǰ21����¼��
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

/* �����ص�¼ʱ����ʼ���ͻ��˼�¼�б� */
bool CClientInfo::InitClientRecord(const char * name)
{
	char sql_line[NORMAL_XG_BUF_LEN];
	char sql_line_exist[NORMAL_XG_BUF_LEN];

	int sql_len_exist = _snprintf_s(sql_line_exist, NORMAL_XG_BUF_LEN-1, _TRUNCATE,
		"SELECT serial FROM %s WHERE serial='%s'", m_client_record_table, name);
	sql_line_exist[sql_len_exist] = '\0';
	MYSQL_RES * existresults = BeginQuery(sql_line_exist);	// ��ѯ�������ƿ��Ƿ��Ѵ���
	if (NULL == existresults)
	{
		return FALSE;
	}

	if(existresults->row_count == 0)
	{	
		EndQuery(existresults);

		// ������������ʼ��client_record���������Ϊ0
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

/* ����������ʱ��ɾ���ͻ����б��и����صļ�¼ */
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

/* HandleRMLogin()�е��ã������пͻ��˵�¼ʱ��������Ӧ���صĿͻ����б� */
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

/* CClientContainer::Delete()�е��ã�����ɾ���ͻ���ʱ���������Ӧ���صĿͻ��˼�¼ */
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

/* ����iHarbor���Ӳ��� */
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

/* ����Client���Ӳ��� */
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

/* ÿ��ת����������������ʱ������������Ӳ��� */
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

/* ÿ��ת����������������ʱ�����ת������������ʱ���¼ */
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

/* ת���������˳��󣬽�����������Ϊ���� */
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

/* ��ĳ����������ʱ����������״̬ */
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

/* ת���������˳���������ؿͻ��˼�¼ */
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