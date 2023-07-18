--Test Mega Comment
--What does the breaker switcher need to do?
--Press dpad button, breaker changes. 
	--spawn new breaker on nero without crashing game. 

--swapping off tomboy when active: play RF end
--swapping onto tomboy when active: play RF end

-- rf on ground 1433606689
-- rf on air 1697104642
-- rf off air 190507351
-- rf off ground 4062100378


--tomboy sound effects:
-- 1095710707 = start id
-- 3392817104 = stop id

--voice calls:
--punchline: 00000000A2982798
--

PlayerManipulator = require "PlayerManipulator"

nero = nil




dobreakeradd = false

custombreaker = 0
keyboard_breaker = -1


breakerlist = {}
breakerlist[1] = 0
breakerlist[2] = 1
breakerlist[3] = 2
breakerlist[4] = 3
breakerlist[5] = 4
breakerlist[6] = 5
breakerlist[7] = 6
breakerlist[8] = 7
breakerlist[9] = 0
breakerlist[10] = 0
breakerlist[11] = 0
breakerlist[12] = 0

alwaysbreakaway = false
dotomboypop = false

autotomboypop = false

disable_diagonal = false

doaddb = false

-- Controls whether nero's bringer input does breaker attacks
DoubleBreaker = {}
DoubleBreaker.enabled = false
DoubleBreaker.main_breakerID = 0
DoubleBreaker.alt_breakerID = 0
DoubleBreaker.requesting_alt_breaker_attack = false
--We want to set this boolean to true and return false in the post hook when DoubleBreaker.enabled is true, so we surpress the buster attack and can trigger a breaker attack.

--Delay by a certain amount of updates to prevent the attack from canceling? IDK.
DoubleBreaker.update_delay = 0
DoubleBreaker.buster_trigger_state = {IDLE=0,TRIGGER=1,REQUESTING=2,WAITING=3,COMPLETE=4}
DoubleBreaker.buster_trigger = DoubleBreaker.buster_trigger_state.IDLE

--gets the most recent gauntlet id attack
local last_used_gauntlet_id = -1
breaker_to_breaker_cancel = false


function setNeroAction(move)
	if not nero then else
		nero:call("setAction(System.String, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)",
		move,0,3.0,0.0,0,1,false,false,false,0)
	end
end



function doRFEnd()
	if not nero then else
		if nero:call("isGround") then
			setNeroAction("RF_End")
		else
			setNeroAction("RF_EndAir")
		end
	end
end

function doRFNode()
	if not nero then else
		if nero:call("get_oldGauntletID") == 7 and nero:call("get_isBoostMode") == false then
			if nero:call("isGround") then
				nero:call("setCurrentNodeID",1433606689,0,true)
			else
				nero:call("setCurrentNodeID",1697104642,0,true)
			end
			nero:get_field("<gauntlet>k__BackingField"):call("set__isBoostMode(System.Boolean)",true)
		else
			if nero:call("isGround") then
				nero:call("setCurrentNodeID",4062100378,0,true)
			else
				nero:call("setCurrentNodeID",190507351,0,true)
			end
			--this shouldn't be necessary most of the time, but still
			if nero:call("get_oldGauntletID") == 7 then
				nero:get_field("<gauntlet>k__BackingField"):call("set__isBoostMode(System.Boolean)",false)
			end
			
		end
	end
end

--Call to turn on boost mode 
function RFOn()
	if not nero then else
		if nero:call("get_oldGauntletID") == 7 then
			if nero:call("isGround") then
				nero:call("setCurrentNodeID",1433606689,0,true)
			else
				nero:call("setCurrentNodeID",1697104642,0,true)
			end
			nero:get_field("<gauntlet>k__BackingField"):call("set__isBoostMode(System.Boolean)",true)
		end
	end
end

--Call to turn off boost mode
function RFOff()
	if nero:call("get_oldGauntletID") == 7 then
		nero:call("get_cachedRedQueen"):call("triggerSE(System.UInt32)",3392817104)
		if nero:call("isGround") then
			nero:call("setCurrentNodeID",4062100378,0,true)
		else
			nero:call("setCurrentNodeID",190507351,0,true)
		end
		nero:get_field("<gauntlet>k__BackingField"):call("set__isBoostMode(System.Boolean)",false)
	end
end


function SetNeroBreakerInList(GauntletID,index)
	if not nero then else
		local array = nero:call("get_gauntletList()")
		local index1 = array[index]
		local index1_alt = array:get_element(index)
		
		log.debug(tostring(array[index]:ToString()))
		log.debug(tostring(array[index].value__))
		
		local new_element = sdk.create_instance("app.GauntletID")
		log.debug(tostring(new_element.value__))

		new_element.value__ = GauntletID

		log.debug(tostring(new_element.value__))

		array[index] = new_element
	end
end

