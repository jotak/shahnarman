cost = {0,5,0,0}
icon = "crusade"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Croisade"
	else
		return "Crusade"
	end
end

function getDescription()
	if language == "french" then
		return "Toutes les unités de croisés, amies ou ennemies, gagnent 2 en mêlée, endurance et tir. Les autres unités de LOI gagnent 1 en mêlée, endurance et tir."
	else
		return "All crusader units, friend or foe, increase their melee, endurance and range by 2. Other LAW units increase their melee, endurance and range by 1."
	end
end

function onCast()
end

function onResolve()
  attachAsGlobal()
end

function getMod_melee(val, params)
	-- parse params (it contains target identifiers)
	objtype, player, unit = split(params, " ")
	if objtype == "unit" then
		name, edition = getUnitNameAndEdition(player, unit)
		if name == "Crusader" then
			return val + 2
		end
		life, law, death, chaos = getUnitAlignment(player, unit)
		if law == 1 then
			return val + 1
		end
	end
	return val
end

function getMod_endurance(val, params)
	-- parse params (it contains target identifiers)
	objtype, player, unit = split(params, " ")
	if objtype == "unit" then
		name, edition = getUnitNameAndEdition(player, unit)
		if name == "Crusader" then
			return val + 2
		end
		life, law, death, chaos = getUnitAlignment(player, unit)
		if law == 1 then
			return val + 1
		end
	end
	return val
end

function getMod_range(val, params)
	-- parse params (it contains target identifiers)
	objtype, player, unit = split(params, " ")
	if val > 0 and objtype == "unit" then
		name, edition = getUnitNameAndEdition(player, unit)
		if name == "Crusader" then
			return val + 2
		end
		life, law, death, chaos = getUnitAlignment(player, unit)
		if law == 1 then
			return val + 1
		end
	end
	return val
end
