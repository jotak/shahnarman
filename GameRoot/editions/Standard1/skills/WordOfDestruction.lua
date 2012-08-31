childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"wordofdestruction"}
icon = "wordofdestruction"

function getName()
	if language == "french" then
		return "Mot de destruction"
	else
		return "Word of destruction"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Mot de destruction"
	else
		return "Word of destruction"
	end
end

function getDescription()
	if language == "french" then
		return "Une fois dans la partie, tue toutes les unités sur la case ciblée, à l'exception des Shamahs."
	else
		return "Once in the game, kill all units on target tile (except Shahmahs)."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("tile", "inrange")
	end
end

function onResolveChild(effect, params)
	if effect == 1 then
		x, y = splitint(params, " ");
		players = {getPlayersList()}
		for p, player in pairs(players) do
			units = {getUnitsList(player)}
			for u, unit in pairs(units) do
				unitx, unity = getObjectPosition("unit", player, unit)
				if x == unitx and y == unity then
					b = isShahmah(player, unit)
					if b == 0 then
						endurance = getUnitData(player, unit, "endurance")
						damageUnit(player, unit, endurance)
					end
				end
			end
		end
	end
end