function BreakerUIUpdate()
	nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x20) --breaker pickup ui update

	nero:call("triggerSE(System.UInt32)",0x7F7009BB) --breaker swap sound
	
	--logic to make nero read using the gauntletID he will have, rather than the one he currently has.
	local gauntlet = nero:get_gauntlet()
	if not gauntlet then return end -- whyyyy
	local store_temp_gauntletID = gauntlet:get_field("<gauntletID>k__BackingField")
	local list_val = tonumber(nero:call("get_currentGauntletID"))
	--log.debug("temp gauntlet ID store " ..store_temp_gauntletID)
	--local test_val = tonumber(list_val)
	--log.debug("current gauntlet id " .. test_val)
	gauntlet:set_field("<gauntletID>k__BackingField",list_val)
	nero:call("get_cachedWwiseFacialCtrl"):call("trigger(System.UInt32)",0xA2982798) --nero voiceline
	gauntlet:set_field("<gauntletID>k__BackingField",store_temp_gauntletID)
end


function SwapBreaker(breakerid)
		if not nero then 
			log.debug("Can't find nero when swapping breaker!")
		else 
			nero:call("exchangeGauntlet(app.GauntletID)",breakerid) --exchange the gauntlet
			nero:call("get_neroStatusTmp"):call("clear") --cancel the normal ui update
			nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x20) --breaker pickup ui update
			nero:call("triggerSE(System.UInt32)",0x7F7009BB) --breaker swap sound
			nero:call("get_cachedWwiseFacialCtrl"):call("trigger(System.UInt32)",0xA2982798) --nero voiceline
			
		end
end

function SwapBreakerSilent(breakerid)
	if not nero then
			log.debug("Can't find nero when swapping breaker!")
		else 
			nero:call("exchangeGauntlet(app.GauntletID)",breakerid) --exchange the gauntlet
			nero:call("get_neroStatusTmp"):call("clear") --cancel the normal ui update
			
		end
end

function CycleBreakers()
	if not nero then else
		nero:call("SetToEndGauntlet(System.Int32)",0)
		--nero:call("exchangeGauntlet(app.GauntletID)",nero:call("get_currentGauntletID()")) --exchange the gauntlet
		nero:call("get_neroStatusTmp"):call("clear") --cancel the normal ui update
		--nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x20) --breaker pickup ui update
		nero:call("triggerSE(System.UInt32)",0x7F7009BB) --breaker swap sound
		nero:call("get_cachedWwiseFacialCtrl"):call("trigger(System.UInt32)",0xA2982798) --nero voiceline
		
		nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x00800)
	end
end


nerotype = sdk.find_type_definition("app.PlayerNero")

nero_setupBringerKnuckle_method = nerotype:get_method("setupBringerKnuckle(System.Boolean)")
nero_checkSpecial0_method = nerotype:get_method("checkSpecial0")

local function pre_nero_setupBringerKnuckle(args)
	if DoubleBreaker.enabled then

		args[3] = sdk.to_ptr(true);
	end
end 

local function post_nero_setupBringerKnuckle(retval)
	return retval
end


local function pre_nero_checkSpecial0(args)
	if DoubleBreaker.enabled then
		
	end
end 

local function post_nero_checkSpecial0(retval)
	return retval
end

--sdk.hook(nero_setupBringerKnuckle_method,pre_nero_setupBringerKnuckle,post_nero_setupBringerKnuckle)
--sdk.hook(nero_checkSpecial0_method,pre_nero_checkSpecial0,post_nero_checkSpecial0)

--controls infinite breaker settings. Overrides the void infinite breakers if enabled, and uses a "breakers don't move" breaker mode instead. 
nero_removeGauntlet = {}
nero_removeGauntlet.method = sdk.find_type_definition("app.PlayerNero"):get_method("removeGauntlet(System.Boolean)")
nero_removeGauntlet.noConsumption = true
nero_removeGauntlet.isInfiniteBraker = false
nero_removeGauntlet.gauntletExchange = false

function nero_removeGauntlet.pre(args)
	log.debug("this nero comparison works")

	--we always backup the state
	local trainman = sdk.get_managed_singleton(sdk.game_namespace("TrainingManager"))	
	if not trainman then else
		--store infinite breakers setting
		nero_removeGauntlet.isInfiniteBraker = trainman.isInfiniteBraker
		log.debug("Infinite breakers is " .. tostring(nero_removeGauntlet.isInfiniteBraker))
	end
	
	--if noConsumption, set no Consumption to true & disable traning manager temporarily. 
	if nero_removeGauntlet.gauntletExchange then
		if not trainman then else
			trainman:call("setTrainingInfiniteBraker",false)
		end
	else
		if nero_removeGauntlet.noConsumption then
			args[3] = sdk.to_ptr(true)
			if not trainman then else
				trainman:call("setTrainingInfiniteBraker",false)
			end
		end
	end
end

function nero_removeGauntlet.post(retval)
	local trainman = sdk.get_managed_singleton(sdk.game_namespace("TrainingManager"))
	if not trainman then else
		--restore infinite breakers setting
		trainman:call("setTrainingInfiniteBraker",nero_removeGauntlet.isInfiniteBraker)
	end
	return retval
end
sdk.hook(nero_removeGauntlet.method,nero_removeGauntlet.pre,nero_removeGauntlet.post)



