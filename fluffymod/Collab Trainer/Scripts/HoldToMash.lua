

local enumerator = require("enumerator")
local playermanip = require("PlayerManipulator")

local DoublePress = {}
DoublePress.threshhold = 0.0
DoublePress.mash_frequency = 0.0
DoublePress.mash_timer = 0.0
DoublePress.button_flag = 0x1
DoublePress.timer = 0.0
DoublePress.trigger_count = 0
DoublePress.active = false
DoublePress.type = 2
DoublePress.type_enum = {"Hold to mash","Double Press to Mash"}
function DoublePress:new(o)
    o = o or {}
    setmetatable(o,self)
    self.__index = self
    return o
end
function DoublePress:update(pad_input,delta_time)
    if self.type == 1 then
        return self:hold_update(pad_input,delta_time)
    end
    if self.type == 2 then
        return self:double_update(pad_input,delta_time)
    end
end
function DoublePress:double_update(pad_input,delta_time)
    if pad_input:call("isButtonTrigger(System.UInt32)",self.button_flag) then
        self.trigger_count = self.trigger_count+1
        self.timer = 0.0
    end
    --check if double press happened
    if self.trigger_count > 1 then
        self.active = true
        self.mash_timer = 0.0
    end

    --if timer exceeds threshhold reset double press
    if self.timer > self.threshhold then
        self.trigger_count = 0.0
    else
        --increment timer
        self.timer = self.timer + delta_time
    end
    --reset mash on release
    if pad_input:call("isButtonRelease(System.UInt32)",self.button_flag) and self.active then
        self.active = false
        self.trigger_count = 0
    end
    return self.active
end

function DoublePress:hold_update(pad_input,delta_time)
    if pad_input:call("isButtonTrigger(System.UInt32)",self.button_flag) then
        self.timer = 0.0
    end
    if pad_input:call("isButtonOn(System.UInt32)",self.button_flag) then
        self.timer = self.timer + delta_time
    else
        self.timer = 0.0
    end

    --if timer exceeds threshhold reset double press
    if self.timer > self.threshhold then
        self.active = true
    end
    --reset mash on release
    if pad_input:call("isButtonRelease(System.UInt32)",self.button_flag) and self.active then
        self.active = false
    end
    return self.active
end

function DoublePress:mash(pad_input,delta_time)
    if not delta_time then
        pad_input:call("clearButton(System.UInt32)",self.button_flag)	
        local trigger_flags = pad_input:call("get_buttonTriggerFlags()")
        local trigger_flags = trigger_flags + self.button_flag
        pad_input:call("set_buttonTriggerFlags(System.UInt32)",trigger_flags)
    else
        self.mash_timer = self.mash_timer+delta_time
        if self.mash_timer > self.mash_frequency then
            pad_input:call("clearButton(System.UInt32)",self.button_flag)	
            local trigger_flags = pad_input:call("get_buttonTriggerFlags()")
            local trigger_flags = trigger_flags + self.button_flag
            pad_input:call("set_buttonTriggerFlags(System.UInt32)",trigger_flags)
            self.mash_timer = 0.0
        end
    end
    
end

local ebony_ivory_startup = {}
ebony_ivory_startup.threshhold = 5
ebony_ivory_startup.count = 0


local renda_doublepress = {}
renda_doublepress.threshhold = 12
renda_doublepress.button_flag = 0
renda_doublepress.frame_count = 0
renda_doublepress.trigger_count = 0
renda_doublepress.active = false

local sword_renda = DoublePress:new({button_flag = 0x1,threshhold=12.0,mash_frequency=5.0})

local style_renda = DoublePress:new({button_flag = 0x1000,threshhold=12.0,mash_frequency=5.0}) 


local dante_ebony_ivory = DoublePress:new({button_flag=0x2,threshhold=12.0,mash_frequency=5.0})
local vergil_summoned_swords = DoublePress:new({button_flag=0x2,threshhold=12.0,mash_frequency=5.0})

