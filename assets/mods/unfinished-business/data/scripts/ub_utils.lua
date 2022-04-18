function _flatten_table(in_table, out_table, key_prefix)
    for k,v in pairs(in_table) do
        if type(k) == 'string' and string.find(k, '%.') ~= nil then
            log.warn("game state keys should be a string value without any '.' characters (given: " .. k .. ')')
        end

        local t = type(v)
        if t == 'table' then
            _flatten_table(v, out_table, key_prefix .. k .. '.')
        elseif t == 'boolean' or t == 'number' or t == 'string' then
            out_table[key_prefix .. k] = v
        elseif t == 'nil' then
            log.warn("nil value is not properly supported yet. It is persisted as false (" .. k .. ')')
            out_table[key_prefix .. k] = false
        else
            log.error("unsupported value type in save states: " .. t .. ' (' .. k .. ')')
        end
    end
end

function flatten(table)
    local flat_table = {}
    _flatten_table(table, flat_table, '')
    return flat_table
end

function _unflatten_value(table, key, val)
    local pos = string.find(key, '%.')
    if pos == nil then
        table[key] = val
    else
        local prefix = string.sub(key, 0, pos - 1)
        if table[prefix] == nil then
            table[prefix] = {}
        end
        _unflatten_value(table[prefix], string.sub(key, pos + 1), val)
    end
end

function unflatten(flat_table)
    local table = {}
    for k,v in pairs(flat_table) do
        _unflatten_value(table, k, v)
    end
    return table
end