----SAFETY HOOK TO TEMPORARILY TURN OFF INFINITE BREAKERS WHEN DOING AN EXCHANGE. THIS PREVENTS A CRASH WHEN EXCHANGING BREAKERS AT MAX BREAKER COUNT WHEN INFINITE BREAKERS IS ON IN THE TRAINING ROOM.
--also trying new hook approach to sandbox hooks appropriately
local nero_exchange_breaker = {}


nero_exchange_breaker.method = sdk.find_type_definition("app.PlayerNero"):get_method("exchangeGauntlet(app.GauntletID)")


function nero_exchange_breaker.pre(args)
	nero_removeGauntlet.gauntletExchange = true
end

function nero_exchange_breaker.post(retval)
	nero_removeGauntlet.gauntletExchange = false
	return retval
end

sdk.hook(nero_exchange_breaker.method,nero_exchange_breaker.pre,nero_exchange_breaker.post)


--gets the most recent gauntlet id attack
--used for checking valid breaker-to-breaker-cancel
gauntlet_set_action_method = sdk.find_type_definition("app.Gauntlet"):get_method("setGauntletAction")
local function pre_gauntlet_set_action_method(args)
	--If a gauntlet action is triggered, the attack has been requested successfully
	if DoubleBreaker.buster_trigger == DoubleBreaker.buster_trigger_state.WAITING then
		DoubleBreaker.buster_trigger = DoubleBreaker.buster_trigger_state.COMPLETE
	end
	local gauntlet_object = sdk.to_managed_object(args[2])
	last_used_gauntlet_id = gauntlet_object:call("get_gauntletID()")
end

local function post_gauntlet_set_action_method(retval)
	return retval
end

sdk.hook(gauntlet_set_action_method,pre_gauntlet_set_action_method,post_gauntlet_set_action_method)

--sets the breaker cancel flag when canceling a breaker attack into a breaker attack from a different breaker

nero_update_breaker_method = sdk.find_type_definition("app.PlayerNero"):get_method("updateSpecial0")
local function pre_nero_update_breaker_method(args)
	local nero_object = sdk.to_managed_object(args[2])
	--if the current gauntlet id has changed since the last attack, permission for faster cancel.
	--trying a direct comparison just to see what happens instead of last_used_gauntlet_id
	--that didn't work so we got with of get_currentGauntletID call (which checked the list) and now we check to see the gauntlet equipped is different than the last one used.
	if last_used_gauntlet_id ~= nero_object:get_currentGauntletID() and breaker_to_breaker_cancel then
		local nero_track = nero_object:call("get_playerNeroTrack")
		if nero_track:get_field("CommandReserve") then
			nero_track:set_field("GauntletReserve", 1)
		end
		if nero_track:get_field("CommandStart") then
			nero_track:set_field("GauntletStart",1)
		end
	end
end

local function post_nero_update_breaker_method(retval)
	return retval
end


sdk.hook(nero_update_breaker_method,pre_nero_update_breaker_method,post_nero_update_breaker_method)


--Hook Nero's buster method
nero_buster_displayclass = sdk.find_type_definition("app.PlayerNero.<>c__DisplayClass1130_0")
nero_buster_displayclass_method = nero_buster_displayclass:get_method("<updateSpecial1>b__0")



local function pre_nero_buster_displayclass(args)

end


local function post_nero_buster_displayclass(retval)
	--store return value as boolean
	local return_bool = (sdk.to_int64(retval) & 1) == 1
	--check if doing buster action and DoubleBreaker.enabled on
	if DoubleBreaker.enabled and return_bool then
		--if so surpress buster attack, prepare breaker attack.
		
		--We might need to do something here where we check breaker flags to make sure breaker attacks don't overlap. 
		--might not need to, bringer flags check this automatically?
		--DoubleBreaker.buster_trigger = DoubleBreaker.buster_trigger_state.TRIGGER
		--SwapBreaker(DoubleBreaker.alt_breakerID)
		return sdk.to_ptr(false)
	end
	return retval
end

--sdk.hook(nero_buster_displayclass_method,pre_nero_buster_displayclass,post_nero_buster_displayclass)



--Hooking Nero's breaker method
nero_breaker_method = sdk.find_type_definition("app.PlayerNero.<>c__DisplayClass1128_0"):get_method("<updateSpecial0>b__0")
local bringer_press = false
local new_triggered_flag = false
local function pre_nero_breaker_method(args)
	--
	--
	bringer_press = false
	local displayclass = sdk.to_managed_object(args[2])
	if not displayclass then return end
	local neroobject = displayclass:get_field("<>4__this");
	if not neroobject then return end
	local padinput = neroobject:get_field("<padInput>k__BackingField");
	if not padinput then return end
	--new_triggered_flag = false
	local GauntletCharge = neroobject:get_field("GauntletCharge")
	if DoubleBreaker.enabled then
		bringer_press = padinput:call("isButtonTrigger(System.UInt32)",0x2000)
	end
	
