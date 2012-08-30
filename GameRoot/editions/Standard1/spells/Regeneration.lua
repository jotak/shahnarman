cost = {2,0,0,0}
icon = "regeneration"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Maître guérisseur"
	else
		return "Master healer"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité visée régénère deux points de vie de plus par tour même lorsqu'elle est tuée, et gagne 1 en endurance."
	else
		return "Target unit regenerates two life points more even when it's killed, and has its endurance increased by 1."
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

-- this function is called, when the spell is attached as effect on a unit, at the end of resolve phase
function onNewUnitTurn(unitids)
	objtype, player, unit = split(unitids, " ")
	life, endurance = getUnitData(player, unit, "life", "endurance")
	if life < endurance then
		setUnitData(player, unit, "life", life + 1)
	end
end

-- this is called at any time the C program or any LUA script needs to get the endurance of targeted unit
function getMod_endurance(endurance)
	return endurance + 1
end
