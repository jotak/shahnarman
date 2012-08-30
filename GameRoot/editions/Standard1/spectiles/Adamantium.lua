x = 0
y = 0
texture = "Adamantium"

childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"Adamantium"}

function getName()
	if language == "french" then
		return "Adamantium"
	else
		return "Adamantium"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Equipement d'adamantium"
	else
		return "Adamantium equipment"
	end
end

function getDescription()
	if language == "french" then
		return "Ce métal extrêmement résistant permet aux villes qui y ont accès de produire des unités mieux armées et plus résistantes."
	else
		return "This extremely resistant metal allows surrounding towns to produce stronger and more resistant units."
	end
end

function init(posx, posy)
	x = posx
	y = posy
end

function getMapPos()
	return x, y
end

function isAllowedOn(terrain)
	if terrain == "sea" then
		return 0
	end
	return 1
end

function onCreate()
	towns = {getTownsList()}
	table.foreach(towns, function(i, town)
		tx, ty = getObjectPosition("town", town)
		if math.abs(tx-x) <= 3 and math.abs(ty-y) <= 3 then
			attachToTown(town)
		end
	end)
end

function onUnitProduced(townargs, unitargs)
	objtype, player, unit = split(unitargs, " ")
	addChildEffectToUnit(1, player, unit)
end

function child_getMod_melee(effect, value)
	if effect == 1 then
		return value + 1
	end
	return value
end

function child_getMod_armor(effect, value)
	if effect == 1 then
		return value + 1
	end
	return value
end
