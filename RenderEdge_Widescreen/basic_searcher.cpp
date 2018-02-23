#include "basic_searcher.h"
#include "memory_search.h"

namespace base { namespace warcraft3 {

	basic_searcher::basic_searcher(HMODULE hModule)
		: module_(hModule)
	{
		initialize();
	}

	void basic_searcher::initialize()
	{
		PIMAGE_SECTION_HEADER rdata_ptr = module_.get_section_by_name(".rdata");
		PIMAGE_SECTION_HEADER data_ptr  = module_.get_section_by_name(".data");

		rdata_beg_ = module_.rva_to_addr(rdata_ptr->VirtualAddress);
		rdata_end_ = rdata_beg_ + rdata_ptr->SizeOfRawData;
		data_beg_  = module_.rva_to_addr(data_ptr->VirtualAddress);
		data_end_  = data_beg_ + data_ptr->SizeOfRawData;
	}

	uintptr_t basic_searcher::search_string_ptr(const char* str, size_t length) const
	{
		uintptr_t retval;

		retval = detail::search_str(rdata_beg_, rdata_end_, str, length);
		if (retval)
			return retval;

		retval = detail::search_str(data_beg_, data_end_, str, length);
		if (retval)
			return retval;

		retval = detail::search_str_no_zero(rdata_beg_, rdata_end_, str, length);
		if (retval)
			return retval;

		retval = detail::search_str_no_zero(data_beg_, data_end_, str, length);
		if (retval)
			return retval;

		return 0;
	}
}}
