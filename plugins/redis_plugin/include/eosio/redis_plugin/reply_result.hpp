#include <iostream>
#include <string>
#include <list>
#include <vector>

enum ReplyType
{
	REPLY_NONE,
	REPLY_STRING,
	REPLY_INTEGER,
	REPLY_BOOLEN,
	REPLY_LIST,
	REPLY_HASH,
	REPLY_ERROR
};

typedef signed long long	SInt64;
namespace eosio {

class CReplyResult
{
public:
	CReplyResult()
	{
		m_strresult = "";
		m_intresult = 0;
		m_boolresult = false;
		m_replyType = REPLY_NONE;
		m_errresult = "";
	}
	~CReplyResult(){}

	std::string							GetReplyString();
	SInt64  							GetReplyInteger();
	std::list<std::string>				GetReplyList();
	vector<std::string>					GetReplyVector();
	chain::flat_map<std::string, std::string>  GetReplyHash();
	bool								GetReplyBool();
	std::string							GetReplyError();
	ReplyType							GetReplyType();

private:
	friend class redis_plugin;
	void								SetReplyString(std::string strreply);
	void								SetReplyInteger(SInt64 intreply);
	void								SetReplyList(std::list<std::string> lstreply);
	void								SetReplyVector(vector<std::string> lstreply);
	void								SetReplyHash(chain::flat_map<std::string,std::string> mapreply);
	void								SetReplyBool(bool boolreply);
	void								SetReplyError(std::string err);
	void								SetReplyType(ReplyType reptype);
	void								Clear();

private:
	std::string							m_strresult;
	SInt64  							m_intresult;
	std::list<std::string>			    m_lstresult;
	vector<std::string>					m_vstresult;
	chain::flat_map<std::string, std::string>  m_mapresult;
	bool								m_boolresult;
	std::string							m_errresult;
	ReplyType							m_replyType;
	
};


inline std::string CReplyResult::GetReplyString()
{
	return m_strresult;
}

inline long long CReplyResult::GetReplyInteger()
{
	return m_intresult;
}

inline std::list<std::string> CReplyResult::GetReplyList()
{
	return m_lstresult;
}

inline vector<std::string> CReplyResult::GetReplyVector()
{
	return m_vstresult;
}

inline chain::flat_map<std::string, std::string> CReplyResult::GetReplyHash()
{
	return m_mapresult;
}

inline bool CReplyResult::GetReplyBool()
{
	return m_boolresult;
}

inline std::string CReplyResult::GetReplyError()
{
	return m_errresult;
}

inline ReplyType CReplyResult::GetReplyType()
{
	return m_replyType;
}

inline void CReplyResult::SetReplyString(std::string strreply)
{
	m_strresult = strreply;
}

inline void CReplyResult::SetReplyInteger(SInt64 intreply)
{
	m_intresult = intreply;
}

inline void CReplyResult::SetReplyVector(vector<std::string> vstreply)
{
	m_vstresult = vstreply;
}

inline void CReplyResult::SetReplyList(std::list<std::string> lstreply)
{
	m_lstresult = lstreply;
}

inline void CReplyResult::SetReplyHash(chain::flat_map<std::string,std::string> mapreply)
{
	m_mapresult = mapreply;
}


inline void CReplyResult::SetReplyBool(bool boolreply)
{
	m_boolresult = boolreply;
}

inline void CReplyResult::SetReplyError(std::string err)
{
	m_errresult = err;
}

inline void CReplyResult::SetReplyType(ReplyType reptype)
{
	m_replyType = reptype;
}

inline void CReplyResult::Clear()
{
	m_strresult = "";
	m_intresult = 0;
	m_lstresult.clear();
	m_mapresult.clear();
	m_boolresult = false;
	m_errresult = "";
	m_replyType = REPLY_NONE;
}

}// namespace eosio