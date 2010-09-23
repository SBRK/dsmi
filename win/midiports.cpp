// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "midiports.h"

#include <windows.h>
#include <mmsystem.h>

std::vector<std::string> getOutputPorts()
{
	std::vector< std:: string > res;
	
	MIDIOUTCAPS caps;
	
	int n_devices = midiOutGetNumDevs();
	
	for(int i=0; i < n_devices; ++i)
	{
		midiOutGetDevCaps(i, &caps, sizeof(MIDIOUTCAPS));
		WCHAR *wname = caps.szPname;
		
		char name[256];
		int j;
		for(j=0; j<255 && wname[j]!=0; ++j) {
			name[j] = (char)wname[j];
		}
		name[j] = 0;
		
		res.push_back(std::string(name));
	}
	
	return res;
}

std::vector<std::string> getInputPorts()
{
	std::vector< std:: string > res;
	
	MIDIINCAPS caps;
	
	int n_devices = midiInGetNumDevs();
	
	for(int i=0; i < n_devices; ++i)
	{
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
		WCHAR *wname = caps.szPname;
		
		char name[256];
		int j;
		for(j=0; j<255 && wname[j]!=0; ++j) {
			name[j] = (char)wname[j];
		}
		name[j] = 0;
		
		res.push_back(std::string(name));
	}
	
	return res;
}
