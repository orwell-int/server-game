/* This class stores most of the useful data of the server. */

#pragma once

//com
#include <RawMessage.hpp>

#include <GlobalContext.hpp>


namespace orwell {
namespace tasks {

class processDecider
{
	public:
		static void Process( com::RawMessage const & iMessage,
                             GlobalContext & ioCtx);

	private:
        processDecider();
};

}} //end namespace

