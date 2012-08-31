cost = {0,0,2,0}
icon = "necrophilia"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Necrophilie"
	else
		return "Necrophilia"
	end
end

function getDescription()
	if language == "french" then
		return "La n�crophilie donne +1 m�l�e et +1 endurance � tous les morts-vivants."
	else
		return "Necrophilia gives +1 melee and +1 endurance to all undeads."
	end
end

function onCast()
end

function onResolve(params)
	attachAsGlobal()
end

function getMod_melee(value, params)
	objtype, player, unit = split(params, " ")
	if objtype == "unit" then
		name, edition = getUnitNameAndEdition(player, unit)
		if name == "Ghoul" then
			return value + 1
		end
	end
	return value
end

function getMod_endurance(value, params)
	objtype, player, unit = split(params, " ")
	if objtype == "unit" then
		name, edition = getUnitNameAndEdition(player, unit)
		if name == "Ghoul" then
			return value + 1
		end
	end
	return value
end
