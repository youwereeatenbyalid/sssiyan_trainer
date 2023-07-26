local app = {}
app.PlayerVergilPL = {}
app.PlayerVergilPL.setupAllWeapon = {}
app.PlayerVergilPL.setupAllWeapon.method = sdk.find_type_definition("app.PlayerVergilPL"):get_method("setupWeapon")
--doing this also fucks with Vergil's normal prefab. Maybe the instantiation call happens further down the line?

local beowulf_prefab = sdk.create_instance("via.Prefab"):add_ref()
local deowulf_prefab = sdk.create_instance("via.Prefab"):add_ref()

beowulf_prefab:set_Standby(true)
beowulf_prefab:set_Path("Prefab/Character/Player/Weapon/wp08_002_beowulf.pfb")
deowulf_prefab:set_Standby(true)
deowulf_prefab:set_Path("Prefab/Character/Player/Weapon/wp08_002_deowulf.pfb")
function app.PlayerVergilPL.setupAllWeapon.pre(args)
    local vergil = sdk.to_managed_object(args[2])
    if not vergil then return end
    local weapon_id = sdk.to_int64(args[3])
    if not weapon_id then return end
    log.debug("Start")
    log.debug("Weapon ID" .. tostring(weapon_id))
    if not deowulf_prefab:get_Valid() then
        log.debug("deowulf_prefab invalid")
    end
    if weapon_id == 0 then
        if vergil:get_field("IsDoppel") then
            local prefab = sdk.to_managed_object(args[4])
            if not prefab then return end
            --log.debug("valid prefab")
            --local path = prefab:call("get_ResourcePath()")
            --log.debug(path)
            prefab:set_Path("Prefab/Character/Player/Weapon/wp08_100_yamato.pfb")
            --args[4] = sdk.to_ptr(prefab)
            --local prefab = sdk.to_managed_object(args[4])
            --if not prefab then return end
            --log.debug("valid prefab")
            --local path = prefab:call("get_ResourcePath()")
            --log.debug(path)
        else
            local prefab = sdk.to_managed_object(args[4])
            if not prefab then return end
            prefab:set_Path("Prefab/Character/Player/Weapon/wp08_000_yamato.pfb")
        end
    end
    if weapon_id == 1 then 
        local prefab = sdk.to_managed_object(args[4])
        if not prefab then return end
        if vergil:get_field("IsDoppel") then
            prefab:set_Path("Prefab/Character/Player/Weapon/wp08_002_deowulf.pfb")
        else
            prefab:set_Path("Prefab/Character/Player/Weapon/wp08_002_beowulf.pfb")
        end
    end

end

function app.PlayerVergilPL.setupAllWeapon.post(retval)
    return retval
end

sdk.hook(app.PlayerVergilPL.setupAllWeapon.method,app.PlayerVergilPL.setupAllWeapon.pre,app.PlayerVergilPL.setupAllWeapon.post)
