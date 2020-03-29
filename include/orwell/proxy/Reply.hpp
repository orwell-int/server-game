#pragma once

#include <string>

namespace orwell
{
namespace proxy
{

class Reply
{
public:
	enum class Status
	{
		kEmpty,
		kSuccess,
		kFailure,
	};

	Reply();

	operator Status () const;

	operator bool () const;

	operator std::string () const;

	Reply & operator = (std::string const& iReply);

	Reply & operator += (std::string const& iReply);

	void fail(std::string const& iReply);

	void fail();

private:
	Status m_status;
	std::string m_reply;
};

}
}
