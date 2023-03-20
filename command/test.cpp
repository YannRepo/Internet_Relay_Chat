# include "test.hpp"
# include "../src/20_class/client.hpp"
#include <sys/types.h>
#include <sys/socket.h>

//source https://modern.ircdocs.horse/#client-messages

void	Client::sendMessage(std::string str)
{
	send(this->socketFd, str.c_str(), str.size(), 0);
}

bool	Client::cmdPASS(std::vector<std::string> &cmd)
{
	int pos = 0;
	int tmp_pos = 0;

	if (cmd[1].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("PASS"));
		return (false);
	}
	// Check avec espace
	if ((pos = this->server.get_password().find(" ")) == std::string::npos)
	{
		if (cmd[1] != this->server.get_password())
		{
			sendMessage(ERR_PASSWDMISMATCH);
			return (false);
		}
	}
	else
	{
		for (int i = 1; i < cmd.size(); i++)
		{
			if (cmd[i] != this->server.get_password().substr(tmp_pos, pos))
			{
				sendMessage(ERR_PASSWDMISMATCH);
				return (false);
			}
			tmp_pos = pos + 1;
		}
	}
	this->isConnected = true;
	sendMessage("Connection established \r\n");
	return (true);
}

bool	Client::cmdNICK(std::vector<std::string> &cmd)
{
	if (cmd[1].empty())
	{
		sendMessage(ERR_NONICKNAMEGIVEN);
		return (false);
	}
	if (cmd.size > 2)
	{
		sendMessage(ERR_ERRONEUSNICKNAME) 
		return (false);
	}
	for (int i = 0; i < srv.clientList.size(); i++)
	{
		if (cmd[1] == srv.clientList[i].ClientNick)
		{
			sendMessage(ERR_NICKNAMEINUSE);
			return (false);
		}
	}
	sendMessage(NICK(this->User.nickname, cmd[1]));
	this->userInfos.nickname = cmd[1];
	//The NICK message may be sent from the server to clients to acknowledge their NICK command was successful, and to inform other clients about the change of nickname. In these cases, the <source> of the message will be the old nickname [ [ "!" user ] "@" host ] of the user who is changing their nickname.
	return (true);
}

bool	Client::cmdUSER(std::vector<std::string> &cmd)
{
	// Deja enregistrer
	if (cmd[1].empty() || cmd[2].empty() || cmd[3].empty() || cmd[4].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("USER"));
		return (false);
	}
	if (cmd[2] != "0" || cmd[3] != "*")
	{
		sendMessage("USER :Parameters incorrect");
		return (false);
	}

	//If a client tries to send the USER command after they have already completed registration with the server, the ERR_ALREADYREGISTERED reply should be sent and the attempt should fail.

	//If the client sends a USER command after the server has successfully received a username using the Ident Protocol, the <username> parameter from this command should be ignored in favour of the one received from the identity server.

	this->userInfos.username = cmd[4];
	this->userInfos.realname = cmd[4];
	//Clients SHOULD use the nickname as a fallback value for <username> and <realname> when they don’t have a meaningful value to use.
	return (true);
}

bool	Client::cmdPING(std::vector<std::string> &cmd)
{
	if (cmd[1].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("PING"));
		return (false);
	}
	sendMessage("PONG" + cmd[1] + "\r");
	return (true);
}

bool	Client::cmdOPER(std::vector<std::string> &cmd)
{
	if (cmd[1].empty() || cmd[2].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("OPER"));
		return (false);
	}
	if (cmd[2] != this->server.get_password())
	{
		sendMessage(ERR_PASSWDMISMATCH);
		return (false);
	}
	sendMessage(RPL_YOUREOPER(cmd[1]);
	//The user will also receive a MODE message indicating their new user modes, and other messages may be sent.
	return (true);
}

bool	Client::cmdQUIT(std::vector<std::string> &cmd)
{
	// This is typically only dispatched to clients that share a channel with the exiting user.
	// Envoye Quit :<reason> au autres clients
	std::cout << ":QUIT : Bye for now" << std::endl;
	return (true);
}

// INFO POUR LES CHANNELS 
// Servers MUST process the parameters of this command as lists on incoming commands from clients, with the first <key> being used for the first <channel>, the second <key> being used for the second <channel>, etc.
// While a client is joined to a channel, they receive all relevant information about that channel including the JOIN, PART, KICK, and MODE messages affecting the channel. They receive all PRIVMSG and NOTICE messages sent to the channel, and they also receive QUIT messages from other clients joined to the same channel (to let them know those users have left the channel and the network). This allows them to keep track of other channel members and channel modes.

bool	Client::cmdJOIN(std::vector<std::string> &cmd)
{
	if (cmd[1].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("JOIN"));
		return (false);
	}
	// Les erreurs possible
	// ERR_TOOMANYCHANNELS
	// ERR_BANNEDFROMCHAN : Returned to indicate that a JOIN command failed because the client has been banned from the channel and has not had a ban exception set for them. The text used in the last param of this message may vary.
	// ERR_INVITEONLYCHAN : Returned to indicate that a JOIN command failed because the channel is set to [invite-only] mode and the client has not been invited to the channel or had an invite exception set for them. The text used in the last param of this message may vary.
	if (cmd[1][0] != '#')
	{
		sendMessage(ERR_BADCHANMASK(cmd[1]));
		return (false);
	}
	//Note that this command also accepts the special argument of ("0", 0x30) instead of any of the usual parameters, which requests that the sending client leave all channels they are currently connected to. The server will process this command as though the client had sent a PART command for each channel they are a member of.

	// This message may be sent from a server to a client to notify the client that someone has joined a channel. In this case, the message <source> will be the client who is joining, and <channel> will be the channel which that client has joined
	// Servers SHOULD NOT send multiple channels in this message to clients, and SHOULD distribute these multiple-channel JOIN messages as a series of messages with a single channel name on each.

	// Des qu'il a rejoint on doit envoyer une message "<source> joined the channel" doit etre envoye ainsi que le topic du channel (avec RPL_TOPIC) et la liste des utilisateurs connectes (avc RPL_NAMREPLY suivi d'un RPL_ENDOFNAMES)
	return (true);
}

