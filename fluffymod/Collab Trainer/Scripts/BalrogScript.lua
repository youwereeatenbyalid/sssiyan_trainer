PlayerManipulator = require "PlayerManipulator"

--function to set punch kick state
--set_weaponBalrog(app.PlayerDante.WeaponS_Balrog)
--get_weaponBalrog()
--get_cachedMotion()
	--getLayer(System.UInt32)
		--get_MotionID()
		--get_Frame()		
	--
		
dante = nil

currentid = 0
currentbankid = 0
currentframe = 0.0
--0=punch
--1=kick
currentbalrogstate = 0


balrog_doAttackS_method= sdk.find_type_definition("app.WeaponBalrog"):get_method("doAttackS")

balrog_doAttackP_method= sdk.find_type_definition("app.WeaponBalrog"):get_method("doAttackP")


local function pre_balrog_doAttackS(args)
	if not dante then return end
	currentbalrogstate = dante:call("get_weaponBalrog")
	if dante:call("isGround") == false then
		dante:call("set_weaponBalrog",1) -- set weapon to kick for divekick
	else
		if currentid == 0x172 and currentframe < 20.0 then --check welter threshhold recovery
					dante:call("set_weaponBalrog",0) --set weapon to punch for recovery
		else
			if (currentid == 0x190 or currentid == 0x19A) and currentframe < 42.0 then
				dante:call("set_weaponBalrog",0) --set weapon to punch for hooks
			else
				dante:call("set_weaponBalrog",1) --else set weapon to kick otherwise
			end
		end
	end
end

local function post_balrog_doAttackS(retval)
	if not dante then return retval end
	dante:call("set_weaponBalrog",currentbalrogstate)
	return retval
end
sdk.hook(balrog_doAttackS_method,pre_balrog_doAttackS,post_balrog_doAttackS)

local function pre_balrog_doAttackP(args)
	if not dante then return end
	currentbalrogstate = dante:call("get_weaponBalrog")
	if dante:call("isGround") == false then
		if dante:call("get_commandType") == 512 then
			dante:call("set_weaponBalrog",0) -- set weapon to punch for volcano punch
		else
			dante:call("set_weaponBalrog",1) -- set weapon to kick for divekick
		end
	else
		if currentid == 0x172 and currentframe < 20.0 then --check welter threshhold recovery
					dante:call("set_weaponBalrog",0) --set weapon to punch for recovery
		else
			if (currentid == 0x190 or currentid == 0x19A) and currentframe < 42.0 then
				dante:call("set_weaponBalrog",0) --set weapon to punch for hooks
			else
				dante:call("set_weaponBalrog",1) --else set weapon to kick otherwise
			end
		end
	end
end


local function post_balrog_doAttackP(retval)
	if not dante then return retval end
	dante:call("set_weaponBalrog",currentbalrogstate)
	return retval
end

sdk.hook(balrog_doAttackP_method,pre_balrog_doAttackP,post_balrog_doAttackP)


balrogtrack_doUpdate_method = sdk.find_type_definition("app.player.dante.BalrogTrack"):get_method("updateInitialize") -- for set quick welter in tracker

balroguniquetrack = nil

local function pre_balrogtrack_doUpdate(args)
	balroguniquetrack = sdk.to_managed_object(args[2])
	
end


local function post_balrogtrack_doUpdate(retval)
	if not balroguniquetrack then return retval end
	
	if currentframe > 12.0 and currentid == 100 then --first kick early welter
		balroguniquetrack:set_field("KeepPress",1)
	end
	if currentframe > 15.0 and currentid == 100 then --first kick early welter
		balroguniquetrack:set_field("KeepPress",2)
	end
	return retval
end

sdk.hook(balrogtrack_doUpdate_method,pre_balrogtrack_doUpdate,post_balrogtrack_doUpdate)


player_checkNormalJump_method = sdk.find_type_definition("app.Player"):get_method("checkNormalJump")

local function pre_player_checkNormalJump(args)

	
	
end

local function post_player_checkNormalJump(retval)
	if not dante then return retval end
		--balrog motion bank
		if currentbankid == 0xD4 then
			--kick combo A-2
			if currentid == 0x6E then
				if currentframe > 17.0 and currentframe < 37.0 then
					return true
				end
			end
			--kick combo A-3
			if currentid == 0x78 then
				if currentframe > 22.0 and currentframe < 46.0 then
					return true
				end
			end
			
			--flint wheel
			if currentid == 0x1770 then
				if currentframe > 15.0 and  currentframe < 30.0 then
					return true
				end
			end
			--pyromania
			if currentid == 0x1B58 then
				if currentframe > 140.0 and  currentframe < 155.0 then
					return true
				end
				if currentframe > 160.0 and  currentframe < 185.0 then
					return true
				end
				if currentframe > 230.0 and  currentframe < 275.0 then
					return true
				end
			end
		end
	return retval
end

sdk.hook(player_checkNormalJump_method,pre_player_checkNormalJump,post_player_checkNormalJump)


playerDante_checkCommand_method = sdk.find_type_definition("app.PlayerDante"):get_method("checkCommand")

local function pre_playerDante_checkCommand(args)
	if not dante then return end --make sure we have dante
	if dante:call("get_currentWeaponS") ~= 3 then return end -- balrog check 
	local commandtype = dante:call("get_commandType") --type of command action we're doing.
	if commandtype == 1 then	--melee forward
		dante:call("set_weaponBalrog",1)
		if dante:call("isGround") == false then
		
			dante:call("setCurrentNodeID",3345136356,0,true)
			--dante:call("set_commandType",2)
			return sdk.PreHookResult.SKIP_ORIGINAL 
		end
	end
	
	if commandtype == 2 then	--melee backward
		dante:call("set_currentStyle",1)
		dante:call("set_weaponBalrog",1)
		dante:call("set_commandType",0x400)
	end
	
	if commandtype == 0x10 then	--melee shuffle
		if dante:call("isGround") then -- do dash forward on shuffle
			dante:call("set_weaponBalrog",0)
			dante:call("set_commandType",1)
		else
			dante:call("set_weaponBalrog",1)
		end
	end
	if dante:call("get_nextStyle") == 1 then
		if commandtype == 512 then
			dante:call("set_weaponBalrog",0) -- give us dragon punch
		end
		
		if commandtype == 1024 then
			if dante:call("isGround") then
				dante:call("set_weaponBalrog",0) -- give us real impact
			else
				dante:call("set_weaponBalrog",1) -- give us the rave
			end
		end
		
		if commandtype == 8192 then
			dante:call("set_weaponBalrog",1) -- give us the rave
			dante:call("set_commandType",0x200)
		end
	end
	
	
	
end

local function post_playerDante_checkCommand(retval)
	return retval
end

sdk.hook(playerDante_checkCommand_method,pre_playerDante_checkCommand,post_playerDante_checkCommand)


re.on_draw_ui(function()
	imgui.text(currentid)
	imgui.text(currentframe)
end)



re.on_pre_application_entry("UpdateBehavior", function()

	dante = PlayerManipulator.get_playerByID(1)
	if not dante then return end
	--log.info(dante:call("getCurrentNodeID(System.UInt32)",0))
	local motion = dante:call("get_cachedMotion")
	if not motion then return end
	--log.info("got motion")
	local layer = motion:call("getLayer(System.UInt32)",0)
	if not layer then return end
	--log.info("got layer")
	currentid = layer:call("get_MotionID")
	currentbankid = layer:call("get_MotionBankID")
	currentframe = layer:call("get_Frame")
	dante:call("set_weaponBalrog",0) -- give us dragon punch
end)