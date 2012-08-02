#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

/*
 **************************************************************************
 * This file contains the enumeration of message types that can be passed
 * over the network.
 *
 * Have a look at network.txt for more information.
 *
 * Remember to put the enumeration as the very first item in the packet!
 * (The length is automatically remembered.)
 *
 **************************************************************************
 */
enum
{
	// This message is actually a command!  Use the command class's 
	// readFromPacket/writeToPacket functions to process the packet.
	msg_command = 998,
	
	// The synchronized time is:
	//   int sequence
	msg_heartbeat,
	
	// I last saw the time sync at...
	//   short whoami
	//   int sequence
	msg_heartbeat_reply,
	
	// A new unit has been created.
		// int playerId  :  The player who will control the unit
		// int unitId    :  The unit id (drrrr)
		// string type
		// float x       :  x pos of the unit
		// float y       :  y pos of the unit
	msg_create_unit = 0, 

	// The server assigns a player id to the new client.
		// int playerId
	msg_assign_playerid, 

	// A new player connected, assign him a nodeId.  The nodeId is what
	// the server uses to identify him.  The player can then send packets
	// including his own nodeId, so that the server can send messages
	// specifically to the player.
		//int nodeId
	msg_assign_nodeid,

	// A unit has moved.
		// int unitId
		// short nextX
		// short nextY
		// short destAngle
	msg_pathfind, 
	
	// A unit's fuel has changed
		// int unitId
		// int fuelLeft
	msg_update_fuel,
	
	// It's time for all units that use fuel when idle to lose some of said fuel
	msg_drain_idle_fuel,

	// A unit fired a shot.
		// unitID shooter
		// int type
		// float targetX
		// float targetY
		// char airTarget
	msg_create_projectile, 

	// Someone said something in-game
		// string text
		// int playerId
	msg_game_chat, 

	// Someone said something in the lobby
		// string text
		// string playerName
	msg_lobby_chat, 

	// A unit's hit points changed
		// int unitId
		// int newHp
	msg_set_hps,
	
	// A unit was killed
		// int unitId
		// int explosionEffect
	msg_kill_unit,
	
	// A unit entered a transport
		// int unitId
		// int transportId
	msg_enter_transport,
	
	// A unit left a transport
		// int unitId
		// int transportId
		// int x
		// int y
	msg_leave_transport,
	
	// Update scrapyard status
		// int ID
		// int owner
		// double starttime
		// short queuePos
		// short queueSize
		// short rallyX, rallyY
		// string[] units
	msg_update_scrapyard,
	
	//global start signal
	msg_start_game,

	// A player joined the lobby
		// string playerName
	msg_join_lobby, 

	// A player tried to join the game, but the host rejected the request.
	// Sent from host to rejected client.
	msg_join_failed, 

	// A player joined the game, and needs to know about the other players':
	// 1) playerName
	// 2) nodeId
	// 3) socketHandle
	// so that everyone knows who the next host is if the host drops.
		// int numPlayers
		// [string playerName1, int nodeId1, int socketHandle1...]
	msg_other_player_data, 

	// A new player connected, but his someone in the game had the same
	// name as he does.  Force the new player's name to change locally for
	// the current game.  It will change back after the game is over.
		// string newName
	msg_change_name, 

	// A player gracefully disconnected.  (He didn't pull the plug or 
	// suddenly turn off his computer.)
		// int nodeId
		// string playerName
	msg_disconnect,

	//has map name
		//string mapName
	msg_map_name,
	
	// State of some unit (ai, facing, etc)
	//   int unitId
	//   int state
	//   char moving
	msg_unit_state,

	// Sorry, you're dead.
	msg_player_lost,
	
	// Congratulations, you won!
	msg_player_won,
};

#endif