// Supprime un client ou des channels
bool	Client::cmdPART(std::vector<std::string> &cmd)
{
	if (cmd[1].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("PART"));
		return (false);
	}
	// ERR_NOSUCHCHANNEL si le channel n'exidte pas 
	// ERR_NOTONCHANNEL si le channel existe mais le client n'est pas dedans
	
	// the user will receive a PART message from the server for each channel they have been removed from. <reason> is the reason that the client has left the channel(s).
	// Servers SHOULD NOT send multiple channels in this message to clients, and SHOULD distribute these multiple-channel JOIN messages as a series of messages with a single channel name on each.
	return (true);
}

// The PRIVMSG command is used to send private messages between users, as well as to send messages to channels. <target> is the nickname of a client or the name of a channel.
bool	Client::cmdPRIVMSG(std::vector<std::string> &cmd)
{
	if (cmd[2].empty())
	{
		std::cout << ERR_NEEDMOREPARAMS("PRIVMSG");
		return (false);
	}
	//If <target> is a channel name and the client is banned and not covered by a ban exception, the message will not be delivered and the command will silently fail.
	// Si le message ne peut pas etre envoye ERR_CANNOTSENDTOCHAN
	// If <target> is a channel name, it may be prefixed with one or more channel membership prefix character (@, +, etc) and the message will be delivered only to the members of that channel with the given or higher status in the channel. Servers that support this feature will list the prefixes which this is supported for in the STATUSMSG RPL_ISUPPORT parameter, and this SHOULD NOT be attempted by clients unless the prefix has been advertised in this token.
	// If <target> is a user and that user has been set as away, the server may reply with an RPL_AWAY (301) numeric and the command will continue.

	// When the PRIVMSG message is sent from a server to a client and <target> starts with a dollar character ('$', 0x24), the message is a broadcast sent to all clients on one or multiple servers.
	return (true);
}

bool	Client::cmdNOTICE(std::vector<std::string> &cmd)
{
	//The difference between NOTICE and PRIVMSG is that automatic replies must never be sent in response to a NOTICE message.This rule also applies to servers – they must not send any error back to the client on receipt of a NOTICE command.
	// SI j'ai bien compris un NOTICE est un PRIVMSG sans message d'erreur
	if (cmd[2].empty())
		return (false);
	// Envoyer le message
}

////////////////////////////////////////////////////////////////////////////
bool	Client::cmdMODE(std::vector<std::string> &cmd)
{
	if (cmd[1].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("MODE"));
		return (false);
	}
}

// Parameters: <nickname> <channel>
bool	Client::cmdINVITE(std::vector<std::string> &cmd)
{
	if (cmd[2].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("INVITE"));
		return (false);
	}
	// Si le channel n'existe pas => ERR_NOSUCHCHANNEL
	// Si le client n'est pas dans le channel dont il a envoye l'invitation => ERR_NOTONCHANNEL
	// Si le client est deja dans le channel => ERR_USERONCHANNEL
	
	//When the invite is successful, the server MUST send a RPL_INVITING numeric to the command issuer, and an INVITE message, with the issuer as <source>, to the target user. Other channel members SHOULD NOT be notified.
}

//  Parameters: <channel> <user> [<comment>]
bool	Client::cmdKICK(std::vector<std::string> &cmd)
{
	if (cmd[2].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("KICK"));
		return (false);
	}
	// supprime le user du channel
}

////////////////////////////////////////////////////////////////////////////
bool	Client::cmdWHOIS(std::vector<std::string> &cmd)
{
	if (cmd[1].empty())
	{
		sendMessage(ERR_NEEDMOREPARAMS("WHOIS"));
		return (false);
	}
}
//- PASS, NICK, USER, PING, OPER, QUIT, JOIN, PART, PRIVMSG, NOTICE, MODE, INVITE. KICK, WHOIS
bool	Client::launchCommand(std::string command)
{
	std::vector<std::string> ccmd = split(command, ':');
	std::vector<std::string> vecmd = split(ccmd[0], ' ');
	std::string	choice[13] = {"NICK", "USER", "PING", "OPER", "QUIT", "JOIN", "PART", "PRIVMSG", "NOTICE", "MODE", "INVITE", "KICK", "WHOIS"};
	bool	(*f[13])(std::vector<std::string> &) = {&cmdNICK, &cmdUSER, &cmdPING, &cmdOPER, &cmdQUIT, &cmdJOIN, &cmdPART, &cmdPRIVMSG, &cmdNOTICE, &cmdMODE, &cmdINVITE, &cmdKICK, &cmdWHOIS};
	int i = 0;
	
	if (ccmd[1])
		vecmd.push_back(ccmd[1]);
	if (vecmd.empty())
		return (false);
	while (i < 13)
	{
		if (vecmd[0] == choice[i])
			return (*f[i])(vecmd);
		i++;
	}
	sendMessage("Command not found");
	return (false);
}