end
local function post_nero_breaker_method(retval)
	--reset this requesting value to be safe
	--DoubleBreaker.requesting_alt_breaker_attack = false
	--get return boolean
	local return_bool = (sdk.to_int64(retval) & 1) == 1
	--if true return bringer attack
	if return_bool then
		--if this is true it means a normal breaker attack was requested.
		new_triggered_flag = false
		log.debug("Normal Breaker Attack Requested")
	else
		if bringer_press then
			new_triggered_flag = true
			bringer_press = false
			return sdk.to_ptr(true)
		end
	end
	--If breaker attack requested from buster, reset flag and trigger attack.
	
	--I wonder if there's a way to do this just by setting a buffered breaker flag somewhere so we use the game's logic instead.
	
	--if DoubleBreaker.enabled and DoubleBreaker.buster_trigger == DoubleBreaker.buster_trigger_state.TRIGGER then
	--	DoubleBreaker.buster_trigger = DoubleBreaker.buster_trigger_state.REQUESTING
	--	return sdk.to_ptr(true)
	--end
	return retval
end

--sdk.hook(nero_breaker_method,pre_nero_breaker_method,post_nero_breaker_method)


local doing_breaker_swap = false

nero_activebreaker_method = sdk.find_type_definition("app.PlayerNero.<>c__DisplayClass1128_0"):get_method("<updateSpecial0>b__1(app.UserDataHolderB, app.Player.CancelState)")


local function pre_nero_activebreaker(args)
	--log.debug("Hello Yes pre_nero_activebreaker was called")
	local flag_type = sdk.to_int64(args[4])
	local displayclass = sdk.to_managed_object(args[2])
	if not displayclass then return end
	local neroobject = displayclass:get_field("<>4__this");
	if not neroobject then return end
	local gauntlet = neroobject:get_field("<gauntlet>k__BackingField")
	if not gauntlet then return end
	--log.debug("Hello we got past all the null checks")
	
	
	if doing_breaker_swap == true  then
		log.debug("Doing Breaker Swap")
		local is_started = gauntlet:call("get_IsStarted()")
		if is_started == true then
			local gauntlet_nero = gauntlet:call("get_cachedNero()") 
			if not gauntlet_nero then return end
			log.debug("Is Started is true, setting flag to triggered")
			doing_breaker_swap = false
			args[4] = sdk.to_ptr(2)
		end
	end
	
	if new_triggered_flag == true then
		log.debug("New Triggered Flag is True")
		if flag_type == 2 or flag_type == 3 then
			log.debug("Call Swap Breaker")
			--this dumb approach makes sure that exchange breaker doesn't overwrite the breaker list so we remember what gauntlet we're supposed to have equipped.
			local backup_breaker = nero:get_currentGauntletID()
			SwapBreaker(0)
			SetNeroBreakerInList(backup_breaker,0)
			
			new_triggered_flag = false
			doing_breaker_swap = true
			args[4] = sdk.to_ptr(0)
		end
	end
	local reserve_bit = neroobject:get_cancelReserveBit()
	local bitwise_and_cancel = reserve_bit & 0x8000 
	if bitwise_and_cancel == 0x8000 then
		log.debug(bitwise_and_cancel)
		log.debug("Cancel Reserve Bit is breaker")
	else
		if flag_type == 0  then 
			new_triggered_flag = false
		end
	end

	
end

local function post_nero_activebreaker(retval)
	return retval
end



--sdk.hook(nero_activebreaker_method,pre_nero_activebreaker,post_nero_activebreaker)



--app.PlayerNero.onChargeComplete() Responsible for Gauntlet Charge Complete Effect
--app.PlayerNero.onChargeStart() Responsible for Gauntlet Charging Effect


function NeroSmartBreakerSwap(breakerid)
	if not nero then 
			log.debug("Can't find nero when swapping breaker!")
		else 
		
		if nero:call("get_gauntletNum")  == 8 then
		--there's a bug we should address with swapping at 8 breakers, seems to cause a crash. 
		--Crash not showing up but UI breaking? Strange. 
		end
		

		--don't need to call push breaker, exchange auto fixes it. However, do need to check to see if we need to do previous gauntlet checks.	
		
		if nero:call("get_gauntletNum")  == 0 then
			SetNeroBreakerInList(breakerid,0)
		else
		

			--tomboy toggle logic
			--if nero:call("get_oldGauntletID") == 7 then
			--	if nero:call("get_isBoostMode") then
			--		RFOff()
			--	else
			--		if breakerid == 7 then
			--			dotomboypop = true
			--		end
			--	end
			--else
			--	if autotomboypop then 
			--		dotomboypop = true
			--	end
			--end
			
			--less aggressive tomboy logic
			if nero:call("get_oldGauntletID") == 7 and breakerid == 7 then
					if nero:call("get_isBoostMode") then
						RFOff()
					else
						dotomboypop = true
					end
			else 
				if breakerid == 7 and autotomboypop then
						dotomboypop = true
				end
			end
			--punchline
			if breakerid == 4 and nero:call("get_oldGauntletID") == 4 then
				nero:call("reqJetGadgetExplode")
				SetNeroBreakerInList(breakerid,0)
			end
			
			--catch logic for tomboy scenario
			if not (breakerid == 7 and nero:call("get_oldGauntletID") == 7) then
				SetNeroBreakerInList(breakerid,0)
			end
		end
	end
