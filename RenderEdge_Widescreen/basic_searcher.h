#pragma once

#include "pe_reader.h"

namespace base { namespace warcraft3 {

	class basic_searcher
	{
	public:
		basic_searcher(HMODULE hModule);
		uintptr_t search_string_ptr(const char* str, size_t length) const;

	private:
		void initialize();

	private:
		win::pe_reader module_;
		uintptr_t rdata_beg_;
		uintptr_t rdata_end_;
		uintptr_t data_beg_;
		uintptr_t data_end_;
	};
}}
