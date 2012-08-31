cost = {0,0,2,0}
icon = "insanestrength"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Force démentielle"
	else
		return "Insane strength"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité visée gagne 2 en mêlée."
	else
		return "Increases target unit melee by 2."
	end
end

function onCast()
	selectTarget("unit", "")
end

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolve(params)
	player, unit = split(params, " ")
	attachToUnit(player, unit)
end

function getMod_melee(i)
	return i + 2
end
