cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Téléportation"
	else
		return "Teleport"
	end
end

function getDescription()
	if language == "french" then
		return "Téléporte l'unité ciblée n'importe où sur la carte."
	else
		return "Teleport target unit anywhere on the map."
	end
end

function onCast()
	str = ""
	if language == "french" then
		str = "Sélectionnez une unité à portée pour la téléportation"
	else
		str = "Select a unit in range to teleport"
	end
	selectTarget("unit", "inrange", str, "selectDestination")
end

function selectDestination()
	str = ""
	if language == "french" then
		str = "Sélectionnez la destination"
	else
		str = "Select the destination"
	end
	selectTarget("tile", "", str)
end

function onResolve(params, caster)
	player, unit, x, y = split(params, " ")
	teleport("unit", player, unit, x, y)
end
