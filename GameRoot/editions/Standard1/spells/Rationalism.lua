cost = {0,5,0,0}
icon = "rationalism"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Rationalisme"
	else
		return "Rationalism"
	end
end

function getDescription()
	if language == "french" then
		return "Divise par 2 le mana disponible pour tous les joueurs."
	else
		return "Divide available mana for all players by 2."
	end
end

function onCast()
end

function onResolve()
  attachAsGlobal()
end

function getMod_mana_life(val, params)
	return val / 2
end

function getMod_mana_law(val, params)
	return val / 2
end

function getMod_mana_death(val, params)
	return val / 2
end

function getMod_mana_chaos(val, params)
	return val / 2
end
