﻿
#include <iostream>

#include "Client.h"
#include "GUIClient.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

enum class GUIState
{
	Quit,
	Login,
	ViewInfo,
	Pokemons,
	Users,
	Rooms,
	Game,
	GameResult
};

int main (int argc, char *argv[])
{
	using namespace PokemonGame;
	using namespace PokemonGameGUI;

	Client client (IPADDR, PORT);

	bool isPassiveOffline = false;
	UserModel curUser;
	PokemonID pidToPlay;
	std::pair<size_t, size_t> worldSize;
	std::vector<RoomPlayer> roomPlayers;
	std::vector<GameModel::ResultPlayer> gameResults;

	auto guiState = GUIState::Login;
	while (guiState != GUIState::Quit)
	{
		switch (guiState)
		{
		case GUIState::Quit:
			// Do nothing just Quit
			break;

		case GUIState::Login:
			try
			{
				curUser = GUIClient::LoginWindow (
					client, isPassiveOffline);
				isPassiveOffline = false;
				guiState = GUIState::ViewInfo;
			}
			catch (const std::exception &)
			{
				client.Logout ();
				guiState = GUIState::Quit;
			}
			break;

		case GUIState::ViewInfo:
			pidToPlay = curUser.pokemons.front ().pid;
			guiState = GUIState::Rooms;
			break;

		case GUIState::Pokemons:
			break;

		case GUIState::Users:
			break;

		case GUIState::Rooms:
			try
			{
				std::tie (worldSize, roomPlayers) =
					GUIClient::RoomWindow (client, curUser, pidToPlay);
				guiState = GUIState::Game;
			}
			catch (const std::exception &ex)
			{
				if (std::string (ex.what ()) == BadSession)
				{
					guiState = GUIState::Login;
					isPassiveOffline = true;
				}
				else
				{
					guiState = GUIState::ViewInfo;
					try { client.RoomLeave (); }
					catch (...) {}
				}
			}
			break;

		case GUIState::Game:
			try
			{
				gameResults = GUIClient::GameWindow (
					client, curUser, roomPlayers,
					worldSize.first, worldSize.second);
				guiState = GUIState::GameResult;
			}
			catch (const std::exception &ex)
			{
				if (std::string (ex.what ()) == BadSession)
				{
					guiState = GUIState::Login;
					isPassiveOffline = true;
				}
				else
				{
					client.RoomLeave ();
					guiState = GUIState::ViewInfo;
				}
			}
			break;

		case GUIState::GameResult:
			client.RoomLeave ();
			guiState = GUIState::ViewInfo;
			break;

		default:
			throw std::runtime_error (
				"This will not happend unless your Compiler has a fatal BUG :-(");
			break;
		}
	}

	return 0;
}