end

--Ensures punchline is recalled if a punchline is out instead of a new punchline being launched.

rocketpunch_doAttack_method = sdk.find_type_definition("app.RocketPunchGauntlet"):get_method("doAttack")

local function pre_rocketPunch_doAttack(args)
	if not nero then return end
	
	--punchline is out end
	if nero:get_field("<rocketPunchList>k__BackingField"):get_field("mSize") > 0 then
		nero:call("setRocketPunchReturn")
		return sdk.PreHookResult.SKIP_ORIGINAL 
	end
	
end

local function post_rocketPunch_doAttack(retval)
	return retval
end

sdk.hook(rocketpunch_doAttack_method, pre_rocketPunch_doAttack, post_rocketPunch_doAttack)

--stupid hook to get the weapon type

local weapon_type = 0

local nero_updateLockOn = {}

nero_updateLockOn.method = sdk.find_type_definition("app.PlayerNero"):get_method("updateLockOn")
function nero_updateLockOn.pre(args)
	if not nero then return end
	weapon_type = nero:get_currentWeaponType()
end

function nero_updateLockOn.post(retval)
	return retval
end
sdk.hook(nero_updateLockOn.method,nero_updateLockOn.pre,nero_updateLockOn.post)
player_doCharacterUpdateCommon_method = sdk.find_type_definition("app.Player"):get_method("doCharacterUpdateCommon")


local state_table = {}
state_table.NO_GAUNTLET = 0
state_table.GAUNTLET_NOT_STARTED = 1
state_table.GAUNTLET_IN_USE = 2
state_table.GAUNTLET_IDLE = 3
state_table.BRINGER_SWAP = 4
state_table.GAUNTLET_MISMATCH = 5
local last_state = 0

local switcher_mode_table = {}

switcher_mode_table.SWITCHER = 0
switcher_mode_table.CYLCLER = 1
local switcher_mode = switcher_mode_table.SWITCHER




local function create_pad_frame()
	local frame = {}
	frame.up = false
	frame.down = false
	frame.left = false
	frame.right = false
	return frame
end




local function initialize_pad_storage(size)
	local storage_table = {}
	for i = 1,size do 
		storage_table[i] = create_pad_frame()
	end
	return storage_table
end

local function iterate_pad(storage,padinput)
	local count = #storage
	--iterate from 4 to 2? in theory?
	for i, v in ipairs(storage) do
		if (count - i < 1) then break end
		--log.debug(count + 1 - i)
		storage[count + 1 - i] = storage[count - i]
	end
	--create blank storage, now we just see what we can find.
	storage[1] = create_pad_frame()
	--add values as appropriate. 
	if padinput:call("isAnyButtonTrigger(System.UInt32)",3840) == true then
		log.debug("Dpad press for breaker switch")
		-- this is up
		if padinput:call("isButtonTrigger(System.UInt32)",256) then
			storage[1].up = true
		end
		--this is down
		if padinput:call("isButtonTrigger(System.UInt32)",512) then
			storage[1].down = true
		end
		--this is left 
		if padinput:call("isButtonTrigger(System.UInt32)",1024) then
			storage[1].left = true
		end
		--this is right
		if padinput:call("isButtonTrigger(System.UInt32)",2048) then
			storage[1].right = true
		end
	end
end

local buffer_size = 4
local store_pad = initialize_pad_storage(buffer_size)

--checks each frame of the storage buffer, combines the values, and resets the storage
function evaluate_storage(storage)
	local composite_pad = create_pad_frame()
	
	for i,v in ipairs(storage) do
		if v.up then
			composite_pad.up = true
		end
		
		if v.down then
			composite_pad.down = true
		end
		
		if v.left then
			composite_pad.left = true
		end
		
		if v.right then
			composite_pad.right = true
		end
	end
	--reset buffer
	store_pad = initialize_pad_storage(buffer_size)
	return composite_pad
end

--This function should take padinput as a parameter and return a slot number if a breaker request occured. Isolate the breakaway stuff in a separate function.
local function GetRequestedGauntletSwitch(storage)
	--if there is anything in the final frame, check input type.
	if sdk.get_managed_singleton("app.HIDManager"):get_inputMode() == 0 then 
		if storage[buffer_size].up or storage[buffer_size].down or storage[buffer_size].left or storage[buffer_size].right then
			--get the composite + reset buffer. 
			local composite_pad = evaluate_storage(storage)
			if not disable_diagonal then
				if composite_pad.up and composite_pad.left then
					return 5
				end
				
				if composite_pad.up and composite_pad.right then
					return 6
				end
				
				if composite_pad.down and composite_pad.left then
					return 7
				end
				
				if composite_pad.down and composite_pad.right then
					return 8
				end
			end
			
			if composite_pad.up then
				return 1
			end
			
			if composite_pad.down then
				return 2
			end
			
			if composite_pad.left then
				return 3
			end
			
			if composite_pad.right then
				return 4
			end
		end
	else
		--keyboard checks
		if keyboard_breaker ~= -1 then
			local retval = keyboard_breaker
			keyboard_breaker = -1
			return retval
		end
	end
	return -1
