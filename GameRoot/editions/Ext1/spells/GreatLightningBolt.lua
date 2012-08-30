cost = {0,0,0,3}
icon = "lbolt"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Grande Foudre"
	else
		return "Great Lightning Bolt"
	end
end

function getDescription()
	if language == "french" then
		return "La Grande Foudre s'abat sur l'unité de votre choix (5 points de dégâts)."
	else
		return "The Great Lightning Bolt hits target unit (5 damage points)."
	end
end

function onCast()
	selectTarget("unit", "")
end

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolve(params)
	player, unit = split(params, " ")
	damageUnit(player, unit, 5)
end
