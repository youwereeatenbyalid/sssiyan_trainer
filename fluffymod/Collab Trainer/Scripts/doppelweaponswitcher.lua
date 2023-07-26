
local function generate_enum(typename)
    local t = sdk.find_type_definition(typename)
    if not t then return {} end

    local fields = t:get_fields()
    local enum = {}

    for i, field in ipairs(fields) do
        if field:is_static() then
            local name = field:get_name()
            local raw_value = field:get_data(nil)

            log.info(name .. " = " .. tostring(raw_value))

            enum[name] = raw_value
        end
    end

    return enum
end
local function generate_reverse_enum(typename)
    local t = sdk.find_type_definition(typename)
    if not t then return {} end

    local fields = t:get_fields()
    local enum = {}

    for i, field in ipairs(fields) do
        if field:is_static() then
            local name = field:get_name()
            local raw_value = field:get_data(nil)

            log.info(name .. " = " .. tostring(raw_value))

            enum[raw_value] = name
        end
    end
    return enum
end

local app = {
    PlayerID = {},
	network = {
		NetworkObjectType = {}
	},
	CommandType = {},
	Reverse_CommandType = {},
	Player = {
		Reverse_CancelReserveBit = {},
		CancelReserveBit = {}
	},
	PlayerDante = {
		ActiveAction = {}
	}
}



local players = {}
local playernames = {}

local playernamelist = {
"Nero",
"Dante",
"Gilver",
"Vergil",
"VergilPL"
}

app.PlayerID = generate_enum("app.PlayerID")
app.network.NetworkObjectType = generate_enum("app.network.NetworkObjectType")
app.Reverse_CommandType = generate_reverse_enum("app.CommandType")
app.Player.Reverse_CancelReserveBit = generate_reverse_enum("app.Player.CancelReserveBit")
app.Player.CancelReserveBit = generate_enum("app.Player.CancelReserveBit") 
app.CommandType = generate_enum("app.CommandType") 
app.PlayerDante.ActiveAction = generate_enum("app.PlayerDante.ActiveAction")




local function get_player(charactername)
    local playman = sdk.get_managed_singleton(sdk.game_namespace("PlayerManager"))
    if not playman then
        return nil
    end
    return playman:call("getPlayerByID",app.PlayerID[charactername])
end

local function get_player_by_ID(id)
    local playman = sdk.get_managed_singleton(sdk.game_namespace("PlayerManager"))
    if not playman then
        return nil
    end
    return playman:call("getPlayerByID",id)
end


local function get_currentplayer()
    local playman = sdk.get_managed_singleton(sdk.game_namespace("PlayerManager"))
    if not playman then
        return nil
    end
	return playman:call("get_manualPlayer")
end

local function is_player(charactername)
	local manualplayer = get_currentplayer()
	local nameplayer = get_player(charactername)
	if not manualplayer then return false end
	return nameplayer == manualplayer
end



local vergil_judgement_cut_cancel_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("judgementCutCancel")
local vergil_shell_create_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("createShell")
local vergil_do_start_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("comeBackDoppelGanger")

local doppelganger = nil
local store_weapon_data = nil
local is_jc_interrupt = false

local beowulf_exception = false

local function pre_judgementCutCancel(args)
	doppelganger = sdk.to_managed_object(args[2])
	log.info("got doppel")
	local vergil = doppelganger:call("get_cachedDoppelOwner")
	log.info("got vergil")
	--if true then return sdk.PreHookResult.SKIP_ORIGINAL end
	--doppelganger:set_field("<RoundTripCharge>k__BackingField",vergil:call("get_DoppelJudgeMentCutCharge"))
	--skips if both vergil and doppel have beowulf equipped and doing just combos.
	if vergil:call("get_BeowulfCharge"):call("isJust") == true and vergil:call("get_cachedDoppel"):call("get_currentWeaponS") == 2 then
		return sdk.PreHookResult.SKIP_ORIGINAL
	end
	if vergil:call("get_vergilTrack"):get_field("DoppelJudgeMentCutJR") == true or vergil:call("get_BeowulfCharge"):call("isJust") == true then return
	else
		if doppelganger:call("get_currentWeaponS") == 0 then return end
		return sdk.PreHookResult.SKIP_ORIGINAL 
	end

end
local function post_judgementCutCancel(retval)

	return retval
end

local vergil_shell_create_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("createShell")
local function pre_shellCreate(args)
	local vergil = sdk.to_managed_object(args[2])
	local shelltrack = sdk.to_managed_object(args[3])
	if shelltrack:get_field("ShellID") == 105 and vergil:call("get_currentWeaponS") ~= 0 and vergil:get_field("IsDoppel") then
		log.info("swap weapons")
		is_jc_interrupt = true
		doppelganger = vergil
		store_weapon_data = doppelganger:call("get_weaponS_00")
		doppelganger:call("set_weaponS_00",doppelganger:call("get_weaponContainer"):call("get_Item",0))
	end

end

local function post_shellCreate(retval)
	if is_jc_interrupt == true then
		log.info("resetting doppel weapon")
		is_jc_interrupt = false
		if not store_weapon_data then return retval end
		if store_weapon_data ~= doppelganger:call("get_weaponS_00") then
			local weaponobject = doppelganger:call("get_weaponS_00"):get_field("Obj"):call("get_GameObject")
			--weaponobject:set_field("Draw",false)
			weaponobject:call("set_DrawSelf(System.Boolean)",false)
		end
		doppelganger:call("set_weaponS_00",store_weapon_data)
		log.info("set doppel weapon")
	end
	return retval
