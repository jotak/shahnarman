cost = {0,0,2,0}
icon = "friche"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Terrain en friche"
	else
		return "Land in friche"
	end
end

function getDescription()
	if language == "french" then
		return "Supprime toute nourriture du terrain."
	else
		return "Remove any food from this tile."
	end
end

function onCast()
	selectTarget("tile", "")
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	attachToTile(x, y)
end

function getMod_food(value)
	return 0
end
