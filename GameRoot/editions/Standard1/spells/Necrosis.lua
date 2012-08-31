cost = {0,0,3,0}
icon = "necrosis"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Nécrose"
	else
		return "Necrosis"
	end
end

function getDescription()
	if language == "french" then
		return "Inflige 1 dégât par tour à l'unité ciblée."
	else
		return "Deals 1 damage per turn to target ubit."
	end
end

function onCast()
	selectTarget("unit", "inrange")
end

function onResolve(params)
	player, unit = splitint(params, " ")
	attachToUnit(player, unit)
end

function onNewUnitTurn(unitids)
	objtype, player, unit = split(unitids, " ")
	damageUnit(player, unit, 1)
end
