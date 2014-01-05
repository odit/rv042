/***************************************************************************
 *  settings.h : This file is part of 'ataga'
 *  created on: Thu Jul  1 18:12:12 2004
 *
 *  (c) 2003,2004 Lukasz Tomicki <lucas.tomicki@gmail.com>
 *  
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//
// SettingsManager Class
// acts as a setting-resource manager, writing settings to a file 
// basic functions
// ----------------------------------------
// template < class tName >
//		uint Set(tName val, int setting = -1)
//      	used to set data
//
// template < class tName >
//		bool Get(tName& type, uint id)
//      	used to retrieve data
//
//
//		uint SetPointerData(const void *data, uint size, int setting = -1)
//      	used to set pointer data
// 
// 		const void* GetPointerData(uint setting, uint *sizeReturn = 0)
//      	used to retrieve pointer data
//
// 
// Feedback : lucas.tomicki@gmail.com 
//

#ifndef _SETTINGSMANAGER_H_
#define _SETTINGSMANAGER_H_

#include <stdio.h>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>

typedef unsigned int uint;

//#pragma warning( disable : 4786 )
//#pragma warning( disable : 4251 )

class CSettings
{
	public:
	// The contructor needs a filename to use for storing the program settings
	// its best to make sure that the filename is program-specific, if a file
	// with the same name exists and is not a valid settings file it's erased
	// and replaced with the settings managers file
		CSettings(const char*);
		 ~CSettings();
	
	// Can be called any time to free memory usage by the manager
		void ClearMemory();

	// Can be called any time to load the data, if not called by the programer, it will
	// will be called by the manager upon an attempt to read any data from the manager
	// can be called multiple times, data will load only once, for every ClearMemory call
		void LoadData();
	
	// Will internally call ClearMemory();
		void DropData();
	
	// This is called by the deconstructor to synchronize the settings in memory
	// with the HDD, can be called at any time to sync the memory this way the
	// settings will not be lost if the program dies goes wack
		void Sync();
	
	// returns the number of pointer data 
		inline uint GetSize() 
		{ 
			if (!dataLoaded)
				LoadData();
			
			return ( m_pData.size() );
		}
	
	// returns the number of data of the given size
		inline uint GetSize(uint d_type) 
		{ 
			if (!dataLoaded)
				LoadData();
			
			std::vector<char*> *n_alloc = AllocateMemory(d_type);
			return ( n_alloc->size() ); 
		}

	// setting - id for data IF setting == -1 then a free id will be allocated and returned
	// data - void pointer to memory block containning memory to save as setting
	// size - size of the data
	// returns setting if setting != -1 otherwise returns a unique resource id
		uint SetPointerData(const void *data, uint size, int setting = -1);
	
	// Sets the resource specified by setting to val
	// setting - id for data IF setting == -1 then a free id will be allocated and returned
	// returns setting if setting != -1 otherwise returns a unique resource id
		template < class tName >
		uint Set(tName val, int setting = -1)
		{			if (!dataLoaded)
				LoadData();

			uint d_type(sizeof(val));

			std::vector<char*> *n_alloc = AllocateMemory(d_type);

			if (setting == -1) {
				setting = n_alloc->size();
				n_alloc->push_back(0);
			} else {
				while (n_alloc->size() < (uint)(setting + 1))
					n_alloc->push_back(0);
			}
			
			void* m_ptr = (*n_alloc)[setting];
			tName *obj;

			if (!m_ptr)
				obj = (tName*) new char[d_type];
			
			else
				obj = (tName*) m_ptr;
			
			memcpy(obj, &val, d_type);
			(*n_alloc)[setting] = (char*)obj;
			
			Sync();
			return setting;
		};

	// setting - unique id of string resource
	// sizeReturn - if not 0 will contain the size of the requested string
	// returns a void* to the data specified by a setting
	// if setting wasn't set before using SetPointerData the function returns 0
		const void* GetPointerData(uint setting, uint *sizeReturn = 0);
	
	// returns data based on the unique id, and the sizeof (type)
	// example : rInfo = Get(UNIQUE_ID, rInfo);
		template < class tName >
		bool Get(tName& type, uint id)
		{
			if (!dataLoaded)
				LoadData();

			uint d_type(sizeof(type));

			std::vector<char*> *n_alloc = AllocateMemory(d_type);
			bool r_set(true);

			while (n_alloc->size() < id + 1) {
				r_set = false;
				n_alloc->push_back(0);
			}
			
			tName *returnValue = (tName*)(*n_alloc)[id];

			if (!returnValue) {
				returnValue = (tName*) new char[d_type];
				memset(returnValue, 0, d_type);
				(*n_alloc)[id] = (char*)returnValue;
			}
			memcpy(&type, returnValue, d_type);
			return ( r_set );
		}
	
	private:
	// Opens a file specified by filename for reading and writing
		void Open();
	
	// Closes the file handle
		void Close();

	// Used to recreate the settings file
		void SetupCleanFile();

		std::vector<char*>* AllocateMemory(uint size);
	
		static const int comparisonbits;
		static const char noData;
		static const char yesData;

		FILE* hFile;
		char* filename;
		bool dataLoaded;
		uint nextAlloc;

		std::vector<std::vector<char*>*> m_data;
		std::map<uint, uint> m_types;

		typedef struct tag_pData
		{
			tag_pData() { };
			tag_pData(char *intData) : data(intData) { };
			tag_pData(char *intData, uint i_Size) : data(intData), size(i_Size) { };
			char* data;
			uint size;
		} pData;
		
		std::vector<pData> m_pData;
};

enum {
	listening_port = 0
};

#endif	//_SETTINGSMANAGER_H_
