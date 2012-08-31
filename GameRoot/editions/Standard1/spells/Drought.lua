cost = {0,0,2,0}
icon = "drought"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Sécheresse"
	else
		return "Drought"
	end
end

function getDescription()
	if language == "french" then
		return "Transforme la case ciblée et les cases adjacentes en déserts."
	else
		return "Turn target tile and surrounding tiles into desert."
	end
end

function onCast()
	selectTarget("tile", "")
end

function onResolve(params, caster)
	centerx, centery = split(params, " ")
	x = centerx - 1
	mapw, maph = getMapDimensions()
	while x <= centerx + 1 do
		if x >= 0 and x < mapw then
			y = centery - 1
			while y <= centery + 1 do
				if y >= 0 and y < maph then
					setTileData(x, y, "terrain", "desert")
				end
				y = y + 1
			end
		end
		x = x + 1
	end
end
