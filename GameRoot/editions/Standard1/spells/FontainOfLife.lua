cost = {3,0,0,0}
icon = "fountainoflife"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Fontaine de vie"
	else
		return "Fountain of life"
	end
end

function getDescription()
	if language == "french" then
		return "Toutes les unités sur la case ciblée gagnent 2 points de vie."
	else
		return "All units on target tile gain 2 hit points."
	end
end

function onCast()
	selectTarget("tile", "")
end

function onResolve(params)
	x, y = split(params, " ")
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			unitx, unity = getObjectPosition("unit", player, unit)
			if x == unitx and y == unity then
				life, endurance = getUnitData(player, unit, "life", "endurance")
				if life < endurance then
					life = math.min(endurance, life+2)
					setUnitData(player, unit, "life", life)
				end
			end
		end
	end
end
