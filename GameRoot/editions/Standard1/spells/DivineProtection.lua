cost = {0,3,0,0}
icon = "divineprotection"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Protection divine"
	else
		return "Divine protection"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité visée gagne 3 en armure."
	else
		return "Increases target unit armor by 3."
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

function getMod_armor(i)
	return i + 3
end