end
--Return if we have requested a breakaway. These are separate than breaker switches now, god bless. 
local function GetRequestedBreakaway(padinput)
	--to start, checking and seeing if breakaway button was pressed.
	if padinput:call("isButtonTrigger(System.UInt32)",65536) then
		--then, if holding the breakaway button or always breakaway, return true.
		if padinput:call("isButtonOn",0x1000) or alwaysbreakaway then
			return true
		end
	else
		return false
	end
end


--safety checks:
--If nero has a breaker, it's started, and we're not doing a breaker attack currently:
--also, let's try using an index and see if this breaks immediately
local function get_validGauntletScenario(nero)
	local gauntlet_num = nero:get_gauntletNum()
	local nerotrack = nero:call("get_playerNeroTrack")
	--log.debug(gauntlet_num)
	if nero:get_gauntletNum() == 0 then
		--if infinite breakers is on, but nero doesn't have gauntlets we should trigger a manual gauntlet exchange to give him one from his list? oh boy I hope this shit works. 
		if nero_removeGauntlet.noConsumption then
			--no reason not to add i don't think?
			log.debug("out of gauntlets, triggering manual exchange")
			return true
		else
			return false
		end
	end
	
	local gauntlet = nero:get_gauntlet()
	if not gauntlet then 
		if last_state ~= state_table.NO_GAUNTLET then
			log.debug("No Gauntlet!")
			last_state = state_table.NO_GAUNTLET
		end
	else
		if not gauntlet:get_IsStarted() then 
			if last_state ~= state_table.GAUNTLET_NOT_STARTED then
				log.debug("Gauntlet Not Started!")
				last_state = state_table.GAUNTLET_NOT_STARTED
			end
		else
			--swapping Gauntlet Start for gauntlet reserve to see how this changes things
			--didn't work, swapping to command start to see if this makes the swaps snappier.
			if nerotrack.ChargeStart == false and nerotrack.GauntletStart == false and nerotrack.CommandStart == false and weapon_type == 3 then
				if last_state ~= state_table.GAUNTLET_IN_USE then
					log.debug("Gauntlet In Use!")
					last_state = state_table.GAUNTLET_IN_USE
				end
			else
				--special exception for double breaker, logging when this happens because it should only be like, ONE call, tops.
				if doing_breaker_swap then
					if last_state ~= state_table.BRINGER_SWAP then
						log.debug("Bringer to Breaker Swap Happening")
						last_state = state_table.BRINGER_SWAP
					end
				else
					if last_state ~= state_table.GAUNTLET_IDLE then
						log.debug("Gauntlet Idle")
						last_state = state_table.GAUNTLET_IDLE
					end
					local equippedID = nero:get_gauntletID()
					local listID = nero:get_currentGauntletID()
					--if there's a mismatch here that means we want to update nero's current gauntlet, so return true.
					if equippedID ~= listID then
						if last_state ~= state_table.GAUNTLET_MISMATCH then
							log.debug("Gauntlet MisMatch, trigger swap!")
							log.debug("Current breaker" .. equippedID)
							log.debug("Swap to breaker" .. listID)
							last_state = state_table.GAUNTLET_MISMATCH
						end
						return true
						--SwapBreakerSilent(listID)
					end
				end
			end
		end
	end
	return false
end

