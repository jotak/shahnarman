cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Terrain gelé"
	else
		return "Frozen land"
	end
end

function getDescription()
	if language == "french" then
		return "Transforme n'importe quel terrain en toundra"
	else
		return "Turn any land into toundra"
	end
end

function onCast()
	selectTarget("tile", "")
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	setTileData(x, y, "terrain", "toundra")
end