end

local vergil_setupDoppelMode_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("setupDoppelMode(app.PlayerVergilPL.DelayState)")
local vergil_comeBackDoppelGanger_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("comeBackDoppelGanger()")
local vergil_updateWeaponChange_method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("updateWeaponChange()")
--Set the DoppelGanger's Weapon to Vergil's weapon
function updateDoppelWeapon(vergil)
	if not vergil then return end
	local doppel = vergil:call("get_cachedDoppel()")
	local vergil_weaponS = vergil:call("get_currentWeaponS()")
	doppel:call("setSlotWeaponS(app.PlayerVergilPL.WeaponS, System.Boolean)",vergil_weaponS,false)
end

local function pre_vergil_setupDoppelMode(args)
	local vergil = sdk.to_managed_object(args[2])
	updateDoppelWeapon(vergil)
end

local function post_vergil_setupDoppelMode(retval)

	return retval
end

local function pre_vergil_comeBackDoppelGanger(args)
	local vergil = sdk.to_managed_object(args[2])

	if vergil:call("get_BeowulfCharge"):call("isJust") == true and vergil:call("get_cachedDoppel"):call("get_currentWeaponS") == 2 then
		return sdk.PreHookResult.SKIP_ORIGINAL
	end
end

local function post_vergil_comeBackDoppelGanger(retval)

	return retval
end

local function pre_vergil_updateWeaponChange(args)
	local vergil_character = sdk.to_managed_object(args[2])
	log.info("got vergil?")
	if vergil_character:get_field("IsDoppel") == true then
		vergil_character:call("get_padInput()"):call("clearButton(System.UInt32)",0x2000)
		log.info("Try to skip")
		--return sdk.PreHookResult.SKIP_ORIGINAL 
	end
end

local function post_vergil_updateWeaponChange(retval)

	return retval
end

--setupDoppelMode(app.PlayerVergilPL.DelayState) called from Player Vergil When summoning doppelganger
--comeBackDoppelGanger()
--changeWeaponS(app.PlayerVergilPL.WeaponS, System.Int32) (hook this to prevent doppelganger from 
local vergil_updateDoppelDelayChange = {}
vergil_updateDoppelDelayChange.method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("updateDoppelDelayChange")
function vergil_updateDoppelDelayChange.pre(args)
	local vergil = sdk.to_managed_object(args[2])
	if not vergil then return end
	local get_doppel_mode = vergil.IsDoppel
	--log.debug(tostring(get_doppel_mode))
	if get_doppel_mode then else
		local pad_input = vergil:get_padInput()
		if not pad_input then return end
		local current_delay_state = vergil:get_CurrentDelayState()
		if current_delay_state == 1 and pad_input:isButtonTrigger(0x800) then
			updateDoppelWeapon(vergil)
		end
		if current_delay_state == 2 and pad_input:isButtonTrigger(0x100) then
			updateDoppelWeapon(vergil)
		end

		if current_delay_state == 3 and pad_input:isButtonTrigger(0x400) then
			updateDoppelWeapon(vergil)
		end
	end
end

function vergil_updateDoppelDelayChange.post(retval)
	return retval
end


sdk.hook(vergil_setupDoppelMode_method,pre_vergil_setupDoppelMode,post_vergil_setupDoppelMode)
sdk.hook(vergil_comeBackDoppelGanger_method,pre_vergil_comeBackDoppelGanger,post_vergil_comeBackDoppelGanger)
sdk.hook(vergil_updateWeaponChange_method,pre_vergil_updateWeaponChange,post_vergil_updateWeaponChange)
sdk.hook(vergil_updateDoppelDelayChange.method,vergil_updateDoppelDelayChange.pre,vergil_updateDoppelDelayChange.post)


local function create_doppel_resource() 
	local resource = sdk.create_resource("via.render.MeshMaterialResource", "Character/Weapon/wp03_000/wp03_000_Astral.mdf2"):add_ref()
	local holder = resource:create_holder("via.render.MeshMaterialResourceHolder"):add_ref()
	if holder ~= nil then
		log.debug("successfully created resource")
		get_currentplayer():call("get_cachedDoppel()"):call("get_weaponS_00"):get_field("Obj"):call("get_cachedMesh()"):call("set_Material(via.render.MeshMaterialResourceHolder)",holder)
	end
end

re.on_draw_ui(function()
	weapchange,weapvalue = imgui.slider_int("Weapon to swap to", weapon_value, 0, 2)
	if weapchange then
		weapon_value = weapvalue
	end
	--if imgui.button("create resource") then
	--	create_doppel_resource()
	--end
	if imgui.button("Set weapon test") then
		local vergil = get_currentplayer()
			vergil:call("setSlotWeaponS(app.PlayerVergilPL.WeaponS, System.Boolean)",weapon_value,false)
	end
end)

re.on_pre_application_entry("UpdateBehavior", function()
	if not doppelganger then return end

end)

sdk.hook(vergil_judgement_cut_cancel_method,pre_judgementCutCancel,post_judgementCutCancel)
sdk.hook(vergil_shell_create_method,pre_shellCreate,post_shellCreate)
--sdk.hook(vergil_do_start_method,pre_vergilDoStart,post_vergilDoStart)