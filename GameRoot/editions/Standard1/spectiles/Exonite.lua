x = 0
y = 0
texture = "Exonite"

function getName()
	if language == "french" then
		return "Exonite"
	else
		return "Exonite"
	end
end

function getDescription()
	if language == "french" then
		return "Ce métal très rare permet aux villes qui y ont accès de produire des unités ayant la capacité Armor piercing."
	else
		return "This very rare metal allows surrounding towns to produce units with Armor piercing skill."
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
	addSkillToUnit("ArmorPiercing", "", player, unit)
end
