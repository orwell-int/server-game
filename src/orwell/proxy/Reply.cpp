#include "orwell/proxy/Reply.hpp"

namespace orwell
{
namespace proxy
{

Reply::Reply()
	: m_status(Status::kEmpty)
{
}

Reply::operator Status () const
{
	return m_status;
}

Reply::operator bool () const
{
	return (Status::kSuccess == m_status);
}

Reply::operator std::string () const
{
	return m_reply;
}

Reply & Reply::operator = (std::string const& iReply)
{
	if (Status::kEmpty == m_status)
	{
		m_status = Status::kSuccess;
	}
	m_reply = iReply;
	return *this;
}

Reply & Reply::operator += (std::string const& iReply)
{
	if (Status::kEmpty == m_status)
	{
		m_status = Status::kSuccess;
	}
	m_reply += iReply;
	return *this;
}

void Reply::fail(std::string const& iReply)
{
	m_status = Status::kFailure;
	m_reply = iReply;
}

void Reply::fail()
{
	m_status = Status::kFailure;
}

}
}
