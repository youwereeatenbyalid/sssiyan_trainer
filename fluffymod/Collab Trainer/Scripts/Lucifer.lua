PlayerManipulator = require "PlayerManipulator"

enumerator = require "Enumerator"


--function to set punch kick state
--set_weaponBalrog(app.PlayerDante.WeaponS_Balrog)
--get_weaponBalrog()
--get_cachedMotion()
	--getLayer(System.UInt32)
		--get_MotionID()
		--get_Frame()		
	--
	
	
	
	
--Air Trick Offset Default: 0, -0.8, 1.5
--Ground Trick Appear Target Distance: 1.8
		
dante = nil --stores dante 
currentid = 0
currentbankid = 0
currentframe = 0
isdarkslayer = false




sword_range = 25.0 -- what we use to control interceptor lock-on. This value is squared and then compared against the square of the x + z distance from enemy (just pythagorean theorem)
-- issue is it doesn't account for up/down and also the comparison feels wonky sometimes. Double check how this works if you have a free day.








supporttype_index = 0
supportypes = enumerator.generate_enum("app.DevilSwordSheathController.SupportType")
supportypes_names = enumerator.generate_reverse_enum("app.DevilSwordSheathController.SupportType")



actionendtype_index = 0
actionendtypes = enumerator.generate_enum("app.player.pl0100.weapon.ActionTheEndType")
actionendtypes_names = enumerator.generate_reverse_enum("app.player.pl0100.weapon.ActionTheEndType")


action_index = 0
swordactions = enumerator.generate_enum("app.DevilSwordSheathController.Action")
swordactions_names = enumerator.generate_reverse_enum("app.DevilSwordSheathController.Action")



--Finds a valid lock-on target, assigns it to the sword, and calls the fire interceptor action 
--sword won't fire without a lock-on target. 
function firesword(sword)
		if not dante then return end
		--old method of accessing directly via 
		--TargetWork = sdk.get_managed_singleton("app.LockOnTargetManager"):call("get_accessableList()"):call("get_Item(System.Int32)",1)
		
		--getting the target from dante sometimes doesn't work when dante isn't locked on.
		--make getting the lock-on more consistent somehow and also provide feedback to user when out of range.
		
		local target = dante:get_lockOnTarget()
		if not target then log.info("No targets available") return end
		TargetWork = target:get_target()
		if not TargetWork then log.info("Dante's lock-on target object returned null for the targetwork!") return end

		if TargetWork:get_playerLengthSQ() > (sword_range*sword_range) then
			log.info("hey that's too far away")
			return
		end 
		
		sword:call("get_lockonTarget()"):call("set__target(app.LockOnTargetWork)",TargetWork)	
		--hey that third variable there is a filter or something, probably decides if it targets enemies or players or something
		sword:call("setActionWithOption(app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)", 45,2,1,0,0,0,0)
end



---Function that checks to see whioch swords are in use, then calls firesword on an available sword.
---this should be redone to use sword_table
function fire_a_sword(list)
	--working off 0 index here, so count-1 for max of loop
	local size = list:get_Count()-1
	for i = 0,size,1 --using a for loop inside the UI
	do 
		local sword = list:call("get_Item(System.Int32)",i)
		local action = sword:get_isAction()
		if not action then
				firesword(sword)
			return
		end 
	end
	imgui.text("All swords in use!")
end

--This function takes mirage swords 5-8, sets their support type to reactor (aka trickster style) and then sets their sheath id's to index 0-4
--This results in the swords being treated as non-sdt swords but positioned in the reactor formation so they don't overlap with the first 4 swords
-- Method is currently hardcoded and should be integrated with sword_table.
function set_extra_ammo(list)
	for i = 4,7,1 --using a for loop inside the UI
	do 
	
		local sword = list:call("get_Item(System.Int32)",i)
		sword:call("set_supportType(app.DevilSwordSheathController.SupportType)",2)
		sword:call("set_sheathID(System.Int32)",i-4)
	end
	imgui.text("All swords in use!")
end 

function hide_extra_ammo(list)
	for i = 4,7,1 --using a for loop inside the UI
	do 
		local sword = list:call("get_Item(System.Int32)",i)
		sword:call("set_supportType(app.DevilSwordSheathController.SupportType)",2)
		sword:call("set_sheathID(System.Int32)",i)
	end
	imgui.text("All swords in use!")
end 

--Sword table
--responsible for storing custom properties to manage the swords.

local sword_table = nil

function initialize_sword_table(swordlist)
	local sword_table = {}
	local size = swordlist:get_Count()
	for i = 1,size,1 do
		local sword = {}
			sword.isLucifer = true -- boolean used to determine if sword is being used as part of lucifer or not.
			sword.doAction = false -- while isLucifer is true, swords are not permitted to perform actions unless this flag is set to true.
			-- after this value is checked, it should always be reset to false.
			
			sword.isFreeze = false -- determines if sword should be frozen
			sword.canEmbed = false -- determines if sword can embed into enemy
			sword.index = i-1 --Index can be used to get swords from the ds_sheath controller
			sword.object = swordlist:call("get_Item(System.Int32)",sword.index)
			sword.timer = 0.0 --Used to track how far through an action a sword is. Updated through CommonAction, can't be checked in the swordsheath itself
			sword.detonate_timer = 0.0
			sword.joints = save_bone_positions(sword.object:get_cachedTransform()) -- a table that stores the position, rotation and hashes of all the bones in the sword. Used for manual positioning
			--plan: 0 = normal, 1 = on field, 2 = in enemy, 3 = explode?
			sword.state = 0 --tracks the state of the sword in the context of being used as part of lucifer
			sword.enemy_joint = nil --tracks the joint of an enemy that the sword has been embedded into
			--may need a field to track the enemy itself?
		sword_table[i] = sword
	end
	return sword_table
