cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Téléportation de ville"
	else
		return "Teleport town"
	end
end

function getDescription()
	if language == "french" then
		return "Téléporte la ville ciblée n'importe où sur la carte."
	else
		return "Teleport target town anywhere on the map."
	end
end

function onCast()
	selectTarget("town", "inrange", "", "selectDestination")
end

function selectDestination()
	selectTarget("tile", "")
end

function onResolve(params, caster)
	town, x, y = split(params, " ")
	teleport("town", town, x, y)
end
