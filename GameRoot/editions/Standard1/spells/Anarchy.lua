cost = {0,0,0,6}
icon = "anarchy"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Anarchie"
	else
		return "Anarchy"
	end
end

function getDescription()
	if language == "french" then
		return "L'anarchie provoque du mécontentement dans toutes les villes et augmente les chances d'apparition de héros. Chaque tour, chaque unité a 20% de chances (40% pour les unités de type LOI) de se faire infliger 1 ou 2 points de dégâts."
	else
		return "The anarchy causes unrest in all towns and raises chances of apparition of heroes. Every turn, each unit has 20% chances (40% for LAW units) to receive 1 or 2 damages."
	end
end

function onCast()
end

function onResolve(params)
	attachAsGlobal()
end


function onNewTurn()
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			pct = math.random(100)
			life, law, death, chaos = getUnitAlignment(player, unit)
			if pct < 20 or (law == 1 and pct < 40) then
				damageUnit(player, unit, math.random(2))
			end
		end
	end
end

function getMod_happiness(val)
	return val - 3
end

function getMod_heroechances(val)
	return 3 * val
end
