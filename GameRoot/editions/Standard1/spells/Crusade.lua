cost = {0,3,0,0}
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
		return "Toutes les unités de Loi, amies ou ennemies, gagnent 2 en force et 2 en vitesse."
	else
		return "All Law units, friend or foe, increase their strength and their speed by 2."
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
		life, law, death, chaos = getUnitAlignment(player, unit)
		if law == 1 then
			return val + 2
		end
	end
	return val
end

function getMod_speed(val, params)
	-- parse params (it contains target identifiers)
	objtype, player, unit = split(params, " ")
	if objtype == "unit" then
		life, law, death, chaos = getUnitAlignment(player, unit)
		if law == 1 then
			return val + 2
		end
	end
	return val
end
