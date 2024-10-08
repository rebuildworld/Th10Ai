#include "Base/Exception/Catcher.h"

#include <ostream>
#include <exception>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/locale.hpp>

#include "Base/Exception/Throw.h"
#include "Base/Windows/StackTrace.h"

namespace base
{
	namespace bloc = boost::locale;
	namespace blc = boost::locale::conv;
	namespace blu = boost::locale::util;

	void Catcher::toStream(std::ostream& out) const
	{
		try
		{
			throw;
		}
		catch (const Throwable& throwable)
		{
			out << throwable;
		}
		catch (const boost::exception& be)
		{
			std::string info = boost::diagnostic_information(be);
			std::string id = blu::get_system_locale();
			if (id.find("UTF-8") == std::string::npos)
			{
				bloc::generator gen;
				info = blc::to_utf<char>(info, gen(id));
			}
			out << info;
		}
		catch (const std::exception& se)
		{
			std::string id = blu::get_system_locale();
			if (id.find("UTF-8") == std::string::npos)
			{
				bloc::generator gen;
				std::string buf = blc::to_utf<char>(se.what(), gen(id));
				out << buf << '\n';
			}
			else
			{
				out << se.what() << '\n';
			}
		}

		win::StackTrace trace = win::StackTrace::FromCurrentException();
		out << "StackTrace:\n" << trace;
	}
}