--this needs more work, the double-press is "Sticky" right now. Maybe hook the reset function to reset the count properly?


local function checkDoublePress(pad_input,double_press,button_flag)
    --increment on trigger/button press + reset frame_count
    if pad_input:call("isButtonTrigger(System.UInt32)",button_flag) then
        double_press.trigger_count = double_press.trigger_count+1
        double_press.frame_count = 0
    end
    --check if double press happened
    if double_press.trigger_count > 1 then
        double_press.active = true
    end

    --if frame counter exceeds threshhold reset double press
    if double_press.frame_count > double_press.threshhold then
        double_press.trigger_count = 0
    else
        --increment frame counter
        double_press.frame_count = double_press.frame_count + 1
    end

    if pad_input:call("isButtonOn(System.UInt32)",button_flag) and double_press.active then
        pad_input:call("clearButton(System.UInt32)",button_flag)	
        local trigger_flags = pad_input:call("get_buttonTriggerFlags()")
        local trigger_flags = trigger_flags + button_flag
        pad_input:call("set_buttonTriggerFlags(System.UInt32)",trigger_flags)
    end

    --reset mash on release
    if pad_input:call("isButtonRelease(System.UInt32)",button_flag) and double_press.active then
        double_press.active = false
        double_press.trigger_count = 0
    end
end

local function checkDoublePress_timer(pad_input,double_press,button_flag,delta_time)
    --increment on trigger/button press + reset frame_count
    if pad_input:call("isButtonTrigger(System.UInt32)",button_flag) then
        double_press.trigger_count = double_press.trigger_count+1
        double_press.frame_count = 0.0
    end
    --check if double press happened
    if double_press.trigger_count > 1 then
        double_press.active = true
        double_press.frequency_count = 0.0
    end

    --if frame counter exceeds threshhold reset double press
    if double_press.frame_count > double_press.threshhold then
        double_press.trigger_count = 0.0
    else
        --increment frame counter
        double_press.frame_count = double_press.frame_count + delta_time
    end

    if double_press.active then
        double_press.frequency_count = double_press.frequency_count+delta_time
        if double_press.frequency_count > double_press.frequency then
            pad_input:call("clearButton(System.UInt32)",button_flag)	
            local trigger_flags = pad_input:call("get_buttonTriggerFlags()")
            local trigger_flags = trigger_flags + button_flag
            pad_input:call("set_buttonTriggerFlags(System.UInt32)",trigger_flags)
            double_press.frequency_count = 0.0
        end
    end
    --reset mash on release
    if pad_input:call("isButtonRelease(System.UInt32)",button_flag) and double_press.active then
        double_press.active = false
        double_press.trigger_count = 0
    end
end

local doCharacterUpdateCommon = {}
doCharacterUpdateCommon.method = sdk.find_type_definition("app.Player"):get_method("doCharacterUpdateCommon")
function doCharacterUpdateCommon.pre(args)
    local player = sdk.to_managed_object(args[2])
    if not player then return end

    if playermanip.get_manualPlayer() == player then
        
        local pad_input = player:call("get_padInput()")
		sword_renda:update(pad_input,player:get_DeltaTime())
        style_renda:update(pad_input,player:get_DeltaTime())
        if player:call("get_playerID()") == 1 then
			--is Dante
            local gun_id = player:get_currentWeaponL()
            dante_ebony_ivory:update(pad_input,player:get_DeltaTime())
            --log.debug(gun_id)
            --If Ebony & Ivory
            if gun_id == 0 then  
				--holding gun button
				--checkDoublePress_timer(pad_input,dante_ebony_ivory,0x2,player:get_DeltaTime())
				if dante_ebony_ivory.active then
					dante_ebony_ivory:mash(pad_input,player:get_DeltaTime())
				end
            end
        else
            vergil_summoned_swords:update(pad_input,player:get_DeltaTime())
            if vergil_summoned_swords.active then
                vergil_summoned_swords:mash(pad_input,player:get_DeltaTime())
            end
            --checkDoublePress_timer(pad_input,vergil_summoned_swords,0x2,player:get_DeltaTime())
        end
    end
