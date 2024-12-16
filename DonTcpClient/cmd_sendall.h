/* Copyright (C) XiongZG.
 * This file is part of DonSocket
 *
 * DonSocket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * DonSocket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DonSocket.If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMAND_SENDALL_H
#define COMMAND_SENDALL_H

#include <string>

#include "../CommandEngine/CommandObject.h"

class CCommand_SendAll : public CCommandObject
{
public:
	CCommand_SendAll(char* szName, void* pHost, std::vector<std::string>& cmds, int nThreadCount, int nMillisecInterval) :
		CCommandObject(szName, pHost, cmds, nThreadCount, nMillisecInterval)
	{

	}
public:
	virtual bool OnCmdStart(std::vector<std::string>& params);
	virtual bool OnCmdStarted();
	virtual bool OnCmdRun();
	virtual bool OnCmdStop();
	virtual bool OnCmdStopped();
};


#endif // COMMAND_SENDALL_H

