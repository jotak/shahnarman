cost = {0,0,0,2}
icon = "mario"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Invoquer goblin des montagnes rouges"
	else
		return "Summon red mountains goblin"
	end
end

function getDescription()
	if language == "french" then
		return "Invoque un goblin des montagnes rouges.\n"..getUnitDescription("Goblin_des_montagnes_rouges")
	else
		return "Summons a red mountains goblin.\n"..getUnitDescription("Goblin_des_montagnes_rouges")
	end
end

function onCast()
	selectTarget("tile", "inrange custom")
end

function onCheckSelect(tile)
	x, y = split(tile, " ")
	terrain = getTileData(x, y, "terrain")
	if terrain == "sea" then
		return 0
	end
	return 1
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "Goblin_des_montagnes_rouges")
end
