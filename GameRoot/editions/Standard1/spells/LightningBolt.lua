cost = {0,0,0,1}
icon = "lbolt"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Foudre"
	else
		return "Lightning bolt"
	end
end

function getDescription()
	if language == "french" then
		return "La foudre s'abat sur l'unité de votre choix (3 points de dégâts)."
	else
		return "A lightning bolt hits target unit (3 damage points)."
	end
end

function onCast()
	selectTarget("unit", "")		-- call C function. The C program will get user's selected target and store it in spell's memory.
											-- The data will be retrieved here at resolve stage.
end

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolve(params)
	player, unit = split(params, " ")
	damageUnit(player, unit, 3)
end
