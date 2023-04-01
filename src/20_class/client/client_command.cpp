# include "client.hpp"
# include <sys/types.h>
# include <sys/socket.h>
# include <vector>
# include <iostream>
# include <fstream>
# include <string>
# include "../../10_tools/errors.hpp"


//source https://modern.ircdocs.horse/#client-messages

/////////////////////////////////////////////////////////
void	Client::sendMessage(std::string str)
{
	sendCustom(this->socketFd, str.c_str(), str.size(), 0);
}

void	Client::sendOtherClient(std::string str)
{
	for (std::vector<Client>::iterator it = this->server.clientList.begin(); it != this->server.clientList.end(); it++)
	{
		if (it->socketFd != this->socketFd)
			sendCustom(it->socketFd, str.c_str(), str.size(), 0);
	}
}

bool	Client::cmdPING(std::vector<std::string> &cmd)
{
	if (cmd.size() < 2)
	{
		sendMessage(ERR_NEEDMOREPARAMS("PING"));
		return (false);
	}
	//std::cout << "PONG " + cmd[1] + "\r" << std::endl;
	sendMessage("PONG " + cmd[1] + "\r");
	return (true);
}

bool	Client::cmdOPER(std::vector<std::string> &cmd)
{
	if (cmd.size() < 3)
	{
		sendMessage(ERR_NEEDMOREPARAMS("OPER"));
		return (false);
	}
	if (cmd[2] != this->server.get_password())
	{
		sendMessage(ERR_PASSWDMISMATCH);
		return (false);
	}
	sendMessage(RPL_YOUREOPER(cmd[1]));
	//The user will also receive a MODE message indicating their new user modes, and other messages may be sent.
	return (true);
}

bool	Client::cmdQUIT(std::vector<std::string> &cmd)
{
	if (cmd.size() == 1)
	{
		sendMessage(QUIT(this->userInfos.nickName, this->userInfos.userName, this->userInfos.hostName));
		sendOtherClient(QUIT(this->userInfos.nickName, this->userInfos.userName, this->userInfos.hostName));
	}
	else
	{
		sendMessage(QUIT_REASON(this->userInfos.nickName, this->userInfos.userName, this->userInfos.hostName, cmd[1]));	
		sendOtherClient(QUIT_REASON(this->userInfos.nickName, this->userInfos.userName, this->userInfos.hostName, cmd[1]));	
	}
	return (true);
}


// Parameters: <nickname> <channel>
bool	Client::cmdINVITE(std::vector<std::string> &cmd)
{
	if (cmd.size() < 3)
	{
		sendMessage(ERR_NEEDMOREPARAMS("INVITE"));
		return (false);
	}
	// Si le channel n'existe pas => ERR_NOSUCHCHANNEL
	// Si le client n'est pas dans le channel dont il a envoye l'invitation => ERR_NOTONCHANNEL
	// Si le client est deja dans le channel => ERR_USERONCHANNEL
	
	//When the invite is successful, the server MUST send a RPL_INVITING numeric to the command issuer, and an INVITE message, with the issuer as <source>, to the target user. Other channel members SHOULD NOT be notified.
	return (true);
}

//  Parameters: <channel> <user> [<comment>]
bool	Client::cmdKICK(std::vector<std::string> &cmd)
{
	if (cmd.size() < 3)
	{
		sendMessage(ERR_NEEDMOREPARAMS("KICK"));
		return (false);
	}
	// supprime le user du channel
	return (true);
}

////////////////////////////////////////////////////////////////////////////
bool	Client::cmdWHOIS(std::vector<std::string> &cmd)
{
	if (cmd.size() < 2)
	{
		sendMessage(ERR_NEEDMOREPARAMS("WHOIS"));
		return (false);
	}
	return (true);
}

bool	Client::cmdCAP(std::vector<std::string> &cmd)
{
	if (cmd.size() == 2)
	{
		//std::cout << RED << cmd[1] << std::endl;
		if (cmd[1].compare("LS") == 0)
		{
			this->status = COMING;
			return (true);
		}
	}
	return (false);
}

//- PASS, NICK, USER, PING, OPER, QUIT, JOIN, PART, PRIVMSG, NOTICE, MODE, INVITE. KICK, WHOIS
bool	Client::launchCommand(std::string command)
{
	if (command.empty())
		return (true);
	
	std::vector<std::string> ccmd = split(command, ':');
	std::vector<std::string> vecmd = split(ccmd[0], ' ');
	std::string	choice[16] = {"CAP", "PASS", "NICK", "USER", "PING", "PONG", "OPER", "QUIT", "JOIN", "PART", "PRIVMSG", "NOTICE", "MODE", "INVITE", "KICK", "WHOIS"};
	bool	(Client::*f[16])(std::vector<std::string> &) = {&Client::cmdCAP, &Client::cmdPASS, &Client::cmdNICK, &Client::cmdUSER, &Client::cmdPING, &Client::cmdPONG, &Client::cmdOPER, &Client::cmdQUIT, &Client::cmdJOIN, &Client::cmdPART, &Client::cmdPRIVMSG, &Client::cmdNOTICE, &Client::cmdMODE, &Client::cmdINVITE, &Client::cmdKICK, &Client::cmdWHOIS};
	int i = 0;

	if (ccmd.size() != 1 && ccmd.size() != 2)
	{
		sendMessage("Arguments incorrect : Too many ':'");
	}
	if (ccmd.size() == 2)
		vecmd.push_back(ccmd[1]);


	if (vecmd.empty())
	{
		return (false);
	}

	//------- Pré authentification pour Yann et Arzu (TEMPORAIRE) ----// JULIA::commentaire
	//	if (this->status == WAITING)
	//	this->status = REGISTERED;
	//	this->hasNick == true;
	////------------------------------------------------------------ //
	while (i < 14)
	{
		if (vecmd[0] == choice[i])
		{
			return (this->*f[i])(vecmd);
		}
		i++;
	}
	// sendMessage("Command not found"); //ARZU: commentaire
	return (false);
}
