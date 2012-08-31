cost = {0,0,0,5}
icon = "volcano"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Volcan"
	else
		return "Volcano"
	end
end

function getDescription()
	if language == "french" then
		return "Le volcan ne peut être lancé que sur une case de montagne. Inflige 3 points de dégâts à toutes les unités sur la case de montagne, et 1 point de dégât à toutes les unités sur les cases adjacentes."
	else
		return "Volcano can only be cast on montains. Deals 3 damages to every units on the montain tile, and 1 damage to every units on adjacent tiles."
	end
end

function onCast()
	selectTarget("tile", "custom")
end

function onCheckSelect(tile)
	x, y = split(tile, " ")
	terrain = getTileData(x, y, "terrain")
	if terrain == "mountain" then
		return 1
	end
	return 0
end

function onResolve(params)
	x, y = splitint(params, " ")
	print("volcano", x, y)
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			unitx, unity = getObjectPosition("unit", player, unit)
	print(unitx, unity)
			if x == unitx and y == unity then
				damageUnit(player, unit, 3)
			else
				if math.abs(x-unitx) <= 1 and math.abs(y-unity) <= 1 then
					damageUnit(player, unit, 1)
				end
			end
		end
	end
end
