x = 0
y = 0
texture = "WoodsOfTheAncients"
attractAI = 1

function getName()
	if language == "french" then
		return "Forêt des Anciens"
	else
		return "Woods of the Ancients"
	end
end

function getDescription()
	if language == "french" then
		return "."
	else
		return "."
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
	if terrain == "forest" then
		return 1
	end
	return 0
end

_nbTowns = 0
_towns = {}
function onCreate(id)
	math.randomseed(os.time())
	attachAsGlobal()
	towns = {getTownsList()}
	table.foreach(towns, function(i, town)
		tx, ty = getObjectPosition("town", town)
		if math.abs(tx-x) <= 2 and math.abs(ty-y) <= 2 then
			attachToTown(town)
			-- we need to store the attached towns, because we called "attachAsGlobal" => thus function "onUnitProduced" wil be caled not only for attached towns, but for every towns in play
			_nbTowns = _nbTowns + 1
			_towns[_nbTowns] = town
		end
	end)
end

function onUnitProduced(townargs, unitargs)
	rnd = math.random(4)
	if rnd == 1 then
		objtype, town = split(townargs, " ")
		for i = 1, _nbTowns do
			if town == _towns[i] then
				objtype, player, unit = split(unitargs, " ")
				addSkillToUnit("Regenerate", "", player, unit)
				break
			end
		end
	end
end

function onNewUnitTurn(unitargs)
	objtype, player, unit = split(unitargs, " ")
	unitx, unity = getObjectPosition("unit", player, unit)
	if x == unitx and y == unity then
		life, endurance = getUnitData(player, unit, "life", "endurance")
		if life < endurance then
			setUnitData(player, unit, "life", life + 1)
		end
	end
end

function isBattleValid(unit1, isRange, isAttacking, unit2)
	objtype, player, unit = split(unit1, " ")
	unitx, unity = getObjectPosition("unit", player, unit)
	if x == unitx and y == unity then
		return 0
	end
	return 1
end

function canRangeAttack(attacker, defender)
	objtype, player, unit = split(attacker, " ")
	unitx, unity = getObjectPosition("unit", player, unit)
	if x == unitx and y == unity then
		return 0
	end
	return 1
end
