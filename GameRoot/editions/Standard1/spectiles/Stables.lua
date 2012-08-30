x = 0
y = 0
texture = "stables"
attractAI = 1

childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"stables"}

function getName()
	if language == "french" then
		return "Ecuries"
	else
		return "Stables"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Des chevaux frais"
	else
		return "Fresh horses"
	end
end

function getDescription()
	if language == "french" then
		return "Les unités qui commencent leur tour sur une écurie voient leur vitesse doublée ce tour-ci."
	else
		return "Units that begin their turn on stables have their speed doubled."
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
	if terrain == "sea" or terrain == "forest" or terrain == "mountain" then
		return 0
	end
	return 1
end

function onCreate()
	attachAsGlobal()
end

nbActive = 0
activeHorses = {}

function onNewTurn()
	-- remove all previous effects
	while 1 <= nbActive do
		removeChildEffectFromUnit(1, activeHorses[1][1], activeHorses[1][2])
		table.remove(activeHorses, 1)
		nbActive = nbActive - 1
	end
end

function onNewUnitTurn(unitargs)
	objtype, player, unit = split(unitargs, " ")
	unitx, unity = getObjectPosition("unit", player, unit)
	if x == unitx and y == unity then
		addChildEffectToUnit(1, player, unit)
		nbActive = nbActive + 1
		activeHorses[nbActive] = { player, unit }
	end
end

function child_getMod_speed(effect, value)
	if effect == 1 then
		return 2 * value
	end
	return value
end