end

function doCharacterUpdateCommon.post(retval)
    return retval
end
sdk.hook(doCharacterUpdateCommon.method,doCharacterUpdateCommon.pre,doCharacterUpdateCommon.post)

--app.fsm2.player.pl0100.TwosomeTimeChangeCondition.evaluate(via.behaviortree.ConditionArg)
local TwosomeTimeChangeCondition_evaluate = {}
TwosomeTimeChangeCondition_evaluate.method = sdk.find_type_definition("app.fsm2.player.pl0100.TwosomeTimeChangeCondition"):get_method("evaluate")
function TwosomeTimeChangeCondition_evaluate.pre(args)
    --fix this later
    local player = playermanip.get_manualPlayer()
    local pad_input = player:call("get_padInput()")
    if style_renda.active then
        style_renda:mash(pad_input,player:get_DeltaTime())
    end
    if dante_ebony_ivory.active then
        dante_ebony_ivory:mash(pad_input,player:get_DeltaTime())
    end
end
function TwosomeTimeChangeCondition_evaluate.post(retval)
    return retval
end
sdk.hook(TwosomeTimeChangeCondition_evaluate.method,TwosomeTimeChangeCondition_evaluate.pre,TwosomeTimeChangeCondition_evaluate.post)
local rendaCheckerUpdate = {}
rendaCheckerUpdate.method = sdk.find_type_definition("app.RendaChecker"):get_method("update")
function rendaCheckerUpdate.pre(args)
    local renda_checker = sdk.to_managed_object(args[2])
    local pad_input = sdk.to_managed_object(args[3])
    if not renda_checker then return end
    if not pad_input then return end
    if renda_checker:get_buttonBit() == sword_renda.button_flag and sword_renda.active then
        sword_renda:mash(pad_input)
    end

    if renda_checker:get_buttonBit() == style_renda.button_flag and style_renda.active then
        style_renda:mash(pad_input)
    end
    --log.debug(string.format("Renda checker count: %d",renda_checker:get_count()))
    --log.debug("Is Previous On: "..tostring(renda_checker:get_isPrevOn()))
    --renda_doublepress.button_flag = renda_checker:get_buttonBit()
    --checkDoublePress(pad_input,renda_doublepress,renda_doublepress.button_flag)
end

function  rendaCheckerUpdate.post(retval)
    return retval
end

sdk.hook(rendaCheckerUpdate.method,rendaCheckerUpdate.pre,rendaCheckerUpdate.post)

local function draw_threshhold(label,double_press)
    imgui.text(label)
    local type_change,type_value = imgui.combo("Type##"..label,double_press.type,double_press.type_enum)
    if type_change then
            double_press.type = type_value
    end
    local threshhold_change,threshhold_value = imgui.slider_float("Startup window ##"..label,double_press.threshhold,0,100)
    local freq_change,freq_value = imgui.slider_float("Frequency ##"..label,double_press.mash_frequency,0,100)
    local active_change,active_value = imgui.checkbox("Active##"..label,double_press.active)
    if threshhold_change then
        double_press.threshhold = threshhold_value
    end
    if freq_change then
        double_press.mash_frequency = freq_value
    end 
end
re.on_draw_ui(function ()

    draw_threshhold("Ebony and Ivory",dante_ebony_ivory)
    draw_threshhold("Vergil summoned swords",vergil_summoned_swords)
	draw_threshhold("Sword Renda",sword_renda)
    imgui.text("Mash combo")
    local threshhold_change,threshhold_value = imgui.slider_float("Startup window",renda_doublepress.threshhold,0,100)
    if threshhold_change then
        renda_doublepress.threshhold = threshhold_value
    end
    imgui.spacing()
    imgui.text("Sword active "..tostring(sword_renda.active))
    imgui.text("Style active "..tostring(style_renda.active))
end)

re.on_application_entry("PreupdateBehavior",function ()

end);