local function pre_player_doCharacterUpdateCommon(args)

	--ensure we are playing as Nero and have Nero player component.
	local player = sdk.to_managed_object(args[2])
	local playman = PlayerManipulator.get_playerManager()
	if (player:get_playerID() == 0 and PlayerManipulator.get_manualPlayer() == player) then
		nero = player
	else
		nero = nil
	end

	if not nero then return end
	
	local padinput = nero:call("get_padInput")

	--log.debug(nero:call("getCurrentNodeID(System.UInt32)",0))
	--this is dumb but we need to update ui after silentswap bc it clears the UI called by exchange breaker but also clears our UI call as well, so have to write status AFTER
	local dobreakeruiupdate = false
	
	--get the next input frame 
	iterate_pad(store_pad,padinput)
	
	--should be good to go.
	local slot = GetRequestedGauntletSwitch(store_pad)
	local is_breakaway = GetRequestedBreakaway(padinput)
	
	if slot ~= -1 then
		local breaker = breakerlist[slot]
		-- need to do smart breaker checks here.
		NeroSmartBreakerSwap(breaker,0)
		dobreakeruiupdate = true
	end

	
	
	if is_breakaway then
		--clear the button, and then write it back. Makes sure we don't accidentally add the button twice.
		padinput:call("clearButton(System.UInt32)",65536)	
		local trigger_flags = padinput:call("get_buttonTriggerFlags()")
		local trigger_flags = trigger_flags + 65536
		padinput:call("set_buttonTriggerFlags(System.UInt32)",trigger_flags)
	else
		--otherwise no breakaway for you.
		padinput:call("clearButton(System.UInt32)",65536)	
	end


	--simplest logic on planet earth: if the breaker in nero's list doesn't match the breaker he has equipped, swap the breaker (silently)
	local valid_gauntlet_switch = get_validGauntletScenario(nero)
	
	--if this is valid, we just trigger the gauntlet switch, easy. 
	if valid_gauntlet_switch then
		local listID = nero:get_currentGauntletID()

		--100 means empty list, so just add uh...overture I guess?
		if listID == 100 then
			SwapBreakerSilent(0)
			dobreakeruiupdate = true
		else
			SwapBreakerSilent(listID)
		end
	end
	--this is after any potential gauntlet exchanges, so safe to update UI now.
	if dobreakeruiupdate then
		--don't call the update if out of breakers and infinite breakers not on.
		if nero:get_gauntletNum() > 0 or nero_removeGauntlet.noConsumption then
			BreakerUIUpdate()
		end
		dobreakeruiupdate = false
	end
	--if DoubleBreaker.main_breakerID ~= nero:call("get_oldGauntletID") and (DoubleBreaker.buster_trigger == DoubleBreaker.buster_trigger_state.COMPLETE or DoubleBreaker.buster_trigger == DoubleBreaker.buster_trigger_state.IDLE) then -- swap back to main breaker
		--this field is true only after a breaker attack has finished, should be a good safety.
	--	if nerotrack:get_field("ChargeStart") == true then
			--SwapBreakerSilent(DoubleBreaker.main_breakerID)
			--clear trigger
	--		DoubleBreaker.buster_trigger = DoubleBreaker.buster_trigger_state.IDLE
	--	end
	--end
end

local function post_player_doCharacterUpdateCommon(retval)
	return retval
end

sdk.hook(player_doCharacterUpdateCommon_method,pre_player_doCharacterUpdateCommon,post_player_doCharacterUpdateCommon)





local function manualBreakaway(consumption)
	if not nero then else
		
		--nero:call("set_currentEventTrigger(System.UInt64)",0x100000000)
		--local status = nero:get_neroStatusTmp()
		--nero:call("set_currentEventTrigger(System.UInt64)",0x100000000)
		--status:call("add(System.UInt32)",8)
		nero:call("setCommandAction(System.String, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority, System.Single, via.motion.InterpolationMode)",
					"GauntletBomb",true,true,false,0,-1.0,1)
		--nero:call("requestBreakway()")
	end
end



SetNeroBreakers = {}
SetNeroBreakers.GauntletID = {}
SetNeroBreakers.GauntletID.variable = 0
SetNeroBreakers.GauntletIndex = {}
SetNeroBreakers.GauntletIndex.variable = 0