end



--Used to update the timers on the swords. Will need to expand this for tracking parts of lucifer combo.

DSS_Commonaction_update_method = sdk.find_type_definition("app.fsm2.player.pl0100.weapon.wp01_006.CommonAction"):get_method("update(via.behaviortree.ActionArg)")




function pre_DSS_Commonaction_update_method(args)
	
	if not sword_table then return end 
	
	action_object = sdk.to_managed_object(args[2])
	--iterate through tables to find which sword is being updated here.
	for key,value in ipairs(sword_table) do
		if value.object == action_object:get_cachedMirage() then
			--value.timer = action_object:get_timer()
			local checkframevalue = value.object:get_cachedMotion():getLayer(0):get_Frame()
			log.info(checkframevalue)
			value.timer = checkframevalue --action_object:getFrame(0)
		end
	end
end



function post_DSS_Commonaction_update_method(retval)
	return retval
end
sdk.hook(DSS_Commonaction_update_method,pre_DSS_Commonaction_update_method,post_DSS_Commonaction_update_method)
	






--resets the states of all the swords to their defaults.
function reset_sword_table(sword_table)
	for key,value in ipairs(sword_table)
	do 
		value.isLucifer = true
		value.doAction = false
		value.isFreeze = false
		value.canEmbed = false
		value.timer = 0.0
		value.detonate_timer = 0.0
		value.object:set_sheathID(value.index) --resets the sheath ID to its default (based on the index of the swordlist)
		value.state = 0
		value.enemy_joint = nil
	end
end 



-- gets the bone positions of the sword.
-- each sword has 3 bones: Null, used to orient the rest of the animation, a bone for the model, and then a bone for the hitbox/effects.
function save_bone_positions(transform)
	local joint_table = {{},{},{}}
	local joints = transform:get_Joints():get_elements()
	for key, value in ipairs(joints) do
		joint_table[key].pos = value:get_Position()
		joint_table[key].rot = value:get_Rotation()
		joint_table[key].hash = value:get_NameHash()
	end
	return joint_table

end


--writes the bone positions of the sword. The correct bones are found by using the hash of the bone's name.
function load_bone_positions(transform,joints)
	for key, value in ipairs(joints) do
		local joint = transform:getJointByHash(joints[key].hash)
		joint:set_Position(joints[key].pos)
		joint:set_Rotation(joints[key].rot)
	end
	--local joint = transform:getJointByHash(joints[3].hash)
	--joint:set_Position(joints[2].pos)
	--joint:set_Rotation(joints[2].rot)
end

--sets the position and rotation of the sword transform to match a bone
function bind_bones_to_enemy(sword_transform,enemy_joint,joints)
	for key, value in ipairs(joints) do
		local joint = sword_transform:getJointByHash(joints[key].hash)
		joint:set_Position(enemy_joint:get_Position())
		joint:set_Rotation(enemy_joint:get_Rotation())
	end
end



	
local hitaddress = ""
	

--A list of bones the swords are allowed to attach to on enemies.
local Valid_BoneList = {

	"Waist", 
	"Stomach", 
	"Chest",
	"Neck",
	"Head",
	"L_UpperArm",
	"L_ForeArm",
	"L_Thigh",
	"L_Shin",
	"R_UpperArm",
	"R_ForeArm",
	"R_Thigh",
	"R_Shin"
}





