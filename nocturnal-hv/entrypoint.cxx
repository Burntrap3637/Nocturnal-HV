#pragma once
#include <includes.h>

bool entry_point( entry_t* entry ) {
	std::memcpy(
		&kernel::m_pdb, 
		&entry->m_pdb,
		sizeof( entry->m_pdb )
	);

	kernel::m_ntoskrnl_base = kernel::get_ntoskrnl_base( );
	if ( !kernel::m_ntoskrnl_base )
		return false;

	paging::m_directory_table_base = entry->m_directory_table_base;
	if ( !paging::m_directory_table_base )
		return false;

	kernel::dbg_print( "[nocturnal] image_base=0x%llx image_size=0x%x\n",
		entry->m_image_base, entry->m_image_size );

	//if ( !callback::create( ) ) {
	//	kernel::dbg_print( "[nocturnal] could not register power callback\n" );
	//	return false;
	//}

	if ( !guest::initialize( ) ) {
		kernel::dbg_print( "[nocturnal] could not initialize guest\n" );
		return false;
	}

	return false;
}