function ui_function()
	local draw_comp = create_pad_frame()
	for i,v in ipairs(store_pad) do
		local frame = tostring(v.up) .. ",".. tostring(v.down).. ",".. tostring(v.left).. ",".. tostring(v.right)
		if v.up then 
			draw_comp.up = true
		end 
		if v.down then 
			draw_comp.down = true
		end 
		
		if v.left then 
			draw_comp.left = true
		end 
		
		if v.right then 
			draw_comp.right = true
		end 
		--imgui.text(frame)
	end
	if draw_comp.up then
		--imgui.text("Up")
	end
	
	if draw_comp.down then
		--imgui.text("Down")
	end
	
	if draw_comp.left then
		--imgui.text("Left")
	end
	
	if draw_comp.right then
		--imgui.text("Right")	
	end
	local diagonal_change,diagonal_value =  imgui.checkbox("Disable diagonals",disable_diagonal)
	if diagonal_change then
		disable_diagonal = diagonal_value
	end
	window_changed,window_value = imgui.drag_int("Buffer window",buffer_size,1,2,12)
	
	changed,value=  imgui.drag_int("Debug Breaker", custombreaker, 1, 0, 10)
	
	
	btb_cancel_change,btb_cancel_value = imgui.checkbox("Breaker to breaker Cancel",breaker_to_breaker_cancel)
	if btb_cancel_change then
		breaker_to_breaker_cancel = btb_cancel_value
	end
	
	if changed then
		custombreaker = value
	end
	
	DoubleBreaker.enabled_change,DoubleBreaker.enabled_value = imgui.checkbox("Double Breaker",DoubleBreaker.enabled)
	if DoubleBreaker.enabled_change then
		DoubleBreaker.enabled = DoubleBreaker.enabled_value
	end
	
	SetNeroBreakers.GauntletID.change, SetNeroBreakers.GauntletID.value = imgui.drag_int("Set Breaker ID", SetNeroBreakers.GauntletID.variable, 1, 0, 10)
	SetNeroBreakers.GauntletIndex.change, SetNeroBreakers.GauntletIndex.value = imgui.drag_int("Set Breaker Index",SetNeroBreakers.GauntletIndex.variable, 1, 0, 7)
	
	if SetNeroBreakers.GauntletID.change then
		SetNeroBreakers.GauntletID.variable = SetNeroBreakers.GauntletID.value
	end
	
	if SetNeroBreakers.GauntletIndex.change then
		SetNeroBreakers.GauntletIndex.variable = SetNeroBreakers.GauntletIndex.value
	end
	
	if imgui.button("Update Values in nero list") then
		SetNeroBreakerInList(SetNeroBreakers.GauntletID.variable,SetNeroBreakers.GauntletIndex.variable)
	end
	
	if imgui.button("Do breakaway") then
		manualBreakaway(false)
	end
	
	if imgui.button("Punchline Recall Test") then
		nero:call("setRocketPunchReturn")
	end
	
	if imgui.button("Demonstrate game code") then
		nero:call("exchangeGauntlet(app.GauntletID)",custombreaker) --exchange the gauntlet
		nero:call("get_neroStatusTmp"):call("clear") --cancel the normal ui update
		nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x20) --breaker pickup ui update
		nero:call("triggerSE(System.UInt32)",0x7F7009BB) --breaker swap sound
		nero:call("get_cachedWwiseFacialCtrl"):call("trigger(System.UInt32)",0xA2982798) --nero voiceline
	end
	
	if imgui.button("Test Logging") then
		log.info("log.info call")
		log.warn("log.warn call")
		log.error("log.error call")
		log.debug("log.debug call")
	
	end
	
	if imgui.button("Add Breaker") then
		dobreakeradd = true
	end
	
		if imgui.button("exchange breaker") then
		NeroSmartBreakerSwap(custombreaker)
	end
	
	if imgui.button("Cycle Breaker") then
		CycleBreakers()
	end
	
	if imgui.button("Pasta Breaker") then
		if not nero then else
			nero:call("SetToEndGauntlet(System.Int32)",1)
			--0x40000 = pastabreaker
			--0x00800 = normal swap
			nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x40000)
		end
	end
	
	
	if imgui.button("Blow Up Punchline") then
		if not nero then else 
			nero:call("reqJetGadgetExplode")
		end
	end
	
	if imgui.button("keyboard breaker test") then
		keyboard_breaker = 2
	end
	
	if imgui.button("Remove Breaker No Consumption") then
		if not nero then else 
			nero:call("removeGauntlet",true)
		end
	end
	

	tomboychange,tomboyvalue = imgui.checkbox("Auto Activate Tomboy On Swap",autotomboypop)
	
	if tomboychange then
		autotomboypop = tomboyvalue
	end
	
	alwaysbreakawaychange, alwaysbreakawayvalue = imgui.checkbox("Always breakaway",alwaysbreakaway)
	if alwaysbreakawaychange then
		alwaysbreakaway = alwaysbreakawayvalue
	end
	
	consumption_change,consumption_value = imgui.checkbox("Infinite Breakers",nero_removeGauntlet.noConsumption)
	if consumption_change then
		nero_removeGauntlet.noConsumption = consumption_value
	end
	


	for i,v in ipairs(breakerlist) do
		local string = "Breaker Slot " .. i
		changed,value = imgui.drag_int(string, v, 1, 0, 10)
		if changed then 
			breakerlist[i] = value
		end
	end
	
	
	
	if imgui.button("Voiceline") then
		if not nero then else 
			nero:call("get_cachedWwiseFacialCtrl"):call("trigger(System.UInt32)",0xA2982798)
		end
	end
	
	if imgui.button("Set action") then
		setNeroAction("RF_End")
	end
	
	if imgui.button("Set RF node") then
		if not nero then else
			doRFNode()
		end
	end
	imgui.text("Buster Trigger: ".. tostring(DoubleBreaker.buster_trigger))
	imgui.text("Request Attack: ".. tostring(DoubleBreaker.requesting_alt_breaker_attack))


end

--re.on_draw_ui(ui_function)

re.on_pre_application_entry("UpdateBehavior", function()


	--log.debug("Gameplay status: " ..tostring(PlayerManipulator.isGameplay()))

	if not nero then return end
	--manually turn off infinite breakers in training mode because I don't want to deal with that right now.
	--probably will need to fix this later.
	if nero:call("get_gauntletNum") > 8 then
		log.warn("Gauntlet Number greater than 8,something is very wrong.")
		nero:call("set_gauntletNum",8)
	end
	
	if dotomboypop then
		if nero:call("get_oldGauntletID") == 7 then 
			RFOn()
			dotomboypop = false
		end
	end



	
	if dobreakeradd  and nero:call("get_playerNeroTrack"):get_field("BreakawayOff") == false then
		
		if nero:call("get_gauntletNum") > 7 then
			log.debug("At max breaker capacity!")
		else
			nero:call("pushGauntlet",custombreaker,true)
			nero:call("get_neroStatusTmp"):call("add(app.PlayerNero.NeroStatusTmp)",0x20) --breaker pickup ui update
			nero:call("triggerSE(System.UInt32)",0x7F7009BB) --breaker swap sound
			nero:call("get_cachedWwiseFacialCtrl"):call("trigger(System.UInt32)",0xA2982798) --nero voiceline
		
		end
		dobreakeradd = false
	end

	--dobreakeradd = false
	
	--log.debug("got nero")
end)