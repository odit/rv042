/***************************************************************************
 *  settings.cc : This file is part of 'ataga'
 *  created on: Thu Jul  1 18:11:57 2004
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

#include "settings.h"

using namespace std;

const int CSettings::comparisonbits = 0xFE48A1FC;
const char CSettings::noData = 0;
const char CSettings::yesData = 1;

CSettings::CSettings(const char* _filename) : hFile(0), dataLoaded(0), nextAlloc(0)
{
	int size(strlen(_filename));
	filename = new char[size + 1];
	*(filename + size) = 0;
	strncpy(filename, _filename, size);

//	SetupCleanFile();
}

CSettings::~CSettings()
{
	ClearMemory();
	delete [] filename;

	Close();
}

void CSettings::DropData()
{
	ClearMemory();
	dataLoaded = 1;
	return;
}

void CSettings::LoadData()
{
	if (dataLoaded)
		ClearMemory();

	Open();
	// check if the file exist if not create a new settings file
	if (!hFile)
	{
		SetupCleanFile();
		dataLoaded = 1;
		return;
	}
	else
	{
		// check if the file is really a setting file
		int readbits;
		fseek(hFile, 0, SEEK_SET);
		fread(&readbits, sizeof(int), 1, hFile);
		if (memcmp(&comparisonbits, &readbits, sizeof(int)))
		{
		    SetupCleanFile();
			dataLoaded = 1;
			return;
		}
		assert(hFile);

		int optionNum;
		fread(&optionNum, sizeof(int), 1, hFile);
		
		for (int i(0); i < optionNum; ++i)
		{
			uint m_size;
			uint d_size;
			fread(&m_size, sizeof(int), 1, hFile);
			fread(&d_size, sizeof(int), 1, hFile);

			vector<char*> *n_alloc = AllocateMemory(d_size);

			n_alloc->reserve(m_size);
			
			for (uint p(0); p < m_size; ++p)
			{
				bool needMemory(0);
				fread(&needMemory, sizeof(bool), 1, hFile);
				if (needMemory)
				{
					char *obj = new char[d_size];
					fread(obj, d_size, 1, hFile);
					n_alloc->push_back(obj);
				}
				else
					n_alloc->push_back(0);
			}
		}
		
		uint size;

		fread(&size, sizeof(int), 1, hFile);
		for (uint i_p(0); i_p < size; ++i_p) {
			pData m_data;
			fread(&m_data.size, sizeof(uint), 1, hFile);
			char *m_temp = new char[m_data.size + 1];
			m_data.data = m_temp;
			*(m_temp + m_data.size) = 0;
			fread(m_data.data, m_data.size, 1, hFile);
			m_pData.push_back(m_data);
		}
	}
	
	Close();
	
	dataLoaded = 1;
}

void CSettings::ClearMemory()
{
	int size(m_data.size());
	for (int i(0); i < size; ++i) {
		vector<char*> *m_vector = m_data[i];
		int m_size(m_vector->size());
		for (int p(0); p < m_size; ++p) {
			char* m_val = (*m_vector)[p];
			delete [] m_val;
		}
		delete m_vector;
	}
	size = m_pData.size();
	for (int i(0); i < size; ++i)
		delete [] m_pData[i].data;
	
	m_pData.clear();
	m_data.clear();
	m_types.clear();
	dataLoaded = 0;
	nextAlloc = 0;
}

vector<char*>* CSettings::AllocateMemory(uint size)
{
	map<uint, uint>::iterator i = m_types.find(size);
	if (i == m_types.end()) {
		m_types.insert(map<uint, uint>::value_type(size, nextAlloc));
		vector<char*> *n_alloc = new vector<char*>;
		m_data.push_back(n_alloc);
		++nextAlloc;
		return n_alloc;
	}
	return ( m_data[i->second] );
}

void CSettings::SetupCleanFile()
{
	Close();
	hFile = fopen(filename, "w+b");
	if (hFile)
		fwrite(&comparisonbits, sizeof(int), 1, hFile);
	else
		perror("fopen");
	Close();
}

void CSettings::Sync()
{
	SetupCleanFile();
	Open();

	int size(m_data.size());
	fwrite(&comparisonbits, sizeof(int), 1, hFile);
	fwrite(&size, sizeof(int), 1, hFile);
	
	map<uint, uint>::iterator i = m_types.begin();

	for (int q(0); q < size; ++q) {
		int d_size(i->first);
		
		vector<char*> *m_vector = AllocateMemory(d_size);
		
		int m_size(m_vector->size());

		fwrite(&m_size, sizeof(int), 1, hFile);
		fwrite(&d_size, sizeof(uint), 1, hFile);

		for (int p(0); p < m_size; ++p) {
			void *tmp = (*m_vector)[p];
			if (!tmp)
				fwrite(&noData, sizeof(bool), 1, hFile);
			
			else {
				fwrite(&yesData, sizeof(bool), 1, hFile);
				fwrite(tmp, d_size, 1, hFile);
			}
		}		
		++i;
	}
	
	size = m_pData.size();
	fwrite(&size, sizeof(int), 1, hFile);
	for (int p(0); p < size; ++p) {
		pData m_data = m_pData[p];
		fwrite(&m_data.size, sizeof(int), 1, hFile);
		fwrite(m_data.data, m_pData[p].size, 1, hFile);
	}

	Close();
}

uint CSettings::SetPointerData(const void *data, uint size, int setting)
{
	if (!dataLoaded)
		LoadData();

	pData m_data(0);
	m_data.size = size;

	if (setting == -1) {
		setting = m_pData.size();
		m_pData.push_back(m_data);
	} else {
		while (m_pData.size() < (uint)(setting + 1))
			m_pData.push_back(m_data);
	}

	char *m_temp;
	pData m_Datatemp = m_pData[setting];
	
	if (m_Datatemp.data) {
		if (m_Datatemp.size != size) {
			delete [] m_Datatemp.data;
			m_temp = new char[size + 1];
		}
		else
			m_temp = (char*)m_Datatemp.data;
	}
	else
		m_temp = new char[size + 1];

	m_data.data = m_temp;
	*(m_temp + size) = 0;
	memcpy(m_temp, data, size);
	m_pData[setting] = m_data;
			
	Sync();
	return setting;
}


const void* CSettings::GetPointerData(uint setting, uint *sizeReturn)
{
	if (!dataLoaded)
		LoadData();
	
	while (m_pData.size() < setting + 1) {
		pData m_data(0, 0);
		m_pData.push_back(m_data);
	}
	if (sizeReturn)
		*sizeReturn = m_pData[setting].size;
	
	return ( m_pData[setting].data );
}

void CSettings::Open()
{
	if (!hFile)
		hFile = fopen(filename, "r+b");
	
	if (!hFile) {
		printf("Unable to open configuration file. Are you sure that %s exists and is readable?\nIf not run 'naptd-confmaker' to create it.\n", filename);
		exit(0);
	}
}

void CSettings::Close()
{
	if (hFile) {
		fclose(hFile);
		hFile = 0;
	}
}
