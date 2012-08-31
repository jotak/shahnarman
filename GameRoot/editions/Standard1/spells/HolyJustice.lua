cost = {6,0,0,0}
icon = "holyjustice"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Justice sacrée"
	else
		return "Holy justice"
	end
end

function getDescription()
	if language == "french" then
		return "La justice sacrée tue toutes les unités de type MORT dans un rayon de 4 cases autour de la case ciblée."
	else
		return "The holy justice kills all DEATH units within 4 tiles range around target tile."
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params)
	x, y = split(params, " ")
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			pct = math.random(100)
			life, law, death, chaos = getUnitAlignment(player, unit)
			unitx, unity = getObjectPosition("unit", player, unit)
			if math.abs(x-unitx) <= 4 and math.abs(y-unity) <= 4 and death == 1 then
				b = isShahmah(player, unit)
				if b == 0 then
					endurance = getUnitData(player, unit, "endurance")
					damageUnit(player, unit, endurance)
				end
			end
		end
	end
end
