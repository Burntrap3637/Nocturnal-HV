#pragma once

namespace memory {
	template <typename type = std::uint64_t>
	type alloc_page( std::uint64_t size ) {
		auto base_address = kernel::mm_allocate_independent_pages( size );
		if ( !base_address )
			return 0;

		memset( base_address, 0, size );
		return reinterpret_cast< type > ( base_address );
	}

	template <typename type = std::uint64_t>
	type alloc_contiguous( std::uint64_t size ) {
		auto base_address = kernel::mm_allocate_contiguous_node_memory( size );
		if ( !base_address )
			return 0;

		memset( base_address, 0, size );
		return reinterpret_cast< type > ( base_address );
	}

	template <typename type = std::uint64_t>
	void free_page( type addr, std::size_t size ) {
		kernel::mm_free_independent_pages( reinterpret_cast< std::uint64_t >( addr ), size );
	}

	template <typename type = std::uint64_t>
	void free_contiguous( type addr ) {
		kernel::mm_free_contiguous_memory( reinterpret_cast< void* >( addr ) );
	}
}