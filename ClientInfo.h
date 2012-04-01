#pragma once

#include <mysql.h>
#include "DBConnection.h"


class CClientInfo : public CDBConnection
{
private:
	static char * m_wangguan_table;
	static char * m_onoff_record_table;
	static char * m_client_record_table;
	static char * m_conn_param_table;

public:
	CClientInfo(void);
	~CClientInfo(void);
	bool AddWangguan(const char * name);
	bool AddOnoffRecord(const char * name, char * add, int onoff);
	bool DelOnoffRecord(const char * name);
	bool InsertOnoffRecord(const char * name, char * add, int onoff);
	bool InitClientRecord(const char * name);
	bool DelClientRecord(const char * name);
	bool IncreClientRecord(const char * name, int type);
	bool DecreClientRecord(const char * name, int type);
	bool UpdateiHarborConn(int size);
	bool UpdateClientConn(int size);
	bool UpdateMaxConn(int size);
	bool AddStartRecord();
	bool ZeroWangguan();
	bool UpdateWangguan(const char * name);
	bool ZeroClientRecord();
};