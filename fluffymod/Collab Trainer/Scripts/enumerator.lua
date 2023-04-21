local enumerator = {}
enumerator.is_verbose = false


function enumerator.generate_enum(typename)
    local t = sdk.find_type_definition(typename)
    if not t then return {} end

    local fields = t:get_fields()
    local enum = {}

    for i, field in ipairs(fields) do
        if field:is_static() then
            local name = field:get_name()
            local raw_value = field:get_data(nil)
			if enumerator.is_verbose then
				log.info(name .. " = " .. tostring(raw_value))
			end
            enum[name] = raw_value
        end
    end
    return enum
end


function enumerator.generate_reverse_enum(typename)
    local t = sdk.find_type_definition(typename)
    if not t then return {} end

    local fields = t:get_fields()
    local enum = {}

    for i, field in ipairs(fields) do
        if field:is_static() then
            local name = field:get_name()
            local raw_value = field:get_data(nil)
			if enumerator.is_verbose then
				log.info(name .. " = " .. tostring(raw_value))
			end
            enum[raw_value] = name
        end
    end
    return enum
end
return enumerator
