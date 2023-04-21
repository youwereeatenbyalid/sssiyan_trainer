

local PlayerManipulator = {}

PlayerManipulator.enumerator = require "enumerator"

PlayerManipulator.enumerator.app = {
	PlayerID = {}
}

PlayerManipulator.enumerator.app.PlayerID = PlayerManipulator.enumerator.generate_enum("app.PlayerID")

function PlayerManipulator.get_playerManager()
	local playman = sdk.get_managed_singleton(sdk.game_namespace("PlayerManager"))
	return playman
end

function PlayerManipulator.get_manualPlayer()
	local playman = PlayerManipulator.get_playerManager()
	if not playman then return nil end
	return playman:call("get_manualPlayer")
end


function PlayerManipulator.isGameplay()
	local player = PlayerManipulator.get_manualPlayer()
	if not player then return false end
	local playerobject = player:call("get_GameObject")
	if not playerobject then return false end
	return playerobject:call("get_Update")
end

function PlayerManipulator.get_playerByID(id)
    local playman = PlayerManipulator.get_playerManager()
    if not playman then return nil end
    return playman:call("getPlayerByID",id)
end

function PlayerManipulator.get_player(charactername)
    return PlayerManipulator.get_playerByID(PlayerManipulator.enumerator.app.PlayerID[charactername])
end

function PlayerManipulator.is_player(charactername)
	local manualplayer = PlayerManipulator.get_manualPlayer()
	local nameplayer = PlayerManipulator.get_player(charactername)
	if not manualplayer then return false end
	return nameplayer == manualplayer
end

PlayerManipulator.players = {}

function PlayerManipulator.request_player_list()
	local playman = sdk.get_managed_singleton(sdk.game_namespace("PlayerManager"))
	if not playman then 
		--log.info("no playman")
		return false
	end
	local playerlist = playman:get_field("players")
	if playerlist == nil then 
		--log.info("no player list")
		return false 
	end
	local playerarray = playerlist:get_field("mItems"):get_elements()
	local temptable = {}
	local tempvalue = 1
	--log.info("oh god oh fuck")
	for i,v in ipairs(playerarray) do
		if v:call("get_playerID") == PlayerManipulator.enumerator.app.PlayerID["Gilver"] then
			--log.info("is v")
			if v:call("get_isSummon") then
				--log.info("is summon")
			else
				temptable[tempvalue] = v
				tempvalue = tempvalue + 1
			end
		else
			--log.info("not v")
			temptable[tempvalue] = v
			tempvalue = tempvalue + 1
		end
	end
	PlayerManipulator.players = temptable
	if PlayerManipulator.players == nil then 
		--log.info("no player array")
		return false 
	end
	return true
end


return PlayerManipulator