--Finds the closest bone to the specified position that's also present in the list of valid bones.
--used to find the bone to attach the sword to on hit
-- position: The position of the sword 
-- joints: a list of all the joints on the enemy to check
-- Valid_BoneList: a whitelist of joints acceptable to attach the sword to (so we don't attach to some cloth or a pinky) 
function get_nearest_joint(sword_position,joints,Valid_BoneList)

	local shortest_length = 0
	local shortest_joint = 0
	log.info("Begin bone list")
	for k,v in ipairs(joints) do
		local bonename = v:get_Name()
		local skipbone = true
		--check if on list of acceptable bones
		for boneindex,bonevalue in ipairs(Valid_BoneList) do
			if bonevalue == bonename then
				skipbone = false
			end
		end
		--gets the difference between the two positions and returns length
		local distance = (v:get_Position() - sword_position):length()
	
		log.info("bone " ..bonename .. " has distance of " .. distance .. " from sword")

		if not skipbone then
			if shortest_joint == 0 then
				shortest_length = distance
				shortest_joint = k
			else
				if distance < shortest_length then
					shortest_length = distance
					shortest_joint = k
				end
			end
		end
	end
	return joints[shortest_joint]

end

--Used to adjust sword knockback properties on hit and to bind the swords to enemies.
local DSS_OnHit_method = sdk.find_type_definition("app.WeaponDevilSwordSheath"):get_method("onAttackHit")

function pre_DSS_OnHit_method(args)
	local sword = sdk.to_managed_object(args[2])
	--get the properties of the attack.
	local hitcontroller_damageinfo = sdk.to_managed_object(args[3])
	local attackdata = hitcontroller_damageinfo:get_AttackUserData()
	log.info("On Attack hit called")
	--log.info(attackdata.DamageTypeBlown)
	--log.info(attackdata.AttackType)
	--log.info(attackdata.AttackID)
	--log.info(attackdata.DamageValue)
	--log.info(attackdata.DmgReactionValue)
	--log.info(attackdata.SubReactionBelowThreshold)
	for key,value in ipairs(sword_table) do
		
		--if not being used by lucifer then perform normal behaviour
		if value.object == sword and value.isLucifer == false then return end
		
		
		--if the sword is in detonation state, we adjust the properties to do damage and knockback
		if value.object == sword and value.state == 3 then
			attackdata.DamageTypeBlown = 4
			attackdata.DmgReactionValue = 50.0
			--set this to some arbitrary value that will conceal the sword
			value.object:set_sheathID(0)
			value.doAction = true
			doconvergence(sword)
			value.state = 0
		
		else
		--if the sword is in binding state, we adjust the properties to not do damage or knockback
			if value.object == sword and ((value.state == 0 and value.canEmbed) or value.state == 1) then
				value.canEmbed = false
				value.isFreeze = false
				attackdata.DamageTypeBlown = 0
				attackdata.DmgReactionValue = 0.0
			
				value.doAction = true
				dosupportreturn(sword)
				value.state = 2
				
				--assigning the sword the nearest joint on the enemy it can attach to.
				local enemy_joints = hitcontroller_damageinfo:get_DamageGameObject():get_Transform():get_Joints():get_elements()
				local sword_position = value.joints[2].pos
				
				value.enemy_joint = get_nearest_joint(sword_position,enemy_joints,Valid_BoneList)
				return sdk.PreHookResult.SKIP_ORIGINAL 
			end
		end
	end
end
	
function post_DSS_OnHit_method(retval)
	return retval
end




sdk.hook(DSS_OnHit_method,pre_DSS_OnHit_method,post_DSS_OnHit_method)	


--Overwriting the action function for the swords so that they aren't called to do something while being used by lucifer.
local DSS_SetAction_method = sdk.find_type_definition("app.WeaponDevilSwordSheath"):get_method("setAction(System.String, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)")


local DSS_SetAction_method2 = sdk.find_type_definition("app.WeaponDevilSwordSheath"):get_method("setAction(app.DevilSwordSheathController.Action, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve)")

function pre_DSS_SetAction_method(args)
	local sword = sdk.to_managed_object(args[2])
	for key,value in ipairs(sword_table) do
		--check which sword we're using
		if value.object == sword then
			log.info("Checking sword " .. key)
			--if not part of lucifer, do normal return.
			if value.isLucifer == false then return sdk.PreHookResult.CALL_ORIGINAL  end
			--while swords are in 0 state, the lucifer functionality hasn't been activated yet, so allow normal behaviour.
			if value.state == 0 then return sdk.PreHookResult.CALL_ORIGINAL  end
			
			--if doAction has been set, the sword has permission to be doing an action, so allow it. 
			if value.doAction == true then
				value.doAction = false 
				return sdk.PreHookResult.CALL_ORIGINAL 
			end 
			--if we haven't returned after any of this, this is a lucifer sword, not in state 0, without permission to do an action, so return skip original.
			return sdk.PreHookResult.SKIP_ORIGINAL
		end
	end
	return sdk.PreHookResult.CALL_ORIGINAL 
end

function post_DSS_SetAction_method(retval)
	return retval
end

sdk.hook(DSS_SetAction_method2,pre_DSS_SetAction_method,post_DSS_SetAction_method)	
sdk.hook(DSS_SetAction_method,pre_DSS_SetAction_method,post_DSS_SetAction_method)	


--call to trigger a stationary active hitbox
function dohelmbreaker(sword)
		--local id = sword:get_sheathID()
		sword:set_sheathID(0)
		sword:call("setActionWithOption(app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)", 27,1,1,0,0,0,0)
		--sword:set_sheathID(id)
end


-- call to transition the sword to a non-damaging state
function dosupportreturn(sword)
	
	sword:call("setActionWithOption(app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)", 5,2,1,0,0,0,0)
end

--call to hide the sword after detonation
function doconvergence(sword)
	
	sword:call("setActionWithOption(app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)", 2,2,1,0,0,0,0)
end


function get_positions(sword_table)
	if not sword_table then else
		for key,value in ipairs(sword_table) --using a for loop inside the UI
			do 
				value.joints = save_bone_positions(value.object:get_cachedTransform())
		end
	end
end



local lucifer_enabled = true --was being used to determine if lucifer bones should be used, need to replace with something neater.


local freeze_cutoff = 45.0 -- point in animation swords should be frozen

local sword_detonate = false -- set this true to make the swords detonate


local action_cancel_frame = 25.0


local combo_index = 1

local LuciferTrack = {
	index = 1,
	ComboStart = false,
	AttackReserve = false,
	AttackStart = false,
	DelayReserve = false,
	DelayStart = false,
	isFreeze0 = false,
	canEmbed0 = false,
	isFreeze1 = false,
	canEmbed1 = false,
	isFreeze2 = false,
	canEmbed2 = false,
	isFreeze3 = false,
	canEmbed3 = false,
}


function reset_lucifer_track(LuciferTrack)
	LuciferTrack.ComboStart = false
	LuciferTrack.AttackReserve = false
	LuciferTrack.AttackStart = false
	LuciferTrack.DelayReserve = false
	LuciferTrack.DelayStart = false
	LuciferTrack.isFreeze0 = false
	LuciferTrack.canEmbed0 = false
	LuciferTrack.isFreeze1 = false
	LuciferTrack.canEmbed1 = false
	LuciferTrack.isFreeze2 = false
	LuciferTrack.canEmbed2 = false
	LuciferTrack.isFreeze3 = false
	LuciferTrack.canEmbed3 = false
end

-- local key_track = {
	-- name = "",
	-- start_frame = 0.0,
	-- end_frame = 0.0
-- }


function makeKey(_name,_startframe,_endframe)
	local key_track = {
		name = _name,
		start_frame = _startframe,
		end_frame = _endframe
	}
	return key_track
end

-- local animation_info = {
	-- id = 0
	-- keys = {}
-- }

function initialize_animation_info(_id)
	local animation_info = {
		id = _id,
		next_node = 1,
		next_node_delay = -1,
		keys = {}
	}
	return animation_info
end


function initialize_combo_path()
	local path = {}
	--creates the first node in the list
	path[#path+1]=initialize_animation_info(0)
	--create the next node in the list aka first hit of the combo
	path[#path+1]=initialize_animation_info(8)
	--point the root node to point to the current end of the list, aka first hit of the combo
	path[#path-1].next_node = #path
	
	path[#path].keys[#path[#path].keys+1] = makeKey("AttackStart",25,35)
	path[#path].keys[#path[#path].keys+1] = makeKey("AttackReserve",10,24)
	
	
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze2",23,30)
	
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze3",28,30)
	--path[#path].keys[#path[#path].keys+1] = makeKey("isEmbed2",36,40)
	
	
	path[#path].keys[#path[#path].keys+1] = makeKey("DelayStart",41,50)
	path[#path].keys[#path[#path].keys+1] = makeKey("DelayReserve",36,40)
	
	
	path[#path+1]=initialize_animation_info(9)
	path[#path-1].next_node = #path
	
	path[#path].keys[#path[#path].keys+1] = makeKey("AttackStart",28,38)
	path[#path].keys[#path[#path].keys+1] = makeKey("AttackReserve",12,27)
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze2",23,30)
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze3",28,30)

	path[#path+1]=initialize_animation_info(10)
	path[#path-1].next_node = #path
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze1",23,30)
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze2",23,30)
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze3",28,30)
	
	path[#path+1]=initialize_animation_info(11)
	path[2].next_node_delay = #path
	
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze2",23,30)
	path[#path].keys[#path[#path].keys+1] = makeKey("isFreeze3",28,30)	
	
	
	return path
	--animation_info.keys[#animation_info.keys+1] = makeKey(_name,_startframe,_endframe)
end 


--local combopath = {8,9,10}
local combopath = initialize_combo_path()

--Updates LuciferTrack's properties based on the current animation info
--LuciferTrack = stores the properties to update
--animation_data = values the properties should be based on the current animation
--frame = floating point number that tracks how far through the animation we are
function UpdateLuciferTrack(LuciferTrack,animation_data,frame)
	
	reset_lucifer_track(LuciferTrack)

	for key,value in ipairs(animation_data.keys) do
		if frame >= value.start_frame and frame <= value.end_frame then
			LuciferTrack[value.name] = true
		else
			LuciferTrack[value.name] = false
		end
	end
end


--Dynamically maps the combo properties to swords based on their position in the formation attack
function UpdateSwordVars(LuciferTrack,sword_table)
	for key,value in ipairs(sword_table) do
		local id = value.object:get_sheathID()
		if id  >=0 and id < 4 then
			value.isFreeze = LuciferTrack["isFreeze" .. tostring(id)]
			value.canEmbed = LuciferTrack["canEmbed" .. tostring(id)]
		end
	end
end




--Used to reset the index of the lucifer combo when summon swords naturally end action

DevilSwordSheathAction_setActionEnd_method = sdk.find_type_definition("app.fsm2.player.pl0100.weapon.wp01_006.DevilSwordSheathAction"):get_method("setActionEnd(via.behaviortree.ActionArg, System.Boolean)")


function pre_DevilSwordSheathAction_setActionEnd_method(args)
	if not sword_table then return end 
	if not LuciferTrack then return end
	log.info("Calling what should just be summon sword end method")
	action_object = sdk.to_managed_object(args[2])
	
	--check to see if sword # 1 is being called here
	if sword_table[1].object == action_object:get_cachedMirage() then
		log.info("Sword " .. 1 .. " action ended")
		LuciferTrack.index = 1
	end
end

 function post_DevilSwordSheathAction_setActionEnd_method(retval)
	--sword_in_actionend:get_cachedMirageController():set_isExpansion(false)
	return retval
end

sdk.hook(DevilSwordSheathAction_setActionEnd_method,pre_DevilSwordSheathAction_setActionEnd_method,post_DevilSwordSheathAction_setActionEnd_method)





local combo_reserve = false
local combo_button = false
local delay_reserve = false


WeaponDevilSword_doAttackP_method = sdk.find_type_definition("app.WeaponDevilSword"):get_method("doAttackP()")


function pre_WeaponDevilSword_doAttackP_method(args)
	combo_button = true
	return sdk.PreHookResult.SKIP_ORIGINAL 
end

 function post_WeaponDevilSword_doAttackP_method(retval)
 
	return retval
end

sdk.hook(WeaponDevilSword_doAttackP_method,pre_WeaponDevilSword_doAttackP_method,post_WeaponDevilSword_doAttackP_method)

PlayerDante_checkProvokeCancel_method = sdk.find_type_definition("app.PlayerDante"):get_method("checkProvokeCancel()")


function pre_PlayerDante_checkProvokeCancel_method(args)
	sword_detonate = true
end

 function post_PlayerDante_checkProvokeCancel_method(retval)
 
	return retval
end

sdk.hook(PlayerDante_checkProvokeCancel_method,pre_PlayerDante_checkProvokeCancel_method,post_PlayerDante_checkProvokeCancel_method)





--move info:
-- reserve frame
-- cancel frame
-- reserve delay frame
-- cancel delay frame
-- sword formation
-- offset info etc.
-- 


local cached_driver_key = -1

function make_swords_available(sword_table)
	for key,value in ipairs(sword_table) do
		value.object:set_sheathID(-1)
	end 
	
	for i=0,3 do
		for key,value in ipairs(sword_table) do
			if value.isLucifer and value.state == 0 and value.object:get_sheathID() == -1 then
				value.object:set_sheathID(i)
				if i == 0 then
					cached_driver_key = key
				end
				break
			end
		end 
	end
end



--this method for delay is bad, fix later or osomething
function do_combo_smart(dante,combopath,LuciferTrack,isdelay) 
			--advance to next stage of combo
			if isdelay then
				LuciferTrack.index = combopath[LuciferTrack.index].next_node_delay
			else
				LuciferTrack.index = combopath[LuciferTrack.index].next_node
			end
			--if we've reached the end of the combo, wrap around to the beginning
			if LuciferTrack.index > #combopath or LuciferTrack.index < 1 then
				log.info("Index out of range, returning to beginning of combo path")
				LuciferTrack.index = 1
			end
			
			make_swords_available(sword_table)
			
			--actually trigger the move
			do_combo(dante,combopath[LuciferTrack.index].id,0,2)
end

function do_combo(dante,actionid,supportid,actionendtypeid)

		dante:call("get_cachedMirageCtrl"):call("setAction(app.DevilSwordSheathController.SupportType, app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)",
												supportid,actionid,actionendtypeid,15,1,0,0,0,0)
end

function make_all_swords_lucifer(sword_table)
	for key,value in ipairs(sword_table) do
		value.isLucifer = true
	end
end


local examine_node_index = 2

re.on_draw_ui(function()
	
	if not dante then return end

	imgui.input_text("Enemy address",hitaddress)
	
	
	
	
	if not combopath then else
		examine_node_index_changed,examine_node_index_value = imgui.drag_int("Node ID", examine_node_index,1,1,#combopath)
		if examine_node_index_changed then
			examine_node_index = examine_node_index_value
		end
		for key,value in ipairs(combopath[examine_node_index].keys) do
			local start_frame_change,start_frame_value = imgui.drag_float(value.name .. " start frame",value.start_frame,1,-2,100)
			local end_frame_change,end_frame_value = imgui.drag_float(value.name .. " end frame",value.end_frame,1,-2,100)
			if start_frame_change then
				value.start_frame = start_frame_value
			end
			
			if end_frame_change then
				value.end_frame = end_frame_value
			end
		end 
	end
	
	if not sword_table then else
		
		if imgui.button("Make all swords luficer") then
			make_all_swords_lucifer(sword_table)
		end
		imgui.text("Animation Timer"..sword_table[1].timer)
		imgui.checkbox("Attack Reserve",LuciferTrack.AttackReserve)
		imgui.checkbox("Attack Start",LuciferTrack.AttackStart)
		imgui.checkbox("Delay Reserve",LuciferTrack.DelayReserve)
		imgui.checkbox("Delay Start",LuciferTrack.DelayStart)
		isFreeze1_change,isFreeze1_value = imgui.checkbox("is Freeze 1",LuciferTrack.isFreeze1)
		if isFreeze1_change then
			LuciferTrack.isFreeze1 = isFreeze1_value
		end
	end
	
	
	changedaction3,actionvalue3= imgui.combo("Support list", supporttype_index, supportypes_names)
	if changedaction3 then
		supporttype_index = actionvalue3
	end


	changedaction,actionvalue= imgui.combo("Action list", action_index, swordactions_names)
	if changedaction then
		action_index = actionvalue
	end
	
	imgui.text(action_index)
	
	changedaction2,actionvalue2= imgui.combo("Action end type list", actionendtype_index, actionendtypes_names)
	if changedaction2 then
		actionendtype_index = actionvalue2
	end
	
	
	local combo_index_changed,combo_index_value = imgui.drag_int("Combo stage",LuciferTrack.index,1,1,10)
	if combo_index_changed then
		LuciferTrack.index = combo_index_value
	end
	
	if imgui.button("Do Combo") then
		combo_button = true
		--use chaser support and follow end action
		--do_combo(dante,combopath[combo_index],0,2)
	end
	
	
	local changedrange,rangevalue = imgui.drag_float("Sword Range",sword_range,1,1,100)
	if changedrange then
		sword_range = rangevalue
	end
	
	local swordlist = dante:call("get_cachedMirageCtrl"):call("get_SheathObjList()")
	
	if not sword_table then else
		for key,value in ipairs(sword_table) --using a for loop inside the UI
			do 
				if imgui.collapsing_header("Sword Joints "..key) then
					if imgui.button("Fire Sword " .. key) then
						firesword(value.object)
					end
					
					local isLucifer_changed,isLucifer_value = imgui.checkbox("isLucifer " .. key,value.isLucifer)
					if isLucifer_changed then
						value.isLucifer = isLucifer_value
					end 
								
					local isFreeze_changed,isFreeze_value = imgui.checkbox("isFreeze " .. key,value.isFreeze)
					if isFreeze_changed then
						value.isFreeze = isFreeze_value
					end 
								
					local canEmbed_changed,canEmbed_value = imgui.checkbox("canEmbed " .. key,value.canEmbed)
					if canEmbed_changed then
						value.canEmbed = canEmbed_value
					end 
					
					
					local action = value.object:get_isAction()
					imgui.text("is action" .. tostring(action))
					imgui.text("Timer " .. value.timer)
					imgui.text("State " .. value.state)
					
					if not value.joints then else
						--if imgui.collapsing_header("Sword Joints "..key) then
							for k,v in ipairs(value.joints) do
								imgui.text("Joint Hash" .. v.hash)
								imgui.drag_float3("Joint position ".. key .. k,v.pos,1,-200,200)
								imgui.drag_float4("Joint rotation"..key .. k,v.rot,1,-200,200)
							end
						--end
					end
					
					if not value.enemy_joint then else
						imgui.drag_float3("Enemy bone position",value.enemy_joint:get_Position())
						imgui.text(value.enemy_joint:get_Name())
					end
				end
		end
	end

	if imgui.button("Fire a sword") then
		fire_a_sword(swordlist)
	end

	if imgui.button("More Swords") then
		set_extra_ammo(swordlist)
	end 
	
	if imgui.button("Hide Swords") then
		hide_extra_ammo(swordlist)
	end 
	if imgui.button("Reset Swords") then
		reset_sword_table(sword_table)
	end 
	if imgui.button("Do action") then
		
		supportid = supportypes[supportypes_names[supporttype_index]]
		actionid = swordactions[swordactions_names[action_index]]
		actionendtypeid = actionendtypes[actionendtypes_names[actionendtype_index]]
		
		
		dante:call("get_cachedMirageCtrl"):call("setAction(app.DevilSwordSheathController.SupportType, app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)",
												supportid,actionid,actionendtypeid,15,1,0,0,0,0)
	end
	
	
	if imgui.button("Do action single sword") then
		
		supportid = supportypes[supportypes_names[supporttype_index]]
		actionid = swordactions[swordactions_names[action_index]]
		actionendtypeid = actionendtypes[actionendtypes_names[actionendtype_index]]

		summoned_sword = dante:call("get_cachedMirageCtrl"):call("get_SheathObjList()"):call("get_Item(System.Int32)",0)
		--summoned_sword:call("get_lockonTarget()"):call("set__target(app.LockOnTargetWork)",TargetWork)

		summoned_sword:call("setActionWithOption(app.DevilSwordSheathController.Action, app.player.pl0100.weapon.ActionTheEndType, System.UInt32, app.player.pl0100.weapon.MirageMotionOption, app.player.pl0100.weapon.MirageConstOption, app.player.pl0100.weapon.MirageOffsetOption, app.player.pl0100.weapon.MirageAnimationOption)", 27,actionendtypeid,1,0,0,0,0)
		
	end

	
	if imgui.button("Detonate") then
		sword_detonate = true
	end
	


	local lucifer_enabled_changed,lucifer_enabled_value = imgui.checkbox("Lucifer Enabled",lucifer_enabled)
	if lucifer_enabled_changed then
		lucifer_enabled = lucifer_enabled_value
	end

	

	changed,value = imgui.checkbox("Darkslayer",isdarkslayer)
	if changed then
		isdarkslayer = value
	end

	freezechanged, freezevalue = imgui.drag_float("Freeze point",freeze_cutoff,0.5,0,100)
	if freezechanged then
		freeze_cutoff = freezevalue
	end 
	imgui.text(currentid)
	imgui.text(currentframe)
end)	



re.on_application_entry("UpdateMotion", function()
	if not dante then return end
	
	if not sword_table then return end
	--log.info("Calling joint setting")
	if lucifer_enabled then
		for key,value in ipairs(sword_table) do
			--if helmbreaker loop is being called, then freeze has started and we just need to update the bone positions constantly.
			if value.isLucifer then
				local new_state = value.state --update this variable
				local current_state = value.state --compare against this variable
				
				--When sword progresses past cutoff point for being frozen, save bone positions and trigger helmbreaker action.
				if value.isFreeze and current_state == 0 then
					value.joints = save_bone_positions(value.object:get_cachedTransform())
					--write value.state to 0 here bc we need to dohelmbreaker, this should probably be reworked.
					value.doAction = true
					dohelmbreaker(value.object)
					new_state = 1
				end
				
				--when not in state 0, lucifer is now in effect and the sword should not react to vanilla updates.
				--therefore we make sure to set the sheath ID to -1 in order to prevent it from updating when vanilla actions are done.
				if current_state ~= 0 then
					if value.timer > 0.0 then
						value.object:set_sheathID(-1)
					end
				end
				
				
				--When in frozen state, 
				--force sword to remain in position 
				if current_state == 1 then
					--continue to overwrite the the transform with the saved positions.
					local sword_transform = value.object:get_cachedTransform()
					load_bone_positions(sword_transform,value.joints)
				end
				
				
				
				--while in state 2 or 3, 
				if current_state == 2 or current_state == 3 then
					if not value.enemy_joint then else 
						local sword_transform = value.object:get_cachedTransform()
						bind_bones_to_enemy(sword_transform,value.enemy_joint,value.joints)
					end
				end
				

				

				if current_state == 2 and sword_detonate then
					value.doAction = true
					dohelmbreaker(value.object)
					new_state = 3
				end
				
				if current_state == 1 and sword_detonate then
					--value.doAction = true
					--dohelmbreaker(value.object)
					value.object:set_sheathID(0)
					value.doAction = true
					doconvergence(value.object)
					new_state = 0
				end
				
				--update the state of the sword. (this makes it so we don't advance multiple states in a single update)
				value.state = new_state
			end
		end
	end
	sword_detonate = false
end)

--Sword table now initialized in main update instead of UI (much safer)
re.on_pre_application_entry("UpdateBehavior", function()

	dante = PlayerManipulator.get_playerByID(1)
	if not dante then return end
	--log.info(dante:call("getCurrentNodeID(System.UInt32)",0))
	local motion = dante:call("get_cachedMotion")
	if not motion then return end
	--log.info("got motion")
	local layer = motion:call("getLayer(System.UInt32)",1)
	if not layer then return end
	
	local swordlist = dante:call("get_cachedMirageCtrl"):call("get_SheathObjList()")
	if not swordlist then else
		if not sword_table then
			sword_table = initialize_sword_table(swordlist)
		end
	end
	
	if not sword_table then return end
	

	--this should be the index of the sword currently acting as sword 0 in the formation
	local index = cached_driver_key
	if cached_driver_key == -1 then
		index = 1
	end
	--update Lucifer Track here?
	 local timer_value = sword_table[index].object:get_cachedMotion():getLayer(0):get_Frame()
	UpdateLuciferTrack(LuciferTrack,combopath[LuciferTrack.index],timer_value)
	UpdateSwordVars(LuciferTrack,sword_table)
	
	--if the combo button has been pressed
	if combo_button then
		--immediately reset the value
		combo_button = false
		
		--If the combo hasn't started allow us to start immediately
		--this is a hacky workaround until we immplement proper conditions to start the combo
		if LuciferTrack.index == 1 then
			
			--advance to next stage of combo
			do_combo_smart(dante,combopath,LuciferTrack)
		
		else
		
			--if the combo HAS started
			
			
			--if the button was pressed while the attack Start flag was true
			if LuciferTrack.AttackStart then
				--advance to next stage of combo
				do_combo_smart(dante,combopath,LuciferTrack,false)
			else
				--if the attack start flag wasn't true but the attack reserve flag was true
				if LuciferTrack.AttackReserve then
					--set the combo to be buffered and start as soon as possible
					combo_reserve = true
				end
			end
			
			--if the button was pressed while the attack Start flag was true
			if LuciferTrack.DelayStart then
				--advance to next stage of combo
				do_combo_smart(dante,combopath,LuciferTrack,true)
			else
				--if the attack start flag wasn't true but the attack reserve flag was true
				if LuciferTrack.DelayReserve then
					--set the combo to be buffered and start as soon as possible
					delay_reserve = true
				end
			end
		end
	end
	
	if combo_reserve then
		if LuciferTrack.AttackStart then
			--set the combo to be buffered and start as soon as possible
			combo_reserve = false
			
			--advance to next stage of combo
			do_combo_smart(dante,combopath,LuciferTrack,false)
		end
	end
	
	if delay_reserve then
		if LuciferTrack.DelayStart then
			--set the combo to be buffered and start as soon as possible
			delay_reserve = false
			
			--advance to next stage of combo
			do_combo_smart(dante,combopath,LuciferTrack,true)
		end
	end


	--log.info("got layer")
	currentid = layer:call("get_MotionID")
	currentbankid = layer:call("get_MotionBankID")
	currentframe = layer:call("get_Frame")
	
end)



--lucifer concept:
-- play a sword formation animation
-- track how far through animation the swords are
-- at a point before they finish getting placed, call helmbreaker loop + force lock their positions
-- on DevilMayCry5.app_WeaponDevilSwordSheath__onAttackHit102763, bind the sword to the enemy + disable the hitbox
-- on taunt, damage the enemy (somehow, projectile spawn or something?)

--DevilMayCry5.app_fsm2_player_pl0100_weapon_wp01_006_CommonAction__update317455 <--- hook this in order to check get_timer()
-- 
--get_setActionName()
--HelmBreaker_Loop
--action cancel for swords entirely controlled by first sword in index, therefore we can just not use that sword and avoid the issue all together.

-- Going to need custom keys for each sword in animations to determine which swords should be frozen, which swords can embed, which don't do anything etc
-- should store do freeze & do embedd variables in sword table properties for each sword. 
-- set those based on booleans stored in lucifertrack, should be mapped to sheath id of sword rather than index in sword table (to support dynamic usage of swords)
-- do embed allows for embedding into enemy, do freeze allows for sword becoming frozen.


--gameplay ideas: enemies with a pin can be targeted by other attacks? Swords positioned around/following enemies with pins in them?
--reposition move: all swords currently on field target a specific enemy and turn into interceptor swords
--


-- finding out about sword position update:
-- DevilMayCry5.app_fsm2_player_pl0100_weapon_wp01_006_CommonAction__setConst317453 
-- position otherwise updated by unlabeled subfunction
-- ycoord update done on DevilMayCry5.app_fsm2_player_pl0100_weapon_wp01_006_CommonAction__updateConst317452 
--		as part of DevilMayCry5.app_fsm2_player_pl0100_weapon_wp01_006_CommonAction__update317455 
-- critical component seems to be call at line DevilMayCry5.exe+1715BE2 
-- possibly boolean result of DevilMayCry5.app_fsm2_player_pl0100_weapon_wp01_006_CommonAction__checkConst317450
-- property MirageConstOption controls whether the output sticks to player or not, 1 for yes, 0 for no
-- 5handed, stingers etc all seem to have const values, while other ones don't seem to. 
--- const is retrieved from common action

--DevilMayCry5.exe+1717945 -> DevilMayCry5.app_fsm2_player_pl0100_weapon_wp01_006_CommonAction__setConst317453 sets the start position for the swords animating (I think)


-----Chaser stuff

-- position reset is just in the default cheat engine script currently
--interesting moves
-- round trips needs to be in DSD to work properly but makes the swords follow dante around while spinning
-- roundtrip places sword but need to figure out how to Offset
-- loop of helmbreaker has a constant hit box frozen in place
-- loop of million stab actually works reasonably well
	--this is probably being played sword by sword for balrog flicker attack
-- combo C2 Loop acts likea big round trip but does too much damage and its hit properties are weird
-- intercepter_defense can launch the swords but not sure how to make them hit stuff
-- Gehenna_Loop looks interesting but no hitbox
-- Ultimate stinger_Loop can fly forever (I think I had some sort of psycho crusher move with this before but I forget how I made it work)
-- follow (as an action, not action end type) makes the swords follow you around as if in DT
-- Expansion makes them Appear
-- Convergence makes them disappear
-- SupportReturn makes them appear with an effect (good for cancelling what we're doing)


---Round trips
--setRoundTripsShoot() might just do the whole thing?




-- If dante isn't in the respective style, the support list feature doesn't work
-- having action end type set to none or notification using intercepter_defense makes the swords visible once they return




------Interceptor stuff

-- Guns are fired through DevilMayCry5.app_DevilSwordSheathController__updateInterceptionAction103374 


--Subfunction for firing specific sword
-- Seems to be fired through DevilMayCry5.app_WeaponDevilSwordSheath__setActionWithOption102759  at DevilMayCry5.exe+90A87A 
-- gun call 2D, 2, 20
-- Weapon still not firing? Might be something to do with app.WeaponDevilSwordSheath.TargetMotionData
-- The mirage option variables don't seem to be used during interceptor
--
--
--
-- DevilMayCry5.app_WeaponDevilSwordSheath__setAction102762 (seems to be the final call the weapon makes)
-- DevilMayCry5.exe+90A83E <-- this setRootObjectCall sets our lockonwork, which is probably necessary to make the sword actually target an enemy
-- We need to figure out how to update lockonwork

--LockOnWork:
--DevilMayCry5.exe+90A46C - 48 8B 47 10           - mov rax,[rdi+10] { Get LockOnTargetWork from devilsheathcontroller display class}
--DevilMayCry5.exe+90A09F - 48 8B 35 1A925507     - mov rsi,[DevilMayCry5.exe+7E632C0] { app.LockOnTargetManager into RSI here}

--OK IT WORKS NOW what

-- We need a way of checking if it's ok to fire the sword or not.
	--DevilMayCry5.exe+90A7BA - E8 81EE7001           - call DevilMayCry5.app_WeaponDevilSwordSheath__get_isAction102737
		--might be candidate
			--works but only if swords have proper expansion values set
			--also seems to control cancel windows for base DSD attacks
		--DevilMayCry5.app_WeaponDevilSwordSheath__onAttackHit102763 called when the sword hits the enemy
		
		
	-- We need a way of picking which of the swords to fire

-- We need a way of finding enemies for dante to shoot
	--DevilMayCry5.exe+90A2B9 - E8 022D0E00           - call DevilMayCry5.app_LockOnTargetWork__get_isLockOnEnable80539
	--can use the above to check if lock-on is enabled, which should mean ok to shoot
	
	
-- Cosmetic stuff
-- <expansionType>k__BackingField is a parameter in the sheath manager that controls if the swords re-appear

-- Possible aligning bones 2 and 3 atop each other removes motion blur effect on swords?

--hitbox seems to be determined based on the sheath id when an animation is called

--Dante swordboard? Skates on a summoned Sword
