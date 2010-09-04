/* serveripc.h
   Copyright (C) 2002 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

serveripc.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

serveripc.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with serveripc.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _SERVERIPC_H
#define _SERVERIPC_H

#include "buffer.h"

#define HRC_SERVERIPC_SUCCESS					0x0000
#define HRC_SERVERIPC_HOST_RESOLUTION_FAILED	0x0001
#define HRC_SERVERIPC_UNABLE_TO_CREATE_SOCKET	0x0002
#define HRC_SERVERIPC_UNABLE_TO_CONNECT_TO_HOST 0x0003
#define HRC_SERVERIPC_SENDDATA_FAILED			0x0004
#define HRC_SERVERIPC_MEM_ALLOC_FAILED			0x0005
#define HRC_SERVERIPC_TIMEDOUT					0x0006
#define HRC_SERVERIPC_NO_DATA_RECIEVED			0x0007
#define HRC_SERVERIPC_NOT_READY					0x0008
#define HRC_SERVERIPC_CONNECTION_CLOSED			0x0009
#define HRC_SERVERIPC_TIMEOUT					0x000A
#define HRC_SERVERIPC_INVALID_PARAMS			0x000B

class cServerIPC
{
	public:
		cServerIPC();
		~cServerIPC();

		int Open(char *pServer, int port);
		void Close();
		void Reset();

		int Send(char *pContent, int size);
		int CheckForResponse(int delay = -1);
		char *GetResponse();
		int GetResponseSize();

	private:
		int m_Socketfd;
		int m_iTimeOut;
		cBuffer m_buffer;
};
#endif // _SERVERIPC_H

