PlayerManipulator = require "PlayerManipulator"
local nero = nil

local mod_breaker_prefab = sdk.create_instance("via.Prefab"):add_ref()
mod_breaker_prefab:set_Standby(true)
mod_breaker_prefab:set_Path("Prefab/character/player/weapon/wp00_010_oldstyle_red.pfb")

local vanilla_breaker_prefab = sdk.create_instance("via.Prefab"):add_ref()
vanilla_breaker_prefab:set_Standby(true)
vanilla_breaker_prefab:set_Path("Prefab/character/player/weapon/wp00_010_oldstyle.pfb")

local function update_Prefab(index,path)
	if not nero then else
		local array = nero:get_field("GauntletPrefabs")
		local index1 = array[index]
		local index1_alt = array:get_element(index)
		
		log.debug(tostring(array[index]:ToString()))
		log.debug(tostring(array[index].value__))
		
		local new_element = sdk.create_instance("via.Prefab"):add_ref()
        new_element:set_Standby(true)
        new_element:set_Path(path)

		array[index] = new_element
	end
end

re.on_draw_ui(function ()
    if imgui.button("blue") then
        nero = PlayerManipulator.get_manualPlayer()
        update_Prefab(0,"Prefab/character/player/weapon/wp00_010_oldstyle.pfb")
    end
    if imgui.button("red") then
        nero = PlayerManipulator.get_manualPlayer()
        update_Prefab(0,"Prefab/character/player/weapon/wp00_010_oldstyle_red.pfb")
    end